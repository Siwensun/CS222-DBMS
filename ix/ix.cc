#include "ix.h"

IndexManager &IndexManager::instance() {
    static IndexManager _index_manager = IndexManager();
    return _index_manager;
}

RC IndexManager::createFile(const std::string &fileName) {
    return PagedFileManager::instance().createFile(fileName);
}

RC IndexManager::destroyFile(const std::string &fileName) {
    return PagedFileManager::instance().destroyFile(fileName);
}

RC IndexManager::openFile(const std::string &fileName, IXFileHandle &ixFileHandle) {
    return PagedFileManager::instance().openFile(fileName, ixFileHandle.getFileHandle());
}

RC IndexManager::closeFile(IXFileHandle &ixFileHandle) {
    return PagedFileManager::instance().closeFile(ixFileHandle.getFileHandle());
}

RC IndexManager::insertEntry(IXFileHandle &ixFileHandle, const Attribute &attribute, const void *key, const RID &rid) {

    unsigned numOfPages = ixFileHandle.getFileHandle().getNumberOfPages();
    RC rc;

    if (numOfPages == 0) {
//        std::cout << "Insert into empty B+ tree" << std::endl;
        rc = appendRootLeafPage(ixFileHandle, attribute, key, rid);
        if(rc != 0){
            std::cout << "[Error] insertEntry -> fail to insert into empty B+ tree." << std::endl;
            return -1;
        }
    }
    else if (numOfPages == 1) {
//        std::cout << "Insert into rootLeaf B+ tree" << std::endl;

        int requiredLength = getRequiredLength(attribute, key, sizeof(rid));
        if(requiredLength == -1){
            // std::cout << "[Error] insertEntry -> fail to get the length of key." << std::endl;
            return -1;
        }
        
        void *page = malloc(PAGE_SIZE);
        rc = ixFileHandle.getFileHandle().readPage(0, page);
        if(rc != 0){
            // std::cout << "[Error] insertEntry -> fail to get the read the root-leaf page." << std::endl;
            return -1;
        }

        leafPageDirectory leafDirectory;
        memcpy(&leafDirectory, page, LEAF_DIR_SIZE);

        if (requiredLength <= leafDirectory.freeSpace) {
            // std::cout << "Insert into root-leaf page." << std::endl;
            rc = insertEntrytoNodeWithoutSplitting(ixFileHandle, ROOT_PAGE, LEAF_FLAG, page, attribute, key, &rid, sizeof(rid));
            if(rc != 0 ){
                // std::cout << "[Error] insertEntry -> fail to insertEntrytoNodeWithoutSplitting." << std::endl;
                return -1;
            }
        }
        else {
            // std::cout << "split the root leaf node and insert." << std::endl;
            rc = splitRootLeafPage(ixFileHandle, attribute, key, rid);
            if(rc != 0 ){
                // std::cout << "[Error] insertEntry -> fail to splitRootLeafPage." << std::endl;
                return -1;
            }
        }

        free(page);
    }
    else {
//        assert(numOfPages >= 4);
//        std::cout << "Insert into normal B+ tree" << std::endl;

        // we search from the root
        void *rootPtr = malloc(PAGE_SIZE);
        ixFileHandle.getFileHandle().readPage(0, rootPtr);

        unsigned rootPageNum = 0;

        memcpy(&rootPageNum, (char *)rootPtr + 4, sizeof(unsigned));

        void *splitKey = malloc(PAGE_SIZE);
        void *splitData = malloc(sizeof(unsigned));
        void *_key = malloc(PAGE_SIZE);
        
        if(attribute.type == TypeInt || attribute.type == TypeReal){
            memcpy(_key, key, 4);
        }
        else{
            int length;
            memcpy(&length, key, sizeof(int));
            memcpy(_key, key, sizeof(int)+length);
        }
        
        bool splitFlag = true;
        rc = insertion(ixFileHandle, attribute, rootPageNum, _key, rid, splitKey, splitData, splitFlag);
        if(rc != 0){
            // std::cout << "[Error]: IndexManager::insertEntry -> fail to insert" << std::endl;
            return -1;
        }

        free(rootPtr);
        free(splitKey);
        free(splitData);
        free(_key);
    }

    return 0;
}

RC IndexManager::insertion(IXFileHandle &ixFileHandle, const Attribute &attribute, unsigned curNode, const void *key,
        const RID &rid, void *splitKey, void *splitData, bool &splitFlag){
    // read this curNode
    void *page = malloc(PAGE_SIZE);
    if (ixFileHandle.getFileHandle().readPage(curNode, page) == 0) {
        int pageFlag;
        memcpy(&pageFlag, page, sizeof(int));

        // First, we check this node is leaf or not
        if (pageFlag == IM_FLAG || pageFlag == ROOT_FLAG) {
            // choose subtree
            unsigned nextNode = 0;
            getNextNode(ixFileHandle, curNode, nextNode, attribute, key);

            // recursively, insert entry
            insertion(ixFileHandle, attribute, nextNode, key, rid, splitKey, splitData, splitFlag);

            if (!splitFlag) {
                // usual case; didn't split child
                free(page);
                return 0;
            }
            else {
                // we split child, must insert *newchildentry
                // First, we check if we could put splitkey in this node
                imPageDirectory imDirectory;
                memcpy(&imDirectory, page, IM_DIR_SIZE);
                int requiredLength = getRequiredLength(attribute, splitKey, sizeof(unsigned));
    
//                int data, pageNum;
//                memcpy(&data, splitKey, sizeof(int));
//                memcpy(&pageNum, splitData, sizeof(int));
//                std::cout << "Insert into im page" << std::endl;
//                std::cout << "splitKey is " << data << "; page num is " << pageNum << std::endl;

                if (imDirectory.freeSpace >= requiredLength) {
                    // this Node has space, put *newchildentry on it, set newchildentry to null, return.
                    // IM_FLAG OR ROOT_FLAG both OK
                    splitFlag = false;
                    insertEntrytoNodeWithoutSplitting(ixFileHandle, curNode, IM_FLAG, page, attribute, splitKey,
                                                      splitData, sizeof(unsigned));
                    
                    free(page);
                    return 0;
                }
                else {
                    splitFlag = true;
                    if (pageFlag == ROOT_FLAG) {
                        splitRootPage(ixFileHandle, attribute, curNode, splitKey, splitData);
//                        std::cout << "[Warning] We split the root page." << std::endl;

                        // it should be the top one, no need to set childptr.
                        free(page);
                        return 0;
                    }
                    else {
                        // IM_PAGE
                        unsigned newimPageNum = 0;
                        void *splitImKey = malloc(PAGE_SIZE);
                        insertEntrytoNodeWithSplitting(ixFileHandle, IM_FLAG, curNode, newimPageNum, page,
                                                       splitImKey, attribute, splitKey, splitData, sizeof(unsigned));

                        memcpy(splitKey, splitImKey, PAGE_SIZE);
                        memcpy(splitData, &newimPageNum, sizeof(unsigned));
    
//                        memcpy(&data, splitKey, sizeof(int));
//                        memcpy(&pageNum, splitData, sizeof(int));
//                        std::cout << "split im page" << std::endl;
//                        std::cout << "splitKey is " << data << "; curNode is " << curNode << "; new page num is " << pageNum << std::endl;
                        
                        free(page);
                        return 0;
                    }
                }
            }
        }
        else if (pageFlag == LEAF_FLAG) {
            // just insert the key and data. First check the freespace.
            leafPageDirectory leafDirectory;
            memcpy(&leafDirectory, page, LEAF_DIR_SIZE);
            int requiredLength = getRequiredLength(attribute, key, sizeof(rid));

            if (leafDirectory.freeSpace >= requiredLength) {
                splitFlag = false;
                //put entry on it, set newchildentry to null, and return
                insertEntrytoNodeWithoutSplitting(ixFileHandle, curNode, LEAF_FLAG, page, attribute, key, &rid,
                                                  sizeof(rid));
                
                free(page);
                return 0;
            } else {
                // split the leaf
//                std::cout << "***************  Split the leaf node  *******************" << std::endl;
                splitFlag = true;
                unsigned newLeafPageNum = 0;
                insertEntrytoNodeWithSplitting(ixFileHandle, LEAF_FLAG, curNode, newLeafPageNum, page, splitKey,
                                               attribute, key, &rid, sizeof(rid));
                
//                leafPageDirectory tempDirectory;
//                memcpy(&tempDirectory, page, LEAF_DIR_SIZE);
//                int data;
//                memcpy(&data, splitKey, sizeof(int));
//                std::cout << "Split leaf page" << std::endl;
//                std::cout << "splitKey is " << data << "; curNode is " << curNode << "; next node is " << tempDirectory.nextNode << "; page num is " << newLeafPageNum << std::endl;
                
                memcpy(splitData, &newLeafPageNum, sizeof(unsigned));
                free(page);
                return 0;
            }
        }
        else {
            // std::cout << "[Error] wrong pageFlag curNode in insertion()." << std::endl;
            free(page);
            return -1;
        }
    }
    else {
        // std::cout << "[Error] Can't read the curNode in insertion()." << std::endl;
        free(page);
        return -1;
    }
}


/*
 * This method is used to search the entry in leafNode, that has the value greater or greater-equal to key, given inclusive
 * return offset as the location.
 */
RC IndexManager::searchEntry(IXFileHandle &ixFileHandle, int &pageNum, int &offset, int &recordId,  const Attribute &attribute, const void *key, bool inclusive){

    unsigned numOfPages = ixFileHandle.getFileHandle().getNumberOfPages();

//    std::cout << "We are in searchEntry(). " << std::endl;

    if(numOfPages == 0){
        // std::cout << "[Error]: search inside a empty B+tree." << std::endl;
        return -1;
    }
    else if(numOfPages == 1){
        pageNum = 0;
        RC rc = searchInsideLeafNode(ixFileHandle, pageNum, offset, recordId, attribute, key, inclusive);
        if(rc != 0){
//            std::cout << "[Error]: searchEntry -> can't find a <key, data> inside a root-leaf node." << std::endl;
            return -1;
        }
    }
    else{
//        std::cout << "searchEntry():  numOfPages > 1. " << std::endl;
        void *rootPtr = malloc(PAGE_SIZE);

        if(ixFileHandle.getFileHandle().readPage(0, rootPtr) != 0){
            // std::cout << "searchEntry():  Can't read the page. " << std::endl;
            return -1;
        }

        unsigned rootPageNum, curNode, nextNode;

        memcpy(&rootPageNum, (char *)rootPtr+4, sizeof(int));
        curNode = rootPageNum;

        while(getNextNode(ixFileHandle, curNode, nextNode, attribute, key) == 0){
            curNode = nextNode;
            pageNum = nextNode;
        }

        RC rc = searchInsideLeafNode(ixFileHandle, curNode, offset, recordId, attribute, key, inclusive);
    
        free(rootPtr);
        
        if(rc != 0){
            // std::cout << "[Error]: searchEntry -> can't find a <key, data>." << std::endl;
            return -1;
        }
    }
    return 0;
}

RC IndexManager::deleteEntry(IXFileHandle &ixFileHandle, const Attribute &attribute, const void *key, const RID &rid) {

    int offset, recordId, pageNum;
    RID tempRid;
    if(searchEntry(ixFileHandle, pageNum, offset, recordId, attribute, key, true) == 0){
        auto *page = (char *)malloc(PAGE_SIZE);
        ixFileHandle.getFileHandle().readPage(pageNum, page);
        leafPageDirectory directory;
        memcpy(&directory, page, LEAF_DIR_SIZE);

        switch (attribute.type){
            case TypeInt:{
                int data, tempData;
                memcpy(&tempData, page+offset, sizeof(int));
                memcpy(&data, key, sizeof(int));
                memcpy(&tempRid, page+offset+ sizeof(int), sizeof(RID));
                if(data == tempData && rid.pageNum == tempRid.pageNum && rid.slotNum == tempRid.slotNum){
                    memmove(page+offset, page+offset+ sizeof(int)+ sizeof(RID), PAGE_SIZE-directory.freeSpace-offset-(sizeof(int)+
                                                                                                                      sizeof(RID)));
                    directory.freeSpace += (sizeof(int)+ sizeof(RID));
                    directory.numOfRecords -= 1;
                    memcpy(page, &directory, LEAF_DIR_SIZE);
                    ixFileHandle.getFileHandle().writePage(pageNum, page);
                    
//                    std::cout << "deleteEntry -> Delete " << recordId << "'th entry inside " << pageNum << " RID is : " << rid.pageNum << "; " << rid.slotNum <<  std::endl;
                    free(page);
                    return 0;
                }
                else{
//                    std::cout << "[Error]: deleteEntry -> can't find such a <key, rid> pair." << std::endl;
                    free(page);
                    return -1;
                }
            }
            case TypeReal:{
                float data, tempData;
                memcpy(&tempData, page+offset, sizeof(float));
                memcpy(&data, key, sizeof(float));
                memcpy(&tempRid, page+offset+ sizeof(float), sizeof(RID));
                if(data == tempData && rid.pageNum == tempRid.pageNum && rid.slotNum == tempRid.slotNum){
                    memmove(page+offset, page+offset+ sizeof(float)+ sizeof(RID), PAGE_SIZE-directory.freeSpace-offset-(sizeof(float)+
                                                                                                                      sizeof(RID)));
                    directory.freeSpace += (sizeof(float)+ sizeof(RID));
                    directory.numOfRecords -= 1;
                    memcpy(page, &directory, LEAF_DIR_SIZE);
                    ixFileHandle.getFileHandle().writePage(pageNum, page);
    
//                    std::cout << "deleteEntry -> Delete " << recordId << "'th entry inside " << pageNum << " RID is : " << rid.pageNum << "; " << rid.slotNum <<  std::endl;
                    free(page);
                    return 0;
                }
                else{
//                    std::cout << "[Error]:  deleteEntry -> can't find such a <key, rid> pair." << std::endl;
                    free(page);
                    return -1;
                }
                break;
            }
            case TypeVarChar:{

                int length, tempLength;
                memcpy(&tempLength, page+offset, sizeof(int));
                memcpy(&length, key, sizeof(int));

                auto *tempData = (char *)malloc(tempLength+1);
                auto *data = (char *)malloc(length+1);

//                std::cout << "offset" << offset << ", " << tempLength << ", " << length << std::endl;

                memcpy(tempData, page+offset+ sizeof(int), tempLength);
                memcpy(data, (char *)key+sizeof(int), length);
                memcpy(&tempRid, page+offset+ sizeof(int)+ tempLength, sizeof(RID));

                tempData[tempLength] = '\0';
                data[length] = '\0';

                if(strcmp(data, tempData) == 0 && rid.pageNum == tempRid.pageNum && rid.slotNum == tempRid.slotNum){
                    memmove(page+offset, page+offset+ sizeof(int)+tempLength+sizeof(RID), PAGE_SIZE-directory.freeSpace-offset-(sizeof(int)+tempLength+
                                                                                                                        sizeof(RID)));
                    directory.freeSpace += (sizeof(int)+tempLength+sizeof(RID));
                    directory.numOfRecords -= 1;
                    memcpy(page, &directory, LEAF_DIR_SIZE);
                    ixFileHandle.getFileHandle().writePage(pageNum, page);
    
                    // std::cout << "deleteEntry -> Delete " << recordId << "'th entry inside " << pageNum << std::endl;
                    free(tempData);
                    free(data);
                    free(page);
                    return 0;
                }
                else{
//                    std::cout << "[Error]: deleteEntry -> can't find such a <key, rid> pair." << std::endl;
                    free(tempData);
                    free(data);
                    free(page);
                    return -1;
                }
                break;
            }
            default:
                break;
        }
    }
    else{
//        std::cout << "[Error]: deleteEntry -> can't find such a rid." << std::endl;
        return -1;
    }

    return 0;
}

RC IndexManager::scan(IXFileHandle &ixFileHandle,
                      const Attribute &attribute,
                      const void *lowKey,
                      const void *highKey,
                      bool lowKeyInclusive,
                      bool highKeyInclusive,
                      IX_ScanIterator &ix_ScanIterator) {

    int pageNum, recordId,  offset = 0;
    
    if(!ixFileHandle.getFileHandle().getFile().is_open()){
//        std::cout << "[Error]: can't scan a non-existing file." << std::endl;
        return -1;
    }
    
    // If we can't find a <key, rid> that satisfies the comparision
    // we set the recordId = numOfRecords and offset is end of valid data which is the start of free space.
    searchEntry(ixFileHandle, pageNum, offset, recordId, attribute, lowKey, lowKeyInclusive);
    
    // initialize the scanIterator with pageNum, offset and recordId which shows from where the scan should start.
    RC rc = ix_ScanIterator.initializeScanIterator(ixFileHandle, attribute, lowKey, highKey, lowKeyInclusive,
                                           highKeyInclusive, pageNum, offset, recordId);
    
    if(rc != 0){
        // std::cout << "[Error]: scan -> initializeScanIterator" << std::endl;
        return -1;
    }
    
    return 0;
    
}

void IndexManager::printBtree(IXFileHandle &ixFileHandle, const Attribute &attribute) const {

    int numOfPages = ixFileHandle.getFileHandle().getNumberOfPages();
    if(numOfPages == 0){
        // std::cerr << "Empty B+ tree" << std::endl;
        return;
    }
    else if(numOfPages == 1){
        printNormalBtree(ixFileHandle, 0, 1, attribute, true);
    }
    else{
        void *rootPtr = malloc(PAGE_SIZE);
        ixFileHandle.getFileHandle().readPage(0, rootPtr);
        int rootPageNum;

        memcpy(&rootPageNum, (char *)rootPtr+4, sizeof(int));

        printNormalBtree(ixFileHandle, rootPageNum, 1, attribute, true);
        
        free(rootPtr);
    }
}

int IndexManager::getRequiredLength(const Attribute &attribute, const void *key, int sizeOfData){
    // get the required length for insertion
    int requiredLength = 0;

    // In this case, we use length(key) + length(rid)
    switch (attribute.type){
        // For INT and REAL: use 4 bytes
        case TypeInt:{
            requiredLength = 4 + sizeOfData;
            break;
        }
        case TypeReal:{
            requiredLength = 4 + sizeOfData;
            break;
        }
        case TypeVarChar:{
            // For VARCHAR: use 4 bytes for the length followed by the characters.
            int length;
            memcpy(&length, key, 4);
            requiredLength = 4 + length + sizeOfData;
            break;
        }
        default:{
            // std::cout << "[Error] wrong attribute type." << std::endl;
            return -1;
        }
    }
    return requiredLength;
}

RC IndexManager::insertEntryToNode(int pageFlag, void *page, const Attribute &attribute, const void *key, const void *data, int sizeOfData){

    int offset;
    int freeSpace, numOfRecords;
    leafPageDirectory leaf_directory;
    imPageDirectory im_directory;

    // initialize freeSpace and numOfRecords
    if(pageFlag == LEAF_FLAG){
        // read from *page
        memcpy(&leaf_directory, page, LEAF_DIR_SIZE);

        freeSpace = leaf_directory.freeSpace;
        numOfRecords = leaf_directory.numOfRecords;
        
        offset = LEAF_DIR_SIZE;
    }
    else if(pageFlag == IM_FLAG || pageFlag == ROOT_FLAG){
        // read
        memcpy(&im_directory, page, IM_DIR_SIZE);

        freeSpace = im_directory.freeSpace;
        numOfRecords = im_directory.numOfRecords;
        // extra 4 bytes to store the P0 pointer(pageNum).
        offset = IM_DIR_SIZE + sizeof(unsigned);
    }
    else{
        // std::cout << "[Error]: insertInNode -> wrong flag" << std::endl;
        return -1;
    }

    // sizeofData should be the length of RID
    switch (attribute.type){
        case TypeInt:{
            // this should not happen
            if(freeSpace < (4 + sizeOfData)){
                // std::cout << "[Error]: insertInsideLeafNode -> can't fit the new insert key." << std::endl;
                return -1;
            }

            int dataInt, tempData;
            int index;

            // get the actual key data to the dataInt
            memcpy(&dataInt, key, 4);

            // find a location to put this entry
            for(index = 0; index < numOfRecords; index++){
                memcpy(&tempData, (char *)page+offset, 4);
                if(dataInt <= tempData)
                    // keep the order that key_n <= key_n+1
                    break;
                offset += (4 + sizeOfData);
            }

            // shift with memory overlap
            memmove((char *)page+offset+4+sizeOfData, (char *)page+offset, PAGE_SIZE-freeSpace-offset);
            // insert the new entry
            memcpy((char *)page+offset, key, 4);
            memcpy((char *)page+offset+4, (char *)data, sizeOfData);

            freeSpace -= (4+sizeOfData);
            numOfRecords += 1;

            break;
        }
        case TypeReal:{
            // this should not happen
            if(freeSpace < (4+sizeOfData)){
                // std::cout << "[Error]: insertInsideLeafNode -> can't fit the new insert key." << std::endl;
                return -1;
            }

            float dataReal, tempData;
            int index;

            // get the actual key data to the dataReal
            memcpy(&dataReal, key, 4);

            for(index = 0; index < numOfRecords; index++){
                memcpy(&tempData, (char *)page+offset, 4);
                if(dataReal <= tempData)
                    // keep the order that key_n <= key_n+1
                    break;
                offset+= (4+sizeOfData);
            }

            // shift with memory overlap
            memmove((char *)page+offset+4+sizeOfData, (char *)page+offset, PAGE_SIZE-freeSpace-offset);
            // insert the new entry
            memcpy((char *)page+offset, key, 4);
            memcpy((char *)page+offset+4, (char *)data, sizeOfData);

            freeSpace -= (4+sizeOfData);
            numOfRecords += 1;

            break;
        }
        case TypeVarChar:{
            int length, tempLength;
            int index;

            memcpy(&length, key, 4);
            auto *dataVarChar = (char *)malloc(length+1);
            memcpy(dataVarChar, (char *)key+4, length);
            dataVarChar[length] = '\0';
            // std::cout << "length is " << length << "; data is " << dataVarChar << std::endl;

            // this should not happen
            if(freeSpace < (4+length+sizeOfData)){
                // std::cout << "[Error]: insertInsideLeafNode -> can't fit the new insert key." << std::endl;
                return -1;
            }

            for(index = 0; index < numOfRecords; index++){
                memcpy(&tempLength, (char *)page+offset, 4);
                auto *tempData = (char *)malloc(tempLength+1);
                memcpy(tempData, (char *)page+offset+4, tempLength);
                tempData[tempLength] = '\0';
                // std::cout << "tempLength is " << tempLength  << "; tempData is" << tempData << std::endl;

                if(strcmp(dataVarChar, tempData) <= 0){
                    // keep the order that key_n <= key_n+1
                    free(tempData);
                    break;
                }

                offset += (4+tempLength+sizeOfData);
                free(tempData);
            }

            // shift with memory overlap
            memmove((char *)page+offset+4+length+sizeOfData, (char *)page+offset, PAGE_SIZE-freeSpace-offset);
            // insert the new entry
            memcpy((char *)page+offset, key, 4+length);
            memcpy((char *)page+offset+4+length, (char *)data, sizeOfData);

            freeSpace -= (4+length+sizeOfData);
            numOfRecords += 1;
            free(dataVarChar);

            break;
        }
        default:
            break;
    }

    // update freespace and numOfRecords
    if(pageFlag == LEAF_FLAG){
        // write back to *page
        leaf_directory.freeSpace = freeSpace;
        leaf_directory.numOfRecords = numOfRecords;

        memcpy(page, &leaf_directory, LEAF_DIR_SIZE);

    }
    else if(pageFlag == IM_FLAG || pageFlag == ROOT_FLAG){
        // write back to *page
        im_directory.freeSpace = freeSpace;
        im_directory.numOfRecords = numOfRecords;

        memcpy(page, &im_directory, IM_DIR_SIZE);
    }

    return 0;
}

RC IndexManager::getSplitInNode(int pageFlag, void *page, const Attribute &attribute, int &splitOffset, int &splitNumOfRecords, void *splitKey, int sizeOfData){

    int startOffset, numOfRecords, freeSpace;
    // get the info about this page
    if(pageFlag == LEAF_FLAG){
        startOffset = LEAF_DIR_SIZE;
        leafPageDirectory directory;
        memcpy(&directory, page, LEAF_DIR_SIZE);
        numOfRecords = directory.numOfRecords;
        freeSpace = directory.freeSpace;
    }
    else if(pageFlag == IM_FLAG || pageFlag == ROOT_FLAG){
        // extra 4 bytes to store the P0 pointer(pageNum).
        startOffset = IM_DIR_SIZE + sizeof(unsigned);
        imPageDirectory directory;
        memcpy(&directory, page, IM_DIR_SIZE);
        numOfRecords = directory.numOfRecords;
        freeSpace = directory.freeSpace;
    }
    else{
        // std::cout << "[Error]: getSplitInNode -> wrong flag." << std::endl;
        return -1;
    }

    switch (attribute.type){
        // we need to make sure the splitOffset is the beginning of an entry which is also the end of the previous entry.
        case TypeInt:{

            splitOffset = (PAGE_SIZE-startOffset-freeSpace) / (4+sizeOfData) / 2 * (4+sizeOfData) + startOffset;
            splitNumOfRecords = (PAGE_SIZE-startOffset-freeSpace) / (4+sizeOfData) / 2;
            memcpy(splitKey, (char *)page+splitOffset, 4);

            break;
        }
        case TypeReal:{

            splitOffset = (PAGE_SIZE-startOffset-freeSpace) / (4+sizeOfData) / 2 * (4+sizeOfData) + startOffset;
            splitNumOfRecords = (PAGE_SIZE-startOffset-freeSpace) / (4+sizeOfData) / 2;
            memcpy(splitKey, (char *)page+splitOffset, 4);

            break;
        }
        case TypeVarChar:{
            splitOffset = startOffset;
            int index, tempLength;
            splitNumOfRecords = 0;
            int pivot = (PAGE_SIZE-startOffset-freeSpace) / 2 + startOffset;

            for(index = 0; index < numOfRecords; index++){

                if(splitOffset > pivot)
                    break;

                memcpy(&tempLength, (char *)page + splitOffset, 4);
                // 4: length of varchar, tempLength: key.
                splitOffset += (4 + tempLength + sizeOfData);
                splitNumOfRecords++;
            }

            int length;
            memcpy(&length, (char *)page+splitOffset, 4);
            memcpy(splitKey, &length, 4);
            memcpy((char *)splitKey+4, (char *)page+splitOffset+4, length);

            break;
        }
        default:
            break;
    }
    return 0;
}

/*
 * Compare splitKey and key to determine which page the <key, data> should be in.
 * This function calls insertEntryToNode to write into page.
*/
RC IndexManager::compareAndInsertToNode(int pageFlag, void *oldPage, void *newPage, const Attribute &attribute, const void *key, void *splitKey, const void *data, int sizeOfData){

    switch (attribute.type){
        case TypeInt:{
            int dataInt, tempDataInt;
            memcpy(&tempDataInt, (char *)splitKey, 4);
            memcpy(&dataInt, key, sizeof(int));
            // insert the new key into newPage or the oldPage if key is LE the splitKey
            if(tempDataInt <= dataInt)
                insertEntryToNode(pageFlag, newPage, attribute, key, data, sizeOfData);
            else
                insertEntryToNode(pageFlag, oldPage, attribute, key, data, sizeOfData);
            break;
        }
        case TypeReal:{
            float dataFloat, tempDataReal;
            memcpy(&tempDataReal, (char *)splitKey, 4);
            memcpy(&dataFloat, key, sizeof(float));
            // insert the new key into newPage or the oldPage if key is LE the splitKey
            if(tempDataReal <= dataFloat)
                insertEntryToNode(pageFlag, newPage, attribute, key, data, sizeOfData);
            else
                insertEntryToNode(pageFlag, oldPage, attribute, key, data, sizeOfData);
            break;
        }
        case TypeVarChar:{
            int length, tempLength;
            memcpy(&length, key, 4);
            auto *dataVarChar = (char *)malloc(length+1);
            memcpy(dataVarChar, (char *)key+4, length);
            dataVarChar[length] = '\0';

            memcpy(&tempLength, (char *)splitKey, 4);
            auto *tempDataVarChar = (char *)malloc(tempLength+1);
            memcpy(tempDataVarChar, (char *)splitKey+4, tempLength);
            tempDataVarChar[tempLength] = '\0';
            // insert the new key into newPage or the oldPage if key is LE the splitKey
            if(strcmp(tempDataVarChar, dataVarChar) <= 0){
                insertEntryToNode(pageFlag, newPage, attribute, key, data, sizeOfData);
            }
            else{
                insertEntryToNode(pageFlag, oldPage, attribute, key, data, sizeOfData);
            }
            free(dataVarChar);
            free(tempDataVarChar);
            break;
        }
        default:{
            return -1;
        }
    }

    return 0;
}

RC IndexManager::redistributeNode(IXFileHandle &ixFileHandle, int pageFlag, void *oldPage, void *newPage, int splitOffset, int splitNumOfRecords, const Attribute &attribute){

    if(pageFlag == IM_FLAG || pageFlag == ROOT_FLAG){
        imPageDirectory oldImDirectory;
        memcpy(&oldImDirectory, (char *)oldPage, IM_DIR_SIZE);
        int splitKeyLength;
        if(attribute.type == TypeVarChar){
            memcpy(&splitKeyLength, (char *)oldPage+splitOffset, 4);
            splitKeyLength += 4;
        } else
            splitKeyLength = 4;


        // split the content of old page into two pages
        // start from the pointer of the <k,p>
        memcpy((char *)newPage+IM_DIR_SIZE, (char *)oldPage+splitOffset+splitKeyLength, PAGE_SIZE-oldImDirectory.freeSpace-splitOffset-splitKeyLength);
        memset((char *)oldPage+splitOffset, 0, PAGE_SIZE-splitOffset);

        // -1 for only remain the pointer of the split key
        imPageDirectory newImDirectory = {oldImDirectory.flag, PAGE_SIZE-(PAGE_SIZE-oldImDirectory.freeSpace-splitOffset-splitKeyLength+IM_DIR_SIZE), oldImDirectory.numOfRecords-splitNumOfRecords-1};
        oldImDirectory.freeSpace = PAGE_SIZE-splitOffset;
        oldImDirectory.numOfRecords = splitNumOfRecords;

        memcpy(newPage, &newImDirectory, IM_DIR_SIZE);
        memcpy(oldPage, &oldImDirectory, IM_DIR_SIZE);
    }
    else if(pageFlag == LEAF_FLAG){
        // get oldLeafDirectory
        leafPageDirectory oldLeafDirectory;
        memcpy(&oldLeafDirectory, (char *)oldPage, LEAF_DIR_SIZE);

        // split the content of old page into two pages
        memcpy((char *)newPage+LEAF_DIR_SIZE, (char *)oldPage+splitOffset, PAGE_SIZE-oldLeafDirectory.freeSpace-splitOffset);
        memset((char *)oldPage+splitOffset, 0, PAGE_SIZE-splitOffset);

        // initialize newLeafDirectory
        leafPageDirectory newLeafDirectory = {LEAF_FLAG, PAGE_SIZE-(PAGE_SIZE-oldLeafDirectory.freeSpace-splitOffset+LEAF_DIR_SIZE), oldLeafDirectory.numOfRecords-splitNumOfRecords, oldLeafDirectory.nextNode};
        // update oldLeafDirectory
        oldLeafDirectory.freeSpace = PAGE_SIZE-splitOffset;
        oldLeafDirectory.numOfRecords = splitNumOfRecords;

        memcpy(newPage, &newLeafDirectory, LEAF_DIR_SIZE);
        memcpy(oldPage, &oldLeafDirectory, LEAF_DIR_SIZE);
    }


    return 0;
}

RC IndexManager::insertEntrytoNodeWithSplitting(IXFileHandle &ixFileHandle, int pageFlag, unsigned pageNum, unsigned &newPageNum, void *page, void *splitKey, const Attribute &attribute, const void *key, const void *data, int sizeOfData){

    RC rc;
    int splitOffset, splitNumOfRecords;
    void *newPage = malloc(PAGE_SIZE);
    memset(newPage, 0, PAGE_SIZE);

    // split and insert
    
    rc = getSplitInNode(pageFlag, page, attribute, splitOffset, splitNumOfRecords, splitKey, sizeOfData);
    if(rc != 0){
        // std::cout << "[Error] insertEntrytoNodeWithSplitting -> getSplitInNode." << std::endl;
        free(newPage);
        return -1;
    }
    rc = redistributeNode(ixFileHandle, pageFlag, page, newPage, splitOffset, splitNumOfRecords, attribute);
    if(rc != 0){
        // std::cout << "[Error] insertEntrytoNodeWithSplitting -> redistributeNode." << std::endl;
        free(newPage);
        return -1;
    }
    rc = compareAndInsertToNode(pageFlag, page, newPage, attribute, key, splitKey, data, sizeOfData);
    if(rc != 0){
        // std::cout << "[Error] insertEntrytoNodeWithSplitting -> compareAndInsertToNode." << std::endl;
        free(newPage);
        return -1;
    }

    // append the new page
    if(ixFileHandle.getFileHandle().appendPage(newPage) == 0){
        newPageNum = ixFileHandle.getFileHandle().getNumberOfPages()-1;
//        std::cout << "new Page number -> " << newPageNum << std::endl;
    }
    else{
        // std::cout << "[Error] insertEntrytoNodeWithSplitting() append the new page" << std::endl;
        free(newPage);
        return -1;
    }
    
    //update oldPage for leafNode -> directory.nextNode = newPageNum;
    if(pageFlag == LEAF_FLAG){
        leafPageDirectory directory;
        memcpy(&directory, page, LEAF_DIR_SIZE);
        directory.nextNode = (int)newPageNum;
        memcpy(page, &directory, LEAF_DIR_SIZE);
//        std::cout << "directory.nextNode: " << directory.nextNode << std::endl;
    }

    if(ixFileHandle.getFileHandle().writePage(pageNum, page) == 0){
        free(newPage);
        return 0;
    }
    else{
        // std::cout << "[Error] insertEntrytoNodeWithSplitting() write the old page" << std::endl;
        free(newPage);
        return -1;
    }

}

/*
 * Insert <key, data> to this node without splitting and write back the page.
*/
RC IndexManager::insertEntrytoNodeWithoutSplitting(IXFileHandle &ixFileHandle, unsigned pageNum, int pageFlag, void *page, const Attribute &attribute, const void *key, const void *data, int sizeOfData){
    RC rc;
    rc = insertEntryToNode(pageFlag, page, attribute, key, data, sizeOfData);
    if(rc != 0){
        // std::cout << "[Error] insertEntrytoNodeWithoutSplitting -> fail to insertEntryToNode" << std::endl;
        return -1;
    }
    rc = ixFileHandle.getFileHandle().writePage(pageNum, page);
    if(rc != 0){
        // std::cout << "[Error] insertEntrytoNodeWithoutSplitting -> fail to write page" << std::endl;
        return -1;
    }
    return 0;
}

RC IndexManager::appendRootLeafPage(IXFileHandle &ixFileHandle, const Attribute &attribute, const void *key, const RID &rid){

    void *page = malloc(PAGE_SIZE);
    leafPageDirectory directory = {LEAF_FLAG, PAGE_SIZE-LEAF_DIR_SIZE, 0, -1};
    memset(page, 0, PAGE_SIZE);
    memcpy(page, &directory, LEAF_DIR_SIZE);

    RC rc = insertEntryToNode(LEAF_FLAG, page, attribute, key, &rid, sizeof(rid));
    if(rc != 0){
        // std::cout << "[Error] appendRootLeafPage -> insertEntryToNode" << std::endl;
        return -1;
    }
    
    ixFileHandle.getFileHandle().appendPage(page);
    free(page);
    return 0;
}

RC IndexManager::splitRootLeafPage(IXFileHandle &ixFileHandle, const Attribute &attribute, const void *key, const RID &rid){

    void *leftPage = malloc(PAGE_SIZE);
    void *rootPage = malloc(PAGE_SIZE);
    void *rootPtrPage = malloc(PAGE_SIZE);
    void *splitKey = malloc(PAGE_SIZE);
    memset(leftPage, 0, PAGE_SIZE);
    memset(rootPage, 0, PAGE_SIZE);
    memset(rootPtrPage, 0, PAGE_SIZE);
    memset(splitKey, 0, PAGE_SIZE);
    
    RC rc;

    ixFileHandle.getFileHandle().readPage(0, leftPage);
    ixFileHandle.getFileHandle().appendPage(leftPage);

    int leftPageNum = (int)(ixFileHandle.getFileHandle().getNumberOfPages()-1);
     unsigned rightageNum, rootPageNum;

    // pageNum should be 1.
//    std::cout<< "leftPageNum is " << leftPageNum << std::endl;

    rc = insertEntrytoNodeWithSplitting(ixFileHandle, LEAF_FLAG, leftPageNum, rightageNum, leftPage, splitKey, attribute, key, &rid,
                                   sizeof(rid));
    if(rc != 0){
        // std::cout << "[Error] splitRootLeafPage -> insertEntrytoNodeWithSplitting." << std::endl;
        return -1;
    }
    
    rc = generateNewRootNode(ixFileHandle, leftPageNum, rightageNum, rootPage, attribute, splitKey);
    if(rc != 0){
        // std::cout << "[Error] splitRootLeafPage -> generateNewRootNode." << std::endl;
        return -1;
    }
    
    // write the rootPage back into disk.
    ixFileHandle.getFileHandle().appendPage(rootPage);
    rootPageNum = ixFileHandle.getFileHandle().getNumberOfPages()-1;

    // we add a ROOT_PTR page which always points to the root page.
    int rootPtrFlag = ROOT_PTR_FLAG;
    // put the pointer flag
    memcpy((char *)rootPtrPage, &rootPtrFlag, sizeof(int));
    // put the real pointer-> the actual pageNum
    memcpy((char *)rootPtrPage+4, &rootPageNum, sizeof(unsigned));

    ixFileHandle.getFileHandle().writePage(0, rootPtrPage);

    free(leftPage);
    free(rootPage);
    free(rootPtrPage);
    free(splitKey);

    return 0;
}


RC IndexManager::splitRootPage(IXFileHandle &ixFileHandle, const Attribute &attribute, unsigned rootPageNum, const void *key, const void *data){

    void *leftPage = malloc(PAGE_SIZE);
    void *rootPage = malloc(PAGE_SIZE);
    void *splitRootKey = malloc(PAGE_SIZE);
    
    memset(leftPage, 0, PAGE_SIZE);
    memset(rootPage, 0, PAGE_SIZE);
    memset(splitRootKey, 0, PAGE_SIZE);
    
    ixFileHandle.getFileHandle().readPage(rootPageNum, leftPage);
    // convert the leftPage from root to imNode
    imPageDirectory imDirectory;
    memcpy(&imDirectory, (char *)leftPage, IM_DIR_SIZE);
    imDirectory.flag = IM_FLAG;
    ixFileHandle.getFileHandle().appendPage(leftPage);
    int leftPageNum = (int)(ixFileHandle.getFileHandle().getNumberOfPages()-1);

    unsigned newimPageNum;
    insertEntrytoNodeWithSplitting(ixFileHandle, IM_FLAG, leftPageNum, newimPageNum, leftPage, splitRootKey, attribute, key, data,
                                   sizeof(unsigned));

    // generate a new root page, which stores two pointers pointing to upper left and right pages.
    generateNewRootNode(ixFileHandle, leftPageNum, newimPageNum, rootPage, attribute, splitRootKey);

    // re-write the root page
    ixFileHandle.getFileHandle().writePage(rootPageNum, rootPage);

    free(leftPage);
    free(rootPage);
    free(splitRootKey);

    return 0;
}


RC IndexManager::generateNewRootNode(IXFileHandle &ixFileHandle, unsigned leftPageNum, unsigned rightPageNum, void *rootPage, const Attribute &attribute, const void *splitkey){

    imPageDirectory rootImDirectory = {ROOT_FLAG, PAGE_SIZE-IM_DIR_SIZE, 0};

    memcpy((char *)rootPage+IM_DIR_SIZE, &leftPageNum, sizeof(unsigned));
    rootImDirectory.freeSpace -= sizeof(unsigned);
    memcpy((char *)rootPage, &rootImDirectory, IM_DIR_SIZE);

    // std::cout << "insert into root node" << std::endl;
    RC rc = insertEntryToNode(ROOT_FLAG, rootPage, attribute, splitkey, &rightPageNum, sizeof(rightPageNum));
    if(rc != 0){
        // std::cout << "[Error] generateNewRootNode -> insertEntryToNode." << std::endl;
        return -1;
    }
    // std::cout << "finish insert into root node" << std::endl;

    return 0;
}

/*
 * get the NextNode(pageNum) when traversing the B+tree.
 * This function is for im Node.
*/
RC IndexManager::getNextNode(IXFileHandle &ixFileHandle, unsigned curNode, unsigned &nextNode, const Attribute &attribute, const void *key){
    void *imPage = malloc(PAGE_SIZE);
    ixFileHandle.getFileHandle().readPage(curNode, imPage);

    int pageFlag;
    memcpy(&pageFlag, imPage, sizeof(int));

    if(pageFlag == LEAF_FLAG){
//        std::cout << "[Warning] getNextNode -> arrive leaf node." << std::endl;
        free(imPage);
        return 1;
    }

    imPageDirectory directory;
    memcpy(&directory, imPage, IM_DIR_SIZE);
    // P0 ptr which is a pageNum
    int startOffset = IM_DIR_SIZE + sizeof(unsigned);
    
    if(key == NULL){
        memcpy(&nextNode, (char *)imPage+IM_DIR_SIZE, sizeof(unsigned));
//        std::cout << "nextNode is " << nextNode << std::endl;
        free(imPage);
        return 0;
    }

    switch (attribute.type){
        case TypeInt:{
            int data, tempData;
            int index;
            memcpy(&data, key, sizeof(data));
            for(index = 0; index < directory.numOfRecords; index++){
                memcpy(&tempData, (char *)imPage+startOffset, sizeof(tempData));
                if(data <= tempData){
                    // get the left pointer if data if less than the tempData
                    memcpy(&nextNode, (char *)imPage+startOffset-sizeof(unsigned), sizeof(unsigned));
                    free(imPage);
                    return 0;
                }
                // key: 4B, ptr: 4B
                startOffset += (sizeof(int) + sizeof(unsigned));
            }
            memcpy(&nextNode, (char *)imPage+startOffset-sizeof(unsigned), sizeof(unsigned));
            free(imPage);
            return 0;
        }
        case TypeReal:{
            float data, tempData;
            int index;
            memcpy(&data, key, sizeof(data));
            for(index = 0; index < directory.numOfRecords; index++){
                memcpy(&tempData, (char *)imPage+startOffset, sizeof(tempData));
                if(data <= tempData){
                    memcpy(&nextNode, (char *)imPage+startOffset-sizeof(unsigned), sizeof(unsigned));
                    free(imPage);
                    return 0;
                }
                // key: 4B, ptr: 4B
                startOffset += (sizeof(int) + sizeof(unsigned));
            }
            memcpy(&nextNode, (char *)imPage+startOffset-sizeof(unsigned), sizeof(unsigned));
            free(imPage);
            return 0;
        }
        case TypeVarChar:{
            int index, length, tempLength;
            memcpy(&length, key, 4);
            auto *data = (char *)malloc(length+1);
            memcpy(data, (char *)key+4, length);
            data[length] = '\0';

            for(index = 0; index < directory.numOfRecords; index++){
                memcpy(&tempLength, (char *)imPage+startOffset, 4);
                auto *tempData = (char *)malloc(tempLength+1);
                memcpy(tempData, (char *)imPage+startOffset+4, tempLength);
                tempData[tempLength] = '\0';

                if(strcmp(data, tempData) <= 0){
                    memcpy(&nextNode, (char *)imPage+startOffset-4, 4);
                    free(tempData);
                    free(data);
                    free(imPage);
                    return 0;
                }
                startOffset += (4+tempLength+4);
                free(tempData);
            }
            memcpy(&nextNode, (char *)imPage+startOffset-4, 4);
            free(data);
            free(imPage);
            return 0;
        }
    }
    free(imPage);
    return 0;
}

RC IndexManager::searchInsideLeafNode(IXFileHandle &ixFileHandle, unsigned curNode, int &offset, int &recordId, const Attribute &attribute, const void *key, bool inclusiveKey){

//    std::cout << "We are in searchInsideLeafNode(). " << std::endl;

    int pageFlag;
    auto *page = (char *)malloc(PAGE_SIZE);
    ixFileHandle.getFileHandle().readPage(curNode, page);
    memcpy(&pageFlag, page, sizeof(int));
    if(pageFlag != LEAF_FLAG){
//        std::cout << "[Error]: searchInsideLeafNode -> this node is not a leaf node." << std::endl;
        return -1;
    }

    leafPageDirectory directory;
    memcpy(&directory, page, LEAF_DIR_SIZE);

    int startOffset = LEAF_DIR_SIZE;
    int numOfRecords = directory.numOfRecords;

//    std::cout << "In searchInsideLeafNode(): numOfRecords: " << numOfRecords << std::endl;
    
    if(key == NULL){
        offset = startOffset;
        recordId = 0;
        free(page);
        return 0;
    }

    switch (attribute.type){
        case TypeInt:{
            int data = 0;
            int tempData = 0;
            int index = 0;
            
            memcpy(&data, key, sizeof(int));
            for(index = 0; index < numOfRecords; index++){
                memcpy(&tempData, (char *)page+startOffset, sizeof(tempData));
                if((inclusiveKey && data <= tempData) || data < tempData){
                    // get the left pointer if data if less than or equal to the tempData
                    offset = startOffset;
                    recordId = index;
                    free(page);
                    return 0;
                }
                startOffset += (4+ sizeof(RID));
            }
            break;
        }
        case TypeReal:{
            float data, tempData;
            int index;
            memcpy(&data, key, 4);
            for(index = 0; index < numOfRecords; index++){
                memcpy(&tempData, (char *)page+startOffset, 4);
                if((inclusiveKey && data <= tempData) || data < tempData){
                    // get the left pointer if data if less than or equal to the tempData
                    offset = startOffset;
                    recordId = index;
                    free(page);
                    return 0;
                }
                startOffset += (4+ sizeof(RID));
            }
            break;
        }
        case TypeVarChar:{
            int index, length, tempLength;
            memcpy(&length, key, 4);
            auto *data = (char *)malloc(length+1);
            memcpy(data, (char *)key+4, length);
            data[length] = '\0';

            for(index = 0; index < numOfRecords; index++){
                memcpy(&tempLength, (char *)page+startOffset, 4);
                auto *tempData = (char *)malloc(tempLength+1);
                memcpy(tempData, (char *)page+startOffset+4, tempLength);
                tempData[tempLength] = '\0';

                if((inclusiveKey && strcmp(data, tempData) <= 0) || strcmp(data, tempData) < 0){
                    // get the left pointer if data if less than or equal to the tempData
                    offset = startOffset;
                    recordId = index;
                    free(tempData);
                    free(data);
                    free(page);
                    return 0;
                }
                
                startOffset += (4+tempLength+ sizeof(RID));
                free(tempData);
            }
            break;
        }
        default:
            break;
    }
    offset = startOffset;
    recordId = numOfRecords;
    free(page);
//    std::cout << "[Error]: searchInsideLeafNode -> can't find a value." << std::endl;
    return -1;
}

/*
 * Print current Node specified by pageFlag
 */
RC IndexManager::printNode(IXFileHandle &ixFileHandle, int pageFlag, void *page, int numOfRecords, int level, const Attribute &attribute) const{
    int startOffset = (pageFlag == LEAF_FLAG)? LEAF_DIR_SIZE : (IM_DIR_SIZE+4);
    int sizeOfData = (pageFlag == LEAF_FLAG) ? sizeof(RID) : sizeof(int);

//    auto *indent = (char *)malloc(level);
//    memset(indent, 9, level-1);
//    indent[level-1] = '\0';

    std::cout << "\"keys\": [";
    for(int index = 0; index < numOfRecords; index++){
        std::cout << "\"";
        switch (attribute.type){
            case TypeInt:{
                int data;
                memcpy(&data, (char *)page+startOffset, sizeof(int));
                startOffset += (sizeof(int));
                std::cout << data;
                break;
            }
            case TypeReal:{
                float data;
                memcpy(&data, (char *)page+startOffset, sizeof(float));
                startOffset += (sizeof(float));
                std::cout << data;
                break;
            }
            case TypeVarChar:{
                int length;
                memcpy(&length, (char *)page+startOffset, sizeof(int));
                auto *data = (char *)malloc(length+1);
                memcpy(data, (char *)page+startOffset+ sizeof(int), length);
                data[length] = '\0';
                startOffset += (sizeof(int)+length);
                std::cout << data;
                free(data);
                break;
            }
            default:
                break;
        }
        if(pageFlag == LEAF_FLAG){
            RID rid;
            memcpy(&rid, (char *)page+startOffset, sizeOfData);
            std::cout << ":[(" << rid.pageNum << ", " << rid.slotNum << ")]";
        }
        startOffset += sizeOfData;
        if(index == numOfRecords-1){
            std::cout << "\"";
        }
        else{
            std::cout << "\",";
        }
    }
    if(pageFlag == LEAF_FLAG){
        leafPageDirectory directory;
        memcpy(&directory, page, LEAF_DIR_SIZE);
        // std::cout << "(" << directory.numOfRecords << "; " << directory.freeSpace << "; " << directory.nextNode << ")";
    }
    std::cout << "]";
    // free(indent);
    return 0;
}

RC IndexManager::printNormalBtree(IXFileHandle &ixFileHandle, int curNode, int level, const Attribute &attribute, bool isLastKey) const{

    void *page = malloc(PAGE_SIZE);
    ixFileHandle.getFileHandle().readPage(curNode, page);

    int pageFlag, numOfRecords;
    memcpy(&pageFlag, page, sizeof(int));

    if(pageFlag == LEAF_FLAG){
        leafPageDirectory directory;
        memcpy(&directory, page, LEAF_DIR_SIZE);
        numOfRecords = directory.numOfRecords;
    }
    else if(pageFlag == IM_FLAG || pageFlag == ROOT_FLAG){
        imPageDirectory directory;
        memcpy(&directory, page, IM_DIR_SIZE);
        numOfRecords = directory.numOfRecords;
    } else{
        // std::cout << "[Error]: printNormalBtree -> wrong pageFlag." << std::endl;
        return -1;
    }

//    auto *indent = (char *)malloc(level);
//    // 9 ASCII Tab
//    memset(indent, 9, level-1);
//    indent[level-1] = '\0';

    std::string indent;
    for(int i=0; i < level - 1; i++){
        indent += "    ";
    }

    if(level == 1){
        // it is the root. Newline.
        std::cout << indent << "{" << std::endl;
    }
    else{
        std::cout << indent << "{";
    }

    printNode(ixFileHandle, pageFlag, page, numOfRecords, level, attribute);

    if(pageFlag == IM_FLAG || pageFlag == ROOT_FLAG){
        std::cout << "," << std::endl;
        std::cout << indent << "\"children\":[" << std::endl;
        int index, nextNode;
        int offset = IM_DIR_SIZE;

        // index <= numRecords -> there are numOFRecords + 1 pointers inside the intermediate node
        for(index = 0; index <= numOfRecords; index++){
            switch (attribute.type){
                case TypeInt:{
                    memcpy(&nextNode, (char *)page+offset, sizeof(int));
                    offset += (sizeof(int)+ sizeof(int));
                    break;
                }
                case TypeReal:{
                    memcpy(&nextNode, (char *)page+offset, sizeof(int));
                    offset += (sizeof(int)+ sizeof(int));
                    break;
                }
                case TypeVarChar:{
                    memcpy(&nextNode, (char *)page+offset, sizeof(int));
                    int length;
                    memcpy(&length, (char *)page+offset + sizeof(int), sizeof(int));
                    offset += (sizeof(int) + sizeof(int) + length);
                    break;
                }
            }

            printNormalBtree(ixFileHandle, nextNode, level + 1, attribute, index == numOfRecords);
        }

        std::cout << indent << "]";

    }

    if(level == 1){
        // it is the root. Newline.
        std::cout << std::endl;
    }

    if(isLastKey){
        std::cout << "}" << std::endl;
    }
    else{
        std::cout << "}," << std::endl;
    }

    free(page);
    // free(indent);
    return 0;
}

IX_ScanIterator::IX_ScanIterator() {}

IX_ScanIterator::~IX_ScanIterator() {
}

RC IX_ScanIterator::initializeScanIterator(IXFileHandle &ixFileHandle,
                                           const Attribute &attribute,
                                           const void *lowKey,
                                           const void *highKey,
                                           bool lowKeyInclusive,
                                           bool highKeyInclusive,
                                           int curNode,
                                           int curOffset,
                                           int curRecordId){
    this->ixFileHandlePtr = &ixFileHandle;
    this->attribute = attribute;
    this->lowKey = lowKey;
    this->highKey = highKey;
    this->lowKeyInclusive = lowKeyInclusive;
    this->highKeyInclusive = highKeyInclusive;
    this->curNode = curNode;
    this->curOffset = curOffset;
    this->curRecordId = curRecordId;
    
    this->curPage = (char *)malloc(PAGE_SIZE);
    this->preOffset = curOffset;
    
    ixFileHandlePtr->getFileHandle().readPage(curNode, curPage);
    memcpy(&curLeafPageDir, curPage, LEAF_DIR_SIZE);
    
    return 0;
}

RC IX_ScanIterator::getNextEntry(RID &rid, void *key) {

    if(curNode == -1){
        // No Next Page.
//        std::cout << "[Warning]: scan terminates." << std::endl;
        return IX_EOF;
    }
    
    // update curRecordId and curOffset
    RC rc = checkDeleteToUpdateCurOffset(curPage);
    if(rc != 0){
        return -1;
    }
    
    //
    if(curRecordId >= curLeafPageDir.numOfRecords && curLeafPageDir.numOfRecords != 0 && curLeafPageDir.nextNode == -1){
        // std::cout << "[warning] -> getNextEntry -> can't get an entry match the comparison." << std::endl;
        return  IX_EOF;
    } else{
        // There may be some node without any records, then we check the next node.
        while (curLeafPageDir.numOfRecords == 0) {
            if (curLeafPageDir.nextNode != -1) {
                curNode = curLeafPageDir.nextNode;
                ixFileHandlePtr->getFileHandle().readPage(curNode, curPage);
                memcpy(&curLeafPageDir, curPage, LEAF_DIR_SIZE);
                curOffset = LEAF_DIR_SIZE;
                preOffset = LEAF_DIR_SIZE;
                curRecordId = 0;
            }
            if(curLeafPageDir.nextNode == IX_EOF) {
//                std::cout << "scan terminate." << std::endl;
                return IX_EOF;
            }
        }
    }
    
//    std::cout << "curRecordId: " << curRecordId << " " << "; curLeafPageDir.numOfRecords: " << curLeafPageDir.numOfRecords << std::endl;
    
    if(rc != 0){
        // std::cout << "[Error] -> getNextEntry -> checkDeleteToUpdateCurOffset" << std::endl;
        return -1;
    }

    switch (attribute.type){
        case TypeInt:{
            int data, highData;
            memcpy(&data, curPage+curOffset, sizeof(int));
            if(highKey != NULL){
                memcpy(&highData, highKey, sizeof(int));
            }
            if((highKey == NULL) || (highKeyInclusive && data <= highData) || (data < highData)){
                memcpy(key, curPage+curOffset, sizeof(int));
                memcpy(&rid, curPage + curOffset + sizeof(int), sizeof(RID));
                curRecordId ++;
                if(curRecordId == curLeafPageDir.numOfRecords){
                    curNode = curLeafPageDir.nextNode;
    
                    ixFileHandlePtr->getFileHandle().readPage(curNode, curPage);
                    memcpy(&curLeafPageDir, curPage, LEAF_DIR_SIZE);
                    
                    curOffset = LEAF_DIR_SIZE;
                    preOffset = LEAF_DIR_SIZE;
                    curRecordId = 0;
//                    std::cout << "[Warning]: Last entry in this page." << std::endl;
                } else{
                    preOffset = curOffset;
                    preRid = rid;
                    curOffset += (sizeof(int) + sizeof(RID));
//                    std::cout << "[Scan] get an entry. -> curNode: " << curNode << "; rid.pageNum: " << rid.pageNum << std::endl;
                }
                return 0;
            } else{
//                std::cout << "[Warning]: scan terminates." << std::endl;
                return IX_EOF;
            }
        }
        case TypeReal:{
            float data, highData;
            memcpy(&data, curPage+curOffset, sizeof(float));
            if(highKey != NULL){
                memcpy(&highData, highKey, sizeof(float));
            }
            if((highKey == NULL) || (highKeyInclusive && data <= highData) || (data < highData)){
                memcpy(key, curPage+curOffset, sizeof(float));
                memcpy(&rid, curPage+curOffset+ sizeof(float), sizeof(RID));
                
                // if this record is the last entry inside this leaf page, update all the curInfo
                curRecordId ++;
                if(curRecordId == curLeafPageDir.numOfRecords){
                    curNode = curLeafPageDir.nextNode;
    
                    ixFileHandlePtr->getFileHandle().readPage(curNode, curPage);
                    memcpy(&curLeafPageDir, curPage, LEAF_DIR_SIZE);
                    
                    curOffset = LEAF_DIR_SIZE;
                    preOffset = LEAF_DIR_SIZE;
                    curRecordId = 0;
//                    ixFileHandlePtr->getFileHandle().readPage(curNode, curPage);
//                    memcpy(&curLeafPageDir, curPage, LEAF_DIR_SIZE);
                } else{
                    preOffset = curOffset;
                    preRid = rid;
                    curOffset += (sizeof(float) + sizeof(RID));
                }
                return 0;
            } else{
//                std::cout << "[Warning]: scan terminates." << std::endl;
                return IX_EOF;
            }
        }
        case TypeVarChar:{
            int length, highLength;
            memcpy(&length, curPage+curOffset, sizeof(int));
            auto *data = (char *)malloc(length+1);
            memcpy(data, curPage+curOffset+ sizeof(int), length);
            data[length] = '\0';
        
            
            
            char *highData;
            if(highKey != NULL){
                memcpy(&highLength, highKey, sizeof(int));
                highData = (char *)malloc(highLength+1);
                memcpy(highData, (char *)highKey+ sizeof(int), highLength);
                highData[highLength] = '\0';
            }
    
            
            if((highKey == NULL) || (highKeyInclusive && strcmp(data, highData) <= 0) || (strcmp(data, highData) < 0)){
                memcpy(key, curPage+curOffset, sizeof(int)+length);
                memcpy(&rid, curPage+curOffset+ sizeof(int)+length, sizeof(RID));
                curRecordId ++;
                if(curRecordId == curLeafPageDir.numOfRecords){
                    curNode = curLeafPageDir.nextNode;
    
                    ixFileHandlePtr->getFileHandle().readPage(curNode, curPage);
                    memcpy(&curLeafPageDir, curPage, LEAF_DIR_SIZE);
                    
                    curOffset = LEAF_DIR_SIZE;
                    preOffset = LEAF_DIR_SIZE;
                    curRecordId = 0;
//                    ixFileHandlePtr->getFileHandle().readPage(curNode, curPage);
//                    memcpy(&curLeafPageDir, curPage, LEAF_DIR_SIZE);
                } else{
                    preOffset = curOffset;
                    preRid = rid;
                    curOffset += (sizeof(int) + length + sizeof(RID));
                }
                free(data);
                if(highKey != NULL) {
                    free(highData);
                }
                return 0;
            } else{
//                std::cout << "[Warning]: scan terminates." << std::endl;
                free(data);
                if(highKey != NULL) {
                    free(highData);
                }
                return IX_EOF;
            }
        }
        default:
            break;
    }
    // std::cout << "[Error] getNextEntry" << std::endl;
    return -1;
}

RC IX_ScanIterator::checkDeleteToUpdateCurOffset(void *page){
    if(preOffset == curOffset){
        // initialized or start in a new page
        // no check needed
        return 0;
    }
    if(preOffset < curOffset){
        
        RID tempRid;
        switch (attribute.type){
            case TypeInt:{
                memcpy(&tempRid, (char *)page+preOffset+ sizeof(int), sizeof(RID));
                break;
            }
            case TypeReal:{
                memcpy(&tempRid, (char *)page+preOffset+ sizeof(float), sizeof(RID));
                break;
            }
    
            case TypeVarChar:{
                int length;
                memcpy(&length, (char *)page+preOffset, sizeof(int));
                memcpy(&tempRid, (char *)page+preOffset+ sizeof(int) + length, sizeof(RID));
                break;
            }
            default:{
                // std::cout << "[Error] -> checkDeleteToUpdateCurOffset -> attribute.type " << std::endl;
                return -1;
            }
        }
        if(tempRid.pageNum == preRid.pageNum && tempRid.slotNum == preRid.slotNum){
            // no delete happen
            return 0;
        }
        else{
            // std::cout << "checkDeleteToUpdateCurOffset -> delete happens." << std::endl;
            curOffset = preOffset;
            curRecordId -= 1;
            return 0;
        }
        //pass
    } else{
        // std::cout << "[Error] checkDeleteToUpdateCurOffset -> preOffset > curOffset" << std::endl;
        return -1;
    }
    
}

RC IX_ScanIterator::close() {
    // This method should terminate the index scan.
    IndexManager::instance().closeFile(*ixFileHandlePtr);
    this->ixFileHandlePtr = nullptr;
    this->lowKey = nullptr;
    this->highKey = nullptr;
    free(this->curPage);
    this->curPage = nullptr;

    return 0;
}

IXFileHandle &IXFileHandle::instance() {
    static IXFileHandle ixFileHandle = IXFileHandle();
    return ixFileHandle;
}

IXFileHandle::IXFileHandle() {
    ixReadPageCounter = 0;
    ixWritePageCounter = 0;
    ixAppendPageCounter = 0;
}

IXFileHandle::~IXFileHandle() {
}

RC IXFileHandle::collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount) {
    RC rc = fileHandle.collectCounterValues(readPageCount, writePageCount, appendPageCount);
    if(rc == 0){
        return 0;
    }
    else{
        // std::cout << "[Error]: collectCounterValues -> fail to collectCounterValues." << std::endl;
        return -1;
    }
}

FileHandle& IXFileHandle::getFileHandle() {
    return fileHandle;
}

