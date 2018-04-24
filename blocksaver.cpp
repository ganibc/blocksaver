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

FileDataOperationManager::FileDataOperationManager(std::vector<char> filePrefix, std::vector<char> filePosfix)
    : m_FilePrefix(std::move(filePrefix))
    , m_FilePostfix(std::move(filePosfix))
{
}

std::unique_ptr<FileDataLoader> FileDataOperationManager::GetFileDataLoader(std::string filename) const
{
    unique_ptr<FileDataLoader> fileDataResult;
    ifstream file(filename.c_str());
    if(!file.is_open())
        return fileDataResult;

    const int prefixSize = m_FilePrefix.size();
    const int postfixSize = m_FilePostfix.size();
    int minFileSize = prefixSize + postfixSize;

    int dataSize = 0;
    if(minFileSize > 0)
    {
        file.seekg(0, file.end);
        const int fileSize = (int)file.tellg();
        if(minFileSize > fileSize)
            return fileDataResult;
        dataSize = fileSize - prefixSize - postfixSize;
    }

    if(prefixSize > 0)
    {
        std::vector<char> buffer(prefixSize);
        file.seekg(0, file.beg);
        file.read(buffer.data(), prefixSize);
        if(buffer != m_FilePrefix)
            return fileDataResult;
    }

    if(postfixSize > 0)
    {
        std::vector<char> buffer(postfixSize);
        file.seekg(-postfixSize, file.end);
        file.read(buffer.data(), postfixSize);
        if(buffer != m_FilePostfix)
            return fileDataResult;        
    }
    fileDataResult = unique_ptr<FileDataLoader>(new FileDataLoader(std::move(filename), prefixSize, dataSize));
    return fileDataResult;
}

bool FileDataOperationManager::IsFileReadyToLoad(std::string filename) const
{
    return GetFileDataLoader(std::move(filename)) != nullptr;
}

std::unique_ptr<FileDataLoader> FileDataOperationManager::SaveDataToFile(std::string filename, std::vector<char>&& data, bool forceOverwrite) const
{
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
    return std::remove(filename.c_str()) == 0;
}


FileLister::FileLister(std::string path, std::regex regex)
    : m_DirPath(std::move(path))
    , m_Regex(std::move(regex))
{

}

std::vector<std::string> FileLister::GetFileList() const
{
    vector<string> files;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(m_DirPath.c_str())) != NULL)
    {
        /* print all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL)
        {
            if(ent->d_type == DT_REG)
            {
                if(regex_match(ent->d_name, m_Regex))
                {
                    files.push_back(ent->d_name);
                }
            }
        }
        closedir(dir);
    }
    else
    {
        //  TODO: add error/warning message here.
    }
    return files;
}


FileManager::FileManager(FileLister*& fileLister)
    : m_KeepRun(true)
    , m_FileLister(fileLister)
{
}

void FileManager::Run()
{
    while(m_KeepRun)
    {

    }
}

void FileManager::RemoveFile(const std::string& filename)
{
}
