#ifndef BLOCK_SAVER_H_
#define BLOCK_SAVER_H_

#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <unordered_map>

#include <stdint.h>
#include <regex>
#include <mysql.h>

class DataHandlerLoadOperationBase
{
public:
    virtual std::string Id() const = 0;
    virtual bool DoLoad(std::vector<char>& outData) = 0;
};

class DataHandlerLoadOperationFile : public DataHandlerLoadOperationBase
{
public:
    bool DoLoad(std::vector<char>& outData) override;
    const std::string& GetFilename() const
    {
        return m_Filename;
    }
    std::string Id() const override
    {
        return GetFilename();
    }

private:
    friend class FileDataOperationManager;
    DataHandlerLoadOperationFile(std::string filename, int startOffset, int dataSize);

    std::string m_Filename;
    int m_StartOffset;
    int m_DataSize;
};

struct StatementCloser
{
    StatementCloser()
        : statement(nullptr)
    {
    }

    ~StatementCloser()
    {
        if(statement)
        {
            mysql_stmt_close(statement);
        }
    }

    MYSQL_STMT* statement;
};

class DataHandlerLoadOperationMysql : public DataHandlerLoadOperationBase
{
public:
    ~DataHandlerLoadOperationMysql();
    bool DoLoad(std::vector<char>& outData) override;
    std::string Id() const override
    {
        return m_ID;
    }

private:
    friend class MysqlDataOperationManager;
    DataHandlerLoadOperationMysql(MYSQL* connection, std::string id);

    MYSQL* m_Connection;
    std::string m_ID;
    MYSQL_STMT* m_Statement;
    MYSQL_BIND m_BindParam[1];
    StatementCloser m_StatementCloser;
};

class DataHandler
{
public:
    DataHandler(DataHandlerLoadOperationBase* loadOperation);
    DataHandler(DataHandlerLoadOperationBase* loadOperation, std::vector<char>&& data);

    bool Load();
    void Unload();
    bool IsLoaded() const;
    std::vector<char> GiveupData()
    {
        std::vector<char> newContainer;
        newContainer = std::move(m_Data);
        Unload();
        return newContainer;
    }
    const std::vector<char>& Data() const
    {
        return m_Data;
    }

    const std::vector<char>& GetData()
    {
        if(!IsLoaded())
            Load();
        return m_Data;
    }

private:

    std::unique_ptr<DataHandlerLoadOperationBase> m_LoadOperation;
    std::vector<char> m_Data;
    bool m_Loaded;
};

class DataOperationManagerBase
{
public:
    virtual std::unique_ptr<DataHandler> GetDataHandler(std::string id) const = 0;
    virtual std::vector<std::string> GetDataList(std::regex regex = std::regex(".*"), bool checkNotation = false) const = 0;
    virtual std::unique_ptr<DataHandler> StoreData(std::string id, std::vector<char>&& data, bool forceOverwrite = false)= 0;
    virtual bool DeleteData(const std::string& id) = 0;
};

class FileDataOperationManager : public DataOperationManagerBase
{
public:

    FileDataOperationManager(std::string path, std::vector<char> filePrefix, std::vector<char> filePosfix);

    std::unique_ptr<DataHandler> GetDataHandler(std::string id) const override;
    std::vector<std::string> GetDataList(std::regex regex = std::regex(".*"), bool checkNotation = false) const override;
    std::unique_ptr<DataHandler> StoreData(std::string id, std::vector<char>&& data, bool forceOverwrite = false) override;
    bool DeleteData(const std::string& id) override;

    int GetFileDataSize(const std::string& filename) const;
    bool IsFileReadyToLoad(const std::string& filename) const;

private:
    std::string m_DirPath;
    std::vector<char> m_FilePrefix;
    std::vector<char> m_FilePostfix;
};

class MysqlDataOperationManager : public DataOperationManagerBase
{
public:

    MysqlDataOperationManager(MYSQL* mysqlConnection, std::string tablename);

    std::unique_ptr<DataHandler> GetDataHandler(std::string id) const override;
    std::vector<std::string> GetDataList(std::regex regex = std::regex(".*"), bool checkNotation = false) const override;
    std::unique_ptr<DataHandler> StoreData(std::string id, std::vector<char>&& data, bool forceOverwrite = false) override;
    bool DeleteData(const std::string& id) override;

    bool IsExists(const std::string& id) const;
private:
    MYSQL* m_Connection;
    std::string m_TableName;
};

class DataManager
{
public:
    using AddAndRemoveDataListPair = std::pair<std::vector<std::string>, std::vector<std::string>>;

    DataManager(DataOperationManagerBase* operationManager);
    //  store data persistently and keep info in the cache
    bool AddData(std::string id, std::vector<char>&& data);
    //  remove data from persistent storage and cache
    void RemoveData(const std::string& id);
    
    //  list files from drive
    //  return new detected filenames
    AddAndRemoveDataListPair DiffDataHandles(bool updateCache = true);

    void SetName(std::string name)
    {
        m_Name = std::move(name);
    }

    const std::string& GetName() const
    {
        return m_Name;
    }

    const std::unordered_map<std::string, std::shared_ptr<DataHandler>>& GetDataHandles() const
    {
        return m_DataHandles;
    }
private:
    // std::unique_ptr<FileLister> m_FileLister;
    std::unique_ptr<DataOperationManagerBase> m_FileOperationManager;
    std::unordered_map<std::string, std::shared_ptr<DataHandler>> m_DataHandles;
    std::string m_Name;
};

#endif // BLOCK_SAVER_H_