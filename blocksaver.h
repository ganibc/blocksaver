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

class DataCache
{
public:
private:
    std::string m_Id;
    std::vector<char> m_Data;
    uint64_t timestamp;
};

using DataCachePtr = std::shared_ptr<DataCache>;

class DataCacher
{
public:
private:
    std::unordered_map<std::string, DataCachePtr> m_CachedData;
    std::vector<DataCachePtr> m_NewCachedData;
};

class DataUploader
{

};

class MetadataFileReader
{
public:
private:
};

class BlockFileReader
{
public:
    BlockFileReader(std::string metadataFilename);
    DataCachePtr Run();
private:
    std::string m_MetadataFilename;
};

class FileDataLoader
{
public:
    bool Load();
    void Unload();
    bool IsLoaded() const;
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

    const std::string& GetFilename() const
    {
        return m_Filename;
    }
private:
    friend class FileDataOperationManager;
    FileDataLoader(std::string&& filename, int startOffset, int dataSize);
    FileDataLoader(std::string&& filename, std::vector<char>&& data, int startOffset);

    std::string m_Filename;
    std::vector<char> m_Data;
    int m_StartOffset;
    int m_DataSize;
    bool m_Loaded;
};


// class FileLister
// {
// public:
//     FileLister(std::string path, std::regex regex = std::regex(".*"));
//     std::vector<std::string> GetFileList() const;
// private:
//     std::string m_DirPath;
//     std::regex m_Regex;
// };

class FileDataOperationManager
{
public:

    FileDataOperationManager(std::string path, std::vector<char> filePrefix, std::vector<char> filePosfix);

    std::unique_ptr<FileDataLoader> GetFileDataLoader(std::string filename) const;
    int GetFileDataSize(const std::string& filename) const;
    bool IsFileReadyToLoad(const std::string& filename) const;
    std::vector<std::string> GetFileList(std::regex regex = std::regex(".*"), bool checkNotation = false) const;

    std::unique_ptr<FileDataLoader> SaveDataToFile(std::string filename, std::vector<char>&& data, bool forceOverwrite = false) const;
    bool DeleteFile(const std::string& filename);
private:
    std::string m_DirPath;
    std::vector<char> m_FilePrefix;
    std::vector<char> m_FilePostfix;
};

class FileManager
{
public:
    FileManager(FileDataOperationManager* operationManager);

    //  save data to file and keep info in the cache
    bool AddFile(std::string filename, std::vector<char>&& data);
    
    //  list files from drive
    //  return new detected filenames
    std::vector<std::string> PoolFiles();

    void RemoveFile(const std::string& filename);

    const std::unordered_map<std::string, std::shared_ptr<FileDataLoader>>& GetFiles() const
    {
        return m_Files;
    }
private:
    // std::unique_ptr<FileLister> m_FileLister;
    std::unique_ptr<FileDataOperationManager> m_FileOperationManager;

    std::unordered_map<std::string, std::shared_ptr<FileDataLoader>> m_Files;
};

#endif // BLOCK_SAVER_H_