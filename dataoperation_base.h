#ifndef _BASE_OPERATIONS_H_
#define _BASE_OPERATIONS_H_

#include <regex>
#include <string>

class DataHandlerLoadOperationBase
{
public:
    virtual std::string Id() const = 0;
    virtual bool DoLoad(std::vector<char>& outData) = 0;
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


#endif