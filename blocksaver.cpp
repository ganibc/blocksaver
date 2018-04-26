#include "blocksaver.h"
#include <dirent.h>
#include <assert.h>
#include <cstdio>
#include <unordered_set>

using namespace std;

DataHandlerLoadOperationFile::DataHandlerLoadOperationFile(std::string filename, int startOffset, int dataSize)
    : m_Filename(std::move(filename))
    , m_StartOffset(startOffset)
    , m_DataSize(dataSize)
{
    
}

bool DataHandlerLoadOperationFile::DoLoad(std::vector<char>& outData)
{
    ifstream ifile(m_Filename.c_str());
    if(!ifile.is_open())
        return false;
    outData.resize(m_DataSize);
    ifile.seekg(m_StartOffset, ifile.beg);
    ifile.read(outData.data(), m_DataSize);
    return true;
}

DataHandlerLoadOperationMysql::DataHandlerLoadOperationMysql(MYSQL* connection, std::string id)
    : m_Connection(connection)
    , m_ID(id)
{
    m_Statement = mysql_stmt_init(m_Connection);
    m_StatementCloser.statement = m_Statement;

    string selectStatement = "SELECT data FROM filedata WHERE id=?";
    mysql_stmt_prepare(m_Statement, selectStatement.c_str(), selectStatement.length());

    memset(m_BindParam, 0, sizeof(m_BindParam));

    m_BindParam[0].buffer_type = MYSQL_TYPE_STRING;
    m_BindParam[0].buffer = (void*)m_ID.c_str();
    m_BindParam[0].buffer_length = m_ID.length();
    mysql_stmt_bind_param(m_Statement, m_BindParam);

}

DataHandlerLoadOperationMysql::~DataHandlerLoadOperationMysql()
{
}


bool DataHandlerLoadOperationMysql::DoLoad(std::vector<char>& outData)
{    
    mysql_stmt_execute(m_Statement);
    MYSQL_BIND bindResult[1];
    memset(bindResult, 0, sizeof(bindResult));

    unsigned long dataLen = 0;
    bindResult[0].buffer_type = MYSQL_TYPE_LONG_BLOB;
    bindResult[0].length = &dataLen;    
    mysql_stmt_bind_result(m_Statement, bindResult);
    
    mysql_stmt_fetch(m_Statement);

    outData.resize(dataLen);
    bindResult[0].buffer = outData.data();
    bindResult[0].buffer_length = outData.size();
    mysql_stmt_fetch_column(m_Statement, bindResult, 0, 0);
}


DataHandler::DataHandler(DataHandlerLoadOperationBase* loadOperation)
    : m_LoadOperation(loadOperation)
    , m_Loaded(false)
{

}

DataHandler::DataHandler(DataHandlerLoadOperationBase* loadOperation, std::vector<char>&& data)
    : m_LoadOperation(loadOperation)
    , m_Data(std::move(data))
    , m_Loaded(true)
{
}

bool DataHandler::Load()
{
    if(!m_Loaded)
    {
        if(!m_LoadOperation->DoLoad(m_Data))
        {
            return false;
        }
        m_Loaded = true;
    }
    return true;
}

void DataHandler::Unload()
{
    m_Data.clear();
    m_Data.shrink_to_fit();
    m_Loaded = false;
}

bool DataHandler::IsLoaded() const
{
    return m_Loaded;
}

FileDataOperationManager::FileDataOperationManager(std::string path, std::vector<char> filePrefix, std::vector<char> filePosfix)
    : m_FilePrefix(std::move(filePrefix))
    , m_FilePostfix(std::move(filePosfix))
    , m_DirPath(std::move(path))
{
}

std::vector<std::string> FileDataOperationManager::GetDataList(std::regex regex, bool checkNotation) const
{
    vector<string> files;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(m_DirPath.c_str())) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            if(ent->d_type == DT_REG)
            {
                if(regex_match(ent->d_name, regex))
                {
                    files.emplace_back(ent->d_name);
                }
            }
        }
        closedir(dir);
    }
    else
    {
        //  TODO: add error/warning message here.
    }

    if(checkNotation)
    {
        for(int i = 0; i < files.size(); ++i)
        {
            auto& file = files[i];
            while(!IsFileReadyToLoad(file) && i < files.size())
            {
                files[i] = files.back();
                files.resize(files.size() - 1);
            }
        }    
    }
    return files;

}


std::unique_ptr<DataHandler> FileDataOperationManager::GetDataHandler(std::string id) const
{
    unique_ptr<DataHandler> fileDataResult;
    std::string& filename = id;
    int dataSize = GetFileDataSize(filename);
    if(dataSize >= 0)
    {
        auto fileOperation = new DataHandlerLoadOperationFile(std::move(m_DirPath + filename), m_FilePrefix.size(), dataSize);
        fileDataResult = make_unique<DataHandler>(fileOperation);
    }
    return fileDataResult;
}

int FileDataOperationManager::GetFileDataSize(const std::string& filename) const
{
    ifstream file((m_DirPath + filename).c_str());
    if(!file.is_open())
        return -1;

    const int prefixSize = m_FilePrefix.size();
    const int postfixSize = m_FilePostfix.size();
    int minFileSize = prefixSize + postfixSize;

    int dataSize = 0;
    if(minFileSize > 0)
    {
        file.seekg(0, file.end);
        const int fileSize = (int)file.tellg();
        if(minFileSize > fileSize)
            return -1;
        dataSize = fileSize - prefixSize - postfixSize;
    }

    if(prefixSize > 0)
    {
        std::vector<char> buffer(prefixSize);
        file.seekg(0, file.beg);
        file.read(buffer.data(), prefixSize);
        if(buffer != m_FilePrefix)
            return -1;
    }

    if(postfixSize > 0)
    {
        std::vector<char> buffer(postfixSize);
        file.seekg(-postfixSize, file.end);
        file.read(buffer.data(), postfixSize);
        if(buffer != m_FilePostfix)
            return -1;        
    }
    return dataSize;
}

bool FileDataOperationManager::IsFileReadyToLoad(const std::string& filename) const
{
    //  GetFileDataSize will add the path
    return GetFileDataSize(filename) >= 0;
}

std::unique_ptr<DataHandler> FileDataOperationManager::StoreData(std::string id, std::vector<char>&& data, bool forceOverwrite)
{
    std::string filename = m_DirPath + id;
    std::unique_ptr<DataHandler> result;
    bool fileExisted = false;
    {
        ifstream ifile(filename.c_str());
        fileExisted = ifile.is_open();
    }

    if(fileExisted && !forceOverwrite)
    {
        //  TODO: warning file existed and not force overwrite
        return result;
    }
    ofstream ofile(filename.c_str());
    if(!ofile.is_open())
    {
        //  TODO: error cannot write to file
        return result;
    }
    ofile.write(m_FilePrefix.data(), m_FilePrefix.size());
    ofile.write(data.data(), data.size());
    ofile.write(m_FilePostfix.data(), m_FilePostfix.size());
    ofile.flush();
    auto fileOperation = new DataHandlerLoadOperationFile(std::move(filename), m_FilePrefix.size(), data.size());    
    result = make_unique<DataHandler>(fileOperation, std::move(data));
    if(fileExisted)
    {
        //  TODO: warn that existing file is overwritten
    }
    return result;
}

bool FileDataOperationManager::DeleteData(const std::string& id)
{
    return std::remove((m_DirPath + id).c_str()) == 0;
}



MysqlDataOperationManager::MysqlDataOperationManager(MYSQL* mysqlConnection, std::string tablename)
    : m_Connection(mysqlConnection)
    , m_TableName(std::move(tablename))
{

}

bool MysqlDataOperationManager::IsExists(const std::string& id) const
{
    MYSQL_STMT* selectStatement = mysql_stmt_init(m_Connection);

    StatementCloser closer;
    closer.statement = selectStatement;

    std::string statementStr("SELECT id FROM filedata WHERE id=?");
    if(mysql_stmt_prepare(selectStatement, statementStr.c_str(), statementStr.length()))
    {
        return false;
    }
    
    MYSQL_BIND bindParam[1];
    memset(bindParam, 0, sizeof(bindParam));

    bindParam[0].buffer_type = MYSQL_TYPE_STRING;
    bindParam[0].buffer = (void*)id.c_str();
    bindParam[0].buffer_length = id.length();
    if(mysql_stmt_bind_param(selectStatement, bindParam))
    {
        return false;
    }
    if(mysql_stmt_execute(selectStatement))
    {
        return false;
    }
    mysql_stmt_store_result(selectStatement);
    auto rowCount = mysql_stmt_num_rows(selectStatement);
    return rowCount > 0;
}


std::unique_ptr<DataHandler> MysqlDataOperationManager::GetDataHandler(std::string id) const
{
    std::unique_ptr<DataHandler> result;

    if(IsExists(id))
    {
        auto mysqlOperation = new DataHandlerLoadOperationMysql(m_Connection, id);
        result = make_unique<DataHandler>(mysqlOperation);
    }

    return result;
}

std::vector<std::string> MysqlDataOperationManager::GetDataList(std::regex regex, bool checkNotation) const
{
    std::vector<std::string> result;

    MYSQL_STMT* selectStatement = mysql_stmt_init(m_Connection);

    StatementCloser closer;
    closer.statement = selectStatement;

    std::string statementStr("SELECT id FROM filedata");
    if(mysql_stmt_prepare(selectStatement, statementStr.c_str(), statementStr.length()))
    {
        return result;
    }

    MYSQL_BIND bindResult[1];
    memset(bindResult, 0, sizeof(bindResult));

    char idBuffer[256];

    bindResult[0].buffer_type = MYSQL_TYPE_STRING;
    bindResult[0].buffer = idBuffer;
    bindResult[0].buffer_length = sizeof(idBuffer);
    if (mysql_stmt_bind_result(selectStatement, bindResult))
    {
        return result;
    }
    
    if(mysql_stmt_execute(selectStatement))
    {
        return result;
    }
    mysql_stmt_store_result(selectStatement);

    while (!mysql_stmt_fetch(selectStatement))
    {
        result.emplace_back(idBuffer);
    }

    return result;
}

std::unique_ptr<DataHandler> MysqlDataOperationManager::StoreData(std::string id, std::vector<char>&& data, bool forceOverwrite)
{
    std::unique_ptr<DataHandler> result;

    bool rowExisted = IsExists(id);
    if(rowExisted && !forceOverwrite)
    {
        //  TODO: warning file existed and not force overwrite
        return result;
    }
    
    MYSQL_STMT* insertStatement = mysql_stmt_init(m_Connection);
    StatementCloser closer;
    closer.statement = insertStatement;
    
    std::string insertStatementStr("INSERT INTO filedata(id, data) VALUES (?, ?)");
    if(mysql_stmt_prepare(insertStatement, insertStatementStr.c_str(), insertStatementStr.length()))
    {
        return result;
    }

    MYSQL_BIND bindParam[2];
    memset(bindParam, 0, sizeof(bindParam));

    auto idLen = id.length();
    bindParam[0].buffer_type = MYSQL_TYPE_STRING;
    bindParam[0].buffer = (void*)id.c_str();
    bindParam[0].buffer_length = idLen;
    bindParam[0].length = &idLen;    

    auto dataSize = data.size();
    bindParam[1].buffer_type = MYSQL_TYPE_LONG_BLOB;
    bindParam[1].buffer = data.data();
    bindParam[1].buffer_length = dataSize;
    bindParam[1].length = &dataSize;
    
    if (mysql_stmt_bind_param(insertStatement, bindParam))
    {
        return result;
    }

    if (mysql_stmt_execute(insertStatement))
    {
        return result;
    }

    auto mysqlOperation = new DataHandlerLoadOperationMysql(m_Connection, id);
    result = make_unique<DataHandler>(mysqlOperation, std::move(data));

    return result;
}

bool MysqlDataOperationManager::DeleteData(const std::string& id)
{
    MYSQL_STMT* deleteStatement = mysql_stmt_init(m_Connection);
    StatementCloser closer;
    closer.statement = deleteStatement;
    
    std::string deleteStatementStr("DELETE FROM filedata WHERE id=?");
    if(mysql_stmt_prepare(deleteStatement, deleteStatementStr.c_str(), deleteStatementStr.length()))
    {
        return false;
    }

    MYSQL_BIND bindParam[1];
    memset(bindParam, 0, sizeof(bindParam));

    auto idLen = id.length();
    bindParam[0].buffer_type = MYSQL_TYPE_STRING;
    bindParam[0].buffer = (void*)id.c_str();
    bindParam[0].buffer_length = idLen;
    bindParam[0].length = &idLen;    
    if (mysql_stmt_bind_param(deleteStatement, bindParam))
    {
        return false;
    }

    if (mysql_stmt_execute(deleteStatement))
    {
        return false;
    }
    auto affectedRow = mysql_stmt_affected_rows(deleteStatement);
    
    return affectedRow > 0;
}


DataManager::DataManager(DataOperationManagerBase* operationManager)
    : m_FileOperationManager(operationManager)
{
}

bool DataManager::AddData(std::string id, std::vector<char>&& data)
{
    if(m_DataHandles.count(id))
        return false;   //  loader already exists
    auto fileDataLoader = m_FileOperationManager->StoreData(id, std::move(data), false);
    if(fileDataLoader == nullptr)
    {
        return false;   //  file already exists, but loader is not
    }
    m_DataHandles[id] = std::move(fileDataLoader);
    return true;
}

void DataManager::RemoveData(const std::string& id)
{
    m_DataHandles.erase(id);
    m_FileOperationManager->DeleteData(id);
}

DataManager::AddAndRemoveDataListPair DataManager::DiffDataHandles(bool updateCache)
{
    AddAndRemoveDataListPair dataListPair;

    std::vector<std::string>& newFiles = dataListPair.first;
    std::vector<std::string>& removedFiles = dataListPair.second;

    auto fileList = m_FileOperationManager->GetDataList();
    unordered_set<string> fileSet(fileList.begin(), fileList.end());
    for(auto& filePair : m_DataHandles)
    {
        auto& filename = filePair.first;
        if(fileSet.count(filename) == 0)
        {
            removedFiles.push_back(filename);
        }
    }
    if(updateCache)
    {
        for(auto& filename : removedFiles)
        {
            m_DataHandles.erase(filename);            
        }
    }

    for(auto& filename : fileList)
    {
        if(m_DataHandles.count(filename) == 0)
        {
            auto fileDataLoader = m_FileOperationManager->GetDataHandler(filename);
            if(fileDataLoader != nullptr)
            {
                if(updateCache)
                    m_DataHandles[filename] = std::move(fileDataLoader);
                newFiles.push_back(filename);
            }
        }
    }
    return dataListPair;
}
