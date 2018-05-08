#include "blocksaver.h"
#include <unistd.h>
#include <libconfig.h++>

using namespace libconfig;

void usage() {
  fprintf(stderr, "Usage:\n\tblocksaver -c \"jobmaker.cfg\" -l \"log_dir\"\n");
}

DataManager* CreateFileDataManagerFromSettings(const libconfig::Setting& setting)
{
    std::string path;
    std::string prefix, postfix;
    if(!setting.lookupValue("path", path))
    {
        return nullptr;
    }
    if(path == "")
    {
        return nullptr;
    }
    setting.lookupValue("prefix", prefix);
    setting.lookupValue("postfix", postfix);

    auto fileOperationManager = new FileDataOperationManager(path
            , std::vector<char>(prefix.begin(), prefix.end())
            , std::vector<char>(postfix.begin(), postfix.end()));
    return new DataManager(fileOperationManager);
}

DataManager* CreateMySQLDataManagerFromSettings(const libconfig::Setting& setting)
{
    std::string server;
    std::string username, password;
    std::string dbschema;
    std::string tablename = "filedata";
    int port = 0;
    if(!setting.lookupValue("server", server))
    {
        return nullptr;
    }
    if(!setting.lookupValue("username", username))
    {
        return nullptr;
    }
    if(!setting.lookupValue("password", password))
    {
        return nullptr;
    }
    if(!setting.lookupValue("dbschema", dbschema))
    {
        return nullptr;
    }
    setting.lookupValue("tablename", tablename);
    setting.lookupValue("port", port);

    auto mysqlOperationManager = new MysqlDataOperationManager(server, username, password, dbschema, tablename, port);
    return new DataManager(mysqlOperationManager);
    
}

DataManager* CreateDataManagerFromSettings(const libconfig::Setting& setting)
{
    DataManager* result = nullptr;
    string workerType;
    setting.lookupValue("type", workerType);
    if(workerType == "file")
    {
        result = CreateFileDataManagerFromSettings(setting);
    }
    else if(workerType == "mysql")
    {
        result = CreateMySQLDataManagerFromSettings(setting);
    } 
    if(result)
    {
        string name;
        setting.lookupValue("name", name);
        bool syncDelete = true;
        setting.lookupValue("syncdelete", syncDelete);
        result->SetName(std::move(name));
        result->EnableSyncDelete(syncDelete);
    }
    return result;
}


int main(int argc, char **argv)
{
    char *optLogDir = NULL;
    char *optConf = NULL;
    int c;

    if (argc <= 1)
    {
        usage();
        return 1;
    }
    while ((c = getopt(argc, argv, "c:l:h")) != -1)
    {
        switch (c)
        {
        case 'c':
            optConf = optarg;
            break;
        case 'l':
            optLogDir = optarg;
            break;
        case 'h':
        default:
            usage();
            exit(0);
        }
    }

    libconfig::Config cfg;
    try
    {
        cfg.readFile(optConf);
    }
    catch (const FileIOException &fioex)
    {
        std::cerr << "I/O error while reading file." << std::endl;
        return (EXIT_FAILURE);
    }
    catch (const ParseException &pex)
    {
        std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
                  << " - " << pex.getError() << std::endl;
        return (EXIT_FAILURE);
    }

    std::vector<unique_ptr<DataManager>> managers;
    std::vector<uint32_t> timePeriods;
    const Setting &root = cfg.getRoot();
    try
    {
        const Setting &workers = root["workers"];
        for (int i = 0; i < workers.getLength(); ++i)
        {
            const Setting &workerSetting = workers[i];
            DataManager* manager = CreateDataManagerFromSettings(workerSetting);
            if(manager)
            {
                int timePeriod = 5000;
                managers.emplace_back(manager);
                workerSetting.lookupValue("time", timePeriod);
                timePeriods.push_back(timePeriod);
            }
        }
    }
    catch (const SettingNotFoundException &nfex)
    {
        // Ignore.
    }

    SyncManager syncManager;
    for(int i = 0; i < managers.size(); ++i)
    {
        std::chrono::milliseconds time(timePeriods[i]);
        syncManager.AddWorker(managers[i].release(), time);
    }
    thread appThread(&SyncManager::Run, &syncManager);

    appThread.join();

    return 0;
}