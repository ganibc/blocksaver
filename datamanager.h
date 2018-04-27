#ifndef BLOCK_SAVER_H_
#define BLOCK_SAVER_H_

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

class DataHandler;
class DataOperationManagerBase;

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