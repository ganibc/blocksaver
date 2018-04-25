#include "blocksaver.h"
#include <dirent.h>
#include <assert.h>
#include <cstdio>

using namespace std;


BlockFileReader::BlockFileReader(std::string metadataFilename)
{

}

DataCachePtr BlockFileReader::Run()
{
    return make_shared<DataCache>();
}

FileDataLoader::FileDataLoader(std::string&& filename, int startOffset, int dataSize)
    : m_Filename(std::move(filename))
    , m_StartOffset(startOffset)
    , m_DataSize(dataSize)
    , m_Loaded(false)
{

}

FileDataLoader::FileDataLoader(std::string&& filename, std::vector<char>&& data, int startOffset)
    : m_Filename(std::move(filename))
    , m_Data(std::move(data))
    , m_StartOffset(startOffset)
    , m_Loaded(true)
{
    m_DataSize = m_Data.size();
}

bool FileDataLoader::Load()
{
    m_Loaded = false;
    ifstream ifile(m_Filename.c_str());
    if(!ifile.is_open())
        return false;
    m_Data.resize(m_DataSize);
    ifile.seekg(m_StartOffset, ifile.beg);
    ifile.read(m_Data.data(), m_DataSize);
    m_Loaded = true;
    return true;
}

void FileDataLoader::Unload()
{
    m_Data.clear();
    m_Data.shrink_to_fit();
    m_Loaded = false;
}

bool FileDataLoader::IsLoaded() const
{
    return m_Loaded;
}

FileDataOperationManager::FileDataOperationManager(std::string path, std::vector<char> filePrefix, std::vector<char> filePosfix)
    : m_FilePrefix(std::move(filePrefix))
    , m_FilePostfix(std::move(filePosfix))
    , m_DirPath(std::move(path))
{
}

std::vector<std::string> FileDataOperationManager::GetFileList(std::regex regex, bool checkNotation) const
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


std::unique_ptr<FileDataLoader> FileDataOperationManager::GetFileDataLoader(std::string filename) const
{
    unique_ptr<FileDataLoader> fileDataResult;
    int dataSize = GetFileDataSize(filename);
    if(dataSize >= 0)
    {
        fileDataResult = unique_ptr<FileDataLoader>(new FileDataLoader(std::move(m_DirPath + filename), m_FilePrefix.size(), dataSize));
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
    return GetFileDataSize(filename) >= 0;
}

std::unique_ptr<FileDataLoader> FileDataOperationManager::SaveDataToFile(std::string filename, std::vector<char>&& data, bool forceOverwrite) const
{
    filename = m_DirPath + filename;
    std::unique_ptr<FileDataLoader> result;
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
    result = unique_ptr<FileDataLoader>(new FileDataLoader(std::move(filename), std::move(data), m_FilePrefix.size()));
    if(fileExisted)
    {
        //  TODO: warn that existing file is overwritten
    }
    return result;
}

bool FileDataOperationManager::DeleteFile(const std::string& filename)
{
    return std::remove((m_DirPath + filename).c_str()) == 0;
}


// FileLister::FileLister(std::string path, std::regex regex)
//     : m_DirPath(std::move(path))
//     , m_Regex(std::move(regex))
// {
//     if(m_DirPath.back() != '/')
//     {
//         m_DirPath += '/';
//     }
// }

// std::vector<std::string> FileLister::GetFileList() const
// {
//     vector<string> files;
//     DIR *dir;
//     struct dirent *ent;
//     if ((dir = opendir(m_DirPath.c_str())) != NULL)
//     {
//         while ((ent = readdir(dir)) != NULL)
//         {
//             if(ent->d_type == DT_REG)
//             {
//                 if(regex_match(ent->d_name, m_Regex))
//                 {
//                     files.push_back(m_DirPath + string(ent->d_name));
//                 }
//             }
//         }
//         closedir(dir);
//     }
//     else
//     {
//         //  TODO: add error/warning message here.
//     }
//     return files;
// }


FileManager::FileManager(FileDataOperationManager* operationManager)
    : m_FileOperationManager(operationManager)
{
}

bool FileManager::AddFile(std::string filename, std::vector<char>&& data)
{
    if(m_Files.count(filename))
        return false;   //  loader already exists
    auto fileDataLoader = m_FileOperationManager->SaveDataToFile(filename, std::move(data), false);
    if(fileDataLoader == nullptr)
    {
        return false;   //  file already exists, but loader is not
    }
    m_Files[filename] = std::move(fileDataLoader);
    return true;
}

void FileManager::RemoveFile(const std::string& filename)
{
    m_Files.erase(filename);
    m_FileOperationManager->DeleteFile(filename);
}

std::vector<std::string> FileManager::PoolFiles()
{
    std::vector<std::string> newFiles;
    auto fileList = m_FileOperationManager->GetFileList();
    for(auto& filename : fileList)
    {
        auto iter = m_Files.find(filename);
        if(iter == m_Files.end() || iter->second == nullptr)
        {
            auto fileDataLoader = m_FileOperationManager->GetFileDataLoader(filename);
            if(fileDataLoader != nullptr)
            {
                m_Files[filename] = std::move(fileDataLoader);
                newFiles.push_back(filename);
            }
        }
    }
    return newFiles;
}
