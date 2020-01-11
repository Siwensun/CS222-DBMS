#include "pfm.h"

PagedFileManager *PagedFileManager::_pf_manager = nullptr;

PagedFileManager &PagedFileManager::instance() {
    static PagedFileManager _pf_manager = PagedFileManager();
    return _pf_manager;
}

PagedFileManager::PagedFileManager() = default;

PagedFileManager::~PagedFileManager() { delete _pf_manager; }

PagedFileManager::PagedFileManager(const PagedFileManager &) = default;

PagedFileManager &PagedFileManager::operator=(const PagedFileManager &) = default;

RC PagedFileManager::createFile(const std::string &fileName) {
    std::fstream f;
    f.open(fileName, std::ios::in);

    if(f){
        // if file exists, should fail

        // comment log message due to submission instructions
        // std::cout << "[Error] Create a File which already exists. " << std::endl;
        return -1;
    }
    else{
        //file not exists, create a new one with only out mode.
        f.open(fileName, std::ios::out);
        if(f.is_open()){
            // std::cout << "[Success] create a file! " << std::endl;
            f.close();
            return 0;
        }else{
            // std::cout << "[Error] Create a File failed. " << std::endl;
            return -1;
        }
    }

}

RC PagedFileManager::destroyFile(const std::string &fileName) {
    // same as above, first check whether the file exists.
    std::fstream f;
    f.open(fileName, std::ios::in);

    if(f){
        // if file exists, delete it.
        f.close();
        if(remove(fileName.c_str()) != 0 ){
            // std::cout << "[Error] Destroy a file failed. " << std::endl;
            return -1;
        }
        else{
//            std::cout << "[Success] destroy a file! " << std::endl;
            return 0;
        }
    }
    else{
        f.close();
//        std::cout << "[Error] Destroy a file which not exists. " << std::endl;
        return -1;
    }
}

RC PagedFileManager::openFile(const std::string &fileName, FileHandle &fileHandle)
{
    std::fstream &f = fileHandle.getFile();

    // doesn't allow if the fileHandle is already a handle for some open file when it passed to the openFile method.
    if(f.is_open())
    {
        // std::cout << "[Error] fileHandlle already open a file" << std::endl;
        return -1;
    }

    f.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
    if(f.is_open())
    {
        f.seekg(0, std::ios::end);
        if(f.tellg() == 0){
//            if the file is the first time to open, initialize the counter page.
            fileHandle.initializeCounterValues();
        }
//        locate to the start of the page
        f.seekg(PAGE_SIZE, std::ios::beg);
        fileHandle.loadCounterValues();
        // std::cout << "[Success] Open a file" << std::endl;
        return 0;
    }
    else
    {
//        std::cout << "[Error] Fail to open a file" << std::endl;
        return -1;
    }
}

RC PagedFileManager::closeFile(FileHandle &fileHandle) {
    if(fileHandle.getFile().is_open()){
        fileHandle.getFile().close();
        // std::cout << "[Success] close a file! " << std::endl;
        return 0;
    }
    else{
        // std::cout << "[Error] Close a File which not opens. " << std::endl;
        return 0;
    }
}

FileHandle::FileHandle() {
    readPageCounter = 0;
    writePageCounter = 0;
    appendPageCounter = 0;
}

FileHandle::~FileHandle()= default;

RC FileHandle::appendPage(const void *data){
    unsigned int pageNumber = getNumberOfPages() + 1;
    if(_file.seekp(pageNumber*PAGE_SIZE, std::ios::beg))
    {
        // if tellg -> 4096, then write start from 4097.
        _file.write(static_cast<const char *>(data), PAGE_SIZE);
        if(_file.good()){
            loadCounterValues();
            appendPageCounter++;
            saveCounterValues();
            return 0;
        }
        else{
            // std::cout << "[Error] appendPage() write a new page failed." << std::endl;
            return -1;
        }
    }
    else{
        // std::cout << "[Error] appendPage() seek method failed." << std::endl;
        return -1;
    }

}

RC FileHandle::readPage(PageNum pageNum, void *data)
{
//    getNUmberOfPages() already consider the hidden page (minus 1 to get the value), so we just pageNum+1 instead of +2
    if(pageNum+1 > getNumberOfPages())
    {
        // std::cout << "[Error] pageNum exceed total number of pages on readPage()" << std::endl;
        return -1;
    }
    else
    {
//        pageNum+1 -> the first page is the hidden page
        if(_file.seekg((pageNum+1)*PAGE_SIZE, std::ios::beg))
        {
            _file.read(static_cast<char *>(data), PAGE_SIZE);
            if(_file.good()){
                loadCounterValues();
                readPageCounter++;
                saveCounterValues();
                return 0;
            }
            else{
                // std::cout << "[Error] readPage() read a new page failed." << std::endl;
                return -1;
            }
        }
        else{
            // std::cout << "[Error] readPage() seek method failed." << std::endl;
            return -1;
        }

    }
}

RC FileHandle::writePage(PageNum pageNum, const void *data)
{
//    getNUmberOfPages() already consider the hidden page (minus 1 to get the value), so we just pageNUm+1 instead of +2
    if(pageNum+1 > getNumberOfPages())
    {
        // std::cout << "[Error] pageNum exceed total number of pages on writePage()" << std::endl;
        return -1;
    }
    else
    {
//        pageNum+1 -> the first page is the hidden page
        if(_file.seekp((pageNum+1)*PAGE_SIZE, std::ios::beg))
        {
            _file.write((char*)(data), PAGE_SIZE);
            if(_file.good()){
                loadCounterValues();
                writePageCounter++;
                saveCounterValues();
                return 0;
            }
            else{
                 // std::cout << "[Error] writePage() write a new page failed." << std::endl;
                return -1;
            }
        }
        else{
            // std::cout << "[Error] writePage() seek method failed." << std::endl;
            return -1;
        }
    }
}

unsigned FileHandle::getNumberOfPages() {
    // This method returns the total number of pages currently in the file.

    _file.seekg(0, std::ios::end);
//    the first page is hidden page, so that -1
    return unsigned(ceil(_file.tellg() / (double) PAGE_SIZE)-1);

}

RC FileHandle::collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount) {
//    load the counter from the file;
    RC rc;
    rc = loadCounterValues();
    if(rc == 0){
        readPageCount = readPageCounter;
        writePageCount = writePageCounter;
        appendPageCount = appendPageCounter;
        return 0;
    }
    else{
        // std::cout << "[Error]: collectCounterValues -> fail to loadCounterValues." << std::endl;
        return -1;
    }

}

RC FileHandle::saveCounterValues() {
    //    First three 4Byte datas are the counter data.
    _file.seekp(0, std::ios::beg);
    _file.write((char*)&readPageCounter, sizeof(unsigned));
    _file.write((char*)&writePageCounter, sizeof(unsigned));
    _file.write((char*)&appendPageCounter, sizeof(unsigned));
    if(_file.good()){
        return 0;
    }
    else{
        // std::cout << "[Error] saveCounterValues() save Counter failed." << std::endl;
        return -1;
    }
}

RC FileHandle::loadCounterValues() {
//    First three 4-Byte datas are the counter data.
    _file.seekg(0, std::ios::beg);
    _file.read((char*)&readPageCounter, sizeof(unsigned));
    _file.read((char*)&writePageCounter, sizeof(unsigned));
    _file.read((char*)&appendPageCounter, sizeof(unsigned));
    if(_file.good()){
        return 0;
    }
    else{
        // std::cout << "[Error] loadCounterValues() load Counter failed." << std::endl;
        return -1;
    }

}

// if the file is open for the first time, need to make sure the counter is set to 0;
RC FileHandle::initializeCounterValues()
{
//    initialize the first hidden page
    readPageCounter = 0;
    writePageCounter = 0;
    appendPageCounter = 0;
    saveCounterValues();

    if(_file.good()){
        return 0;
    }
    else{
        // std::cout << "[Error] initializeCounterValues() initialize Counter failed." << std::endl;
        return -1;
    }

}

std::fstream& FileHandle::getFile() {
    return _file;
}