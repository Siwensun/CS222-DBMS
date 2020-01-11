#include "rm.h"

RC RM_ScanIterator::getNextTuple(RID &rid, void *data) {
    RC rc = _rbfmScanItearator.getNextRecord(rid, data);
    if(rc == IX_EOF){
//        std::cout << "[Warning]: getNextEntry -> scan terminates" << std::endl;
        return RM_EOF;
    }
    return rc;
};

RC RM_ScanIterator::close() {
    _rbfmScanItearator.close();
    return 0;
};

RC RM_IndexScanIterator::getNextEntry(RID &rid, void *key) {
    RC rc = _ixScanItearator.getNextEntry(rid, key);
    if(rc == IX_EOF){
//        std::cout << "[Warning]: getNextEntry -> scan terminates" << std::endl;
        return RM_EOF;
    }
    return rc;
};    // Get next matching entry

RC RM_IndexScanIterator::close() {
    _ixScanItearator.close();
    return 0;
};                        // Terminate index scan

RelationManager *RelationManager::_relation_manager = nullptr;

RelationManager &RelationManager::instance() {
    static RelationManager _relation_manager = RelationManager();
    return _relation_manager;
}

RelationManager::RelationManager(){
    this->_rbfm = &RecordBasedFileManager::instance();
    this->_im = &IndexManager::instance();
    
    this->prepareTablesDescriptor();
    this->prepareColumnsDescriptor();
    this->prepareIndexesDescriptor();
    
    
    
};

RelationManager::~RelationManager() { delete _relation_manager; }

RelationManager::RelationManager(const RelationManager &) = default;

RelationManager &RelationManager::operator=(const RelationManager &) = default;

RC RelationManager::createCatalog() {
    
    FileHandle fileHandle;
    RC rc;
    
    if((_rbfm->createFile(TABLE_NAME) != 0) || _rbfm->createFile(COLUMN_NAME) != 0 || _rbfm->createFile(INDEX_NAME) != 0){
        // std::cout << "[Error]: createCatalog() -> system Catalogs already exist." << std::endl;
        return -1;
    }
    
    _rbfm->openFile(TABLE_NAME, fileHandle);
    rc = insertRecordToTables(fileHandle, *_rbfm, 1, TABLE_NAME, TABLE_NAME);
    if(rc != 0){
        // std::cout << "[Error] createCatalog -> insertRecordToTables" << std::endl;
        return -1;
    }
    rc = insertRecordToTables(fileHandle, *_rbfm, 2, COLUMN_NAME, COLUMN_NAME);
    insertRecordToTables(fileHandle, *_rbfm, 3, INDEX_NAME, INDEX_NAME);
    _rbfm->closeFile(fileHandle);
    
    _rbfm->openFile(COLUMN_NAME, fileHandle);
    insertDescriptorToColumns(fileHandle, *_rbfm, 1, _tablesDescriptor);
    insertDescriptorToColumns(fileHandle, *_rbfm, 2, _columnsDescriptor);
    insertDescriptorToColumns(fileHandle, *_rbfm, 3, _indexesDescriptor);
    _rbfm->closeFile(fileHandle);
    
    
    _rbfm->openFile(TABLE_NAME, fileHandle);
    fileHandle.getFile().seekp(3* sizeof(unsigned), std::ios::beg);
    // set the num of table to 0;
    int num = 3;
    fileHandle.getFile().write((char *)&num, sizeof(int));
    _rbfm->closeFile(fileHandle);
    
    return 0;
}

RC RelationManager::deleteCatalog() {
    // destroy Tables Catalog
    _rbfm->destroyFile(TABLE_NAME);
    //delete Columns Catalog
    _rbfm->destroyFile(COLUMN_NAME);
    // delete Indexes Catalog
    _rbfm->destroyFile(INDEX_NAME);
    return 0;
}

RC RelationManager::createTable(const std::string &tableName, const std::vector<Attribute> &attrs) {
    
    FileHandle fileHandle;
    int tableID;
    void *data = malloc(PAGE_SIZE);
    
    // 1. check table file name
    if(tableName == TABLE_NAME || tableName == COLUMN_NAME || tableName == INDEX_NAME){
        // std::cout << "[Error]: createTable -> can't create a Catalog file." << std::endl;
        return -1;
    }
    
    // 2. Table ID
    _rbfm->openFile(TABLE_NAME, fileHandle);
    //retrive the current num of tables
    fileHandle.getFile().seekg(3* sizeof(unsigned), std::ios::beg);
    fileHandle.getFile().read((char *)&tableID, sizeof(int));
    
    tableID += 1;
//    std::cout << "table ID is " << tableID << std::endl;
    
    // write back num of tables
    fileHandle.getFile().seekg(3* sizeof(unsigned), std::ios::beg);
    fileHandle.getFile().write((char *)&tableID, sizeof(int));
    _rbfm->closeFile(fileHandle);
    
    // 3. create new table
    _rbfm->createFile(tableName);
    
    // 4. insert into Tables
    _rbfm->openFile(TABLE_NAME, fileHandle);
    insertRecordToTables(fileHandle, *_rbfm, tableID, tableName, tableName);
    _rbfm->closeFile(fileHandle);
    
    // 5. insert into Columns
    _rbfm->openFile(COLUMN_NAME, fileHandle);
    insertDescriptorToColumns(fileHandle, *_rbfm, tableID, attrs);
    _rbfm->closeFile(fileHandle);
    
    free(data);
    return 0;
    
}

RC RelationManager::deleteTable(const std::string &tableName) {
    if(tableName == TABLE_NAME || tableName == COLUMN_NAME || tableName == INDEX_NAME){
        // std::cout << "[Error] deleteTable -> can't delete catalog file: Tables and Columns." << std::endl;
        return -1;
    }
    
    RC rc;
    rc = deleteTableInsideIndexes(tableName);
    if(rc != 0){
        // std::cout << "[Error] deleteTable -> fail to deleteTableInsideIndexes" << std::endl;
        return -1;
    }
    
    int tableId;
    // 1. delete tuple in Tables
//    std::cout << "********delete tuple in Tables********" << std::endl;
    rc = deleteTableInsideTables(tableName, tableId);
    if(rc != 0){
        // std::cout << "[Error] deleteTable -> fail to deleteTableInsideTables" << std::endl;
        return -1;
    }
    // 2. deletes tuples in Columns
//    std::cout << "********delete tuple in Columns********" << std::endl;
//    std::cout << "deleteTable -> tableId:  " << tableId << std::endl;
    rc = deleteTableInsideColumns(tableId);
    if(rc != 0){
         // std::cout << "[Error] deleteTable -> fail to deleteTableInsideColumns" << std::endl;
        return -1;
    }
    
    // 3. delete table
    _rbfm->destroyFile(tableName);
    return 0;
}

RC RelationManager::getAttributes(const std::string &tableName, std::vector<Attribute> &attrs) {
    int tableId;
    RID rid;
    getTableIdForCustomTable(tableName, tableId, rid);
    getAttributesGivenTableId(tableId, attrs);
    return 0;
}

RC RelationManager::insertTuple(const std::string &tableName, const void *data, RID &rid) {
    FileHandle fileHandle;
    std::vector<Attribute> attrs;
    RC rc;
    
    if(tableName == TABLE_NAME || tableName == COLUMN_NAME || tableName == INDEX_NAME){
//        std::cout << "[Warning]: User can not change the Catalog files." << std::endl;
        return -1;
    }
    
    rc = getAttributes(tableName, attrs);
    if(rc != 0){
//        std::cout << "[Error] insertTuple -> can't get correct descriptor for tableName." << std::endl;
        return -1;
    }
    
    // 0. insert into heap file
    rc = _rbfm->openFile(tableName, fileHandle);
    if(rc != 0){
        // std::cout << "[Error] insertTuple -> fail to open file." << std::endl;
        return -1;
    }
    
    rc = _rbfm->insertRecord(fileHandle, attrs, data, rid);
//    void *temp = malloc(PAGE_SIZE);
//    _rbfm->readRecord(fileHandle, attrs, rid, temp);
//    _rbfm->printRecord(attrs, temp);
    if(rc != 0){
        rc = _rbfm->closeFile(fileHandle);
        // std::cout << "[Error] insertTuple -> fail to insert tuple." << std::endl;
        return -1;
    }
    rc = _rbfm->closeFile(fileHandle);
    if(rc != 0){
        // std::cout << "[Error] insertTuple -> fail to close file." << std::endl;
        return -1;
    }
    
    // insert into index file
    std::map<std::pair<std::string, std::string>, RID> columnIndexMap;
    rc = generateCoumnIndexMapGivenTable(tableName, columnIndexMap);
    if(rc != 0){
        // std::cout << "[Error]: insertTuple -> generateCoumnIndexMapGivenTable." << std::endl;
        return -1;
    }
    
    // this rid is from _rbfm.insertRecord
    rc = indexOperationWhenTupleChanged(tableName, rid, columnIndexMap, attrs, 1);
    if(rc != 0){
        // std::cout << "[Error]: insertTuple -> indexOperationWhenTupleChanged" << std::endl;
        return  -1;
    }
    
    return 0;
}

RC RelationManager::deleteTuple(const std::string &tableName, const RID &rid) {
    FileHandle fileHandle;
    std::vector<Attribute> attrs;
    RC rc;
    
    if(tableName == TABLE_NAME || tableName == COLUMN_NAME || tableName == INDEX_NAME){
//        std::cout << "[Warning]: User can not change the Catalog files." << std::endl;
        return -1;
    }
    rc = getAttributes(tableName, attrs);
    if(rc != 0){
        // std::cout << "[Error] deleteTuple -> can't get correct descriptor for tableName." << std::endl;
        return -1;
    }
    // delete from index file
    std::map<std::pair<std::string, std::string>, RID> columnIndexMap;
    rc = generateCoumnIndexMapGivenTable(tableName, columnIndexMap);
    if(rc != 0){
        // std::cout << "[Error]: insertTuple -> generateCoumnIndexMapGivenTable." << std::endl;
        return -1;
    }
    
    rc = indexOperationWhenTupleChanged(tableName, rid, columnIndexMap, attrs, 2);
    if(rc != 0){
        // std::cout << "[Error]: insertTuple -> indexOperationWhenTupleChanged" << std::endl;
        return  -1;
    }
    
    // delete from heap file
    rc = _rbfm->openFile(tableName, fileHandle);
    if(rc != 0){
        // std::cout << "[Error] deleteTuple -> fail to open file." << std::endl;
        return -1;
    }
    
    rc = _rbfm->deleteRecord(fileHandle, attrs, rid);
    if(rc != 0){
        rc = _rbfm->closeFile(fileHandle);
        if(rc != 0){
            // std::cout << "[Error] deleteTuple -> fail to close file." << std::endl;
            return -1;
        }
        
        // std::cout << "[Error] deleteTuple -> fail to delete tuple." << std::endl;
        return -1;
    }
    rc = _rbfm->closeFile(fileHandle);
    if(rc != 0){
        // std::cout << "[Error] deleteTuple -> fail to close file." << std::endl;
        return -1;
    }
    
    return 0;
}

RC RelationManager::updateTuple(const std::string &tableName, const void *data, const RID &rid) {
    FileHandle fileHandle;
    std::vector<Attribute> attrs;
    RC rc;
    
    if(tableName == TABLE_NAME || tableName == COLUMN_NAME){
//        std::cout << "[Warning]: User can not change the Catalog files." << std::endl;
        return -1;
    }
    
    rc = getAttributes(tableName, attrs);
    if(rc != 0){
        // std::cout << "[Error] updateTuple -> can't get correct descriptor for tableName." << std::endl;
        return -1;
    }
    
    // delete from index file
    std::map<std::pair<std::string, std::string>, RID> columnIndexMap;
    rc = generateCoumnIndexMapGivenTable(tableName, columnIndexMap);
    if(rc != 0){
        // std::cout << "[Error]: insertTuple -> generateCoumnIndexMapGivenTable." << std::endl;
        return -1;
    }
    
    rc = indexOperationWhenTupleChanged(tableName, rid, columnIndexMap, attrs, 2);
    if(rc != 0){
        // std::cout << "[Error]: insertTuple -> indexOperationWhenTupleChanged" << std::endl;
        return  -1;
    }
    
    // update heap file
    rc = _rbfm->openFile(tableName, fileHandle);
    if(rc != 0){
        // std::cout << "[Error] updateTuple -> fail to open file." << std::endl;
        return -1;
    }
    
    rc = _rbfm->updateRecord(fileHandle, attrs, data, rid);
    if(rc != 0){
        rc = _rbfm->closeFile(fileHandle);
        if(rc != 0){
            // std::cout << "[Error] updateTuple -> fail to close file." << std::endl;
            return -1;
        }
        
        // std::cout << "[Error] updateTuple -> fail to update record." << std::endl;
        return -1;
    }
    rc = _rbfm->closeFile(fileHandle);
    if(rc != 0){
        // std::cout << "[Error] updateTuple -> fail to close file." << std::endl;
        return -1;
    }
    
    // insert into index file
    rc = generateCoumnIndexMapGivenTable(tableName, columnIndexMap);
    if(rc != 0){
        // std::cout << "[Error]: insertTuple -> generateCoumnIndexMapGivenTable." << std::endl;
        return -1;
    }
    
    // this rid is from _rbfm.insertRecord
    rc = indexOperationWhenTupleChanged(tableName, rid, columnIndexMap, attrs, 1);
    if(rc != 0){
        // std::cout << "[Error]: insertTuple -> indexOperationWhenTupleChanged" << std::endl;
        return  -1;
    }
    return 0;
}

RC RelationManager::readTuple(const std::string &tableName, const RID &rid, void *data) {
    FileHandle fileHandle;
    RC rc;
    std::vector<Attribute> attrs;
    
    
    rc = getAttributes(tableName, attrs);
    if(rc != 0){
//        std::cout << "[Error] readTuple -> can't get correct descriptor for tableName." << std::endl;
        return -1;
    }
    
    rc = _rbfm->openFile(tableName, fileHandle);
    if(rc != 0){
        // std::cout << "[Error] readTuple -> fail to open file." << std::endl;
        return -1;
    }
    
    rc = _rbfm->readRecord(fileHandle, attrs, rid, data);
    if(rc != 0){
        rc = _rbfm->closeFile(fileHandle);
        if(rc != 0){
            // std::cout << "[Error] readTuple -> fail to close file." << std::endl;
            return -1;
        }
//        std::cout << "[Error] readTuple -> fail to read record." << std::endl;
        return -1;
    }
    
    rc = _rbfm->closeFile(fileHandle);
    if(rc != 0){
        // std::cout << "[Error] readTuple -> fail to close file." << std::endl;
        return -1;
    }
    return 0;
}

RC RelationManager::printTuple(const std::vector<Attribute> &attrs, const void *data) {
    return _rbfm->printRecord(attrs, data);
}

RC RelationManager::readAttribute(const std::string &tableName, const RID &rid, const std::string &attributeName,
                                  void *data) {
    FileHandle fileHandle;
    RC rc;
    std::vector<Attribute> attrs;
    
    
    rc = getAttributes(tableName, attrs);
    if(rc != 0){
        // std::cout << "[Error] readAttribute -> can't get correct descriptor for tableName." << std::endl;
        return -1;
    }
    
    rc = _rbfm->openFile(tableName, fileHandle);
    if(rc != 0){
        // std::cout << "[Error] readAttribute -> fail to open file." << std::endl;
        return -1;
    }
    
    rc = _rbfm->readAttribute(fileHandle, attrs, rid, attributeName, data);
    if(rc != 0){
        // std::cout << "[Error] readAttribute -> fail to read attribute." << std::endl;
        rc = _rbfm->closeFile(fileHandle);
        if(rc != 0){
            // std::cout << "[Error] readAttribute -> fail to close file." << std::endl;
            return -1;
        }
        return -1;
    }
    
    rc = _rbfm->closeFile(fileHandle);
    if(rc != 0){
        // std::cout << "[Error] insertTuple -> fail to close file." << std::endl;
        return -1;
    }
    return 0;
}

RC RelationManager::scan(const std::string &tableName,
                         const std::string &conditionAttribute,
                         const CompOp compOp,
                         const void *value,
                         const std::vector<std::string> &attributeNames,
                         RM_ScanIterator &rm_ScanIterator) {
    RC rc;
    std::vector<Attribute> attrs;
    rc = getAttributes(tableName, attrs);
    if(rc != 0){
        // std::cout << "[Error]: RelationManager::scan -> getAttributes" << std::endl;
        return -1;
    }
    _rbfm->openFile(tableName, rm_ScanIterator.getFileHandle());
    rc = _rbfm->scan(rm_ScanIterator.getFileHandle(), attrs, conditionAttribute, compOp, value, attributeNames, rm_ScanIterator.getRBFMScanIterator());
    if(rc != 0){
        // std::cout << "[Error]: RelationManager::scan -> _rbfm->scan" << std::endl;
        return -1;
    }
    return 0;
}

RC RelationManager::createIndex(const std::string &tableName, const std::string &attributeName){
    FileHandle fileHandle;
    IXFileHandle ixFileHandle;
    RBFM_ScanIterator rbfmScanIterator;
    
    std::string indexFileName = tableName + "_" + attributeName;
    // 1. test the file existence.
    if(_im->openFile(indexFileName, ixFileHandle) == 0){
        // std::cout << "[Error]: createIndex -> Index file already exists." << std::endl;
        return -1;
    }
    else if(_im->createFile(indexFileName) != 0){
        // std::cout << "[Error]: createIndex -> fail to create index file." << std::endl;
        return -1;
    }
    
    RID rid;
    RC rc;
    
    // 2. insert record into Indexes Catalog
    void *data = malloc(PAGE_SIZE);
    _rbfm->openFile(INDEX_NAME, fileHandle);
    rc = insertRecordToIndexes(fileHandle, *_rbfm, tableName, attributeName, indexFileName);
    free(data);
    if(rc != 0){
        // std::cout << "[Error] createIndex -> fail to insertRecordToIndexes" << std::endl;
        return -1;
    }
    _rbfm->closeFile(fileHandle);
    
    // Open for table file
    rc = _rbfm->openFile(tableName, fileHandle);
    if(rc != 0){
        // std::cout << "[Error]: createIndex -> fail to open table file" << std::endl;
        return -1;
    }
    if(fileHandle.getNumberOfPages() == 0){
        _rbfm->closeFile(fileHandle);
//        std::cout << "[Warning]: createIndex -> empty table." << std::endl;
        return 0;
    }
    _rbfm->closeFile(fileHandle);
    
    // 3. get attributes and attribute
    std::vector<Attribute> attrs;
    Attribute attribute;
    getAttributes(tableName, attrs);
    for(Attribute &attr : attrs){
        if(attributeName == attr.name){
            attribute = attr;
            break;
        }
    }
    if(attribute.name != attributeName){
        // std::cout << "[Error]: createIndex -> can't find the index." << std::endl;
        return -1;
    }
    
    // initialize scan
    void *returnedData = malloc(PAGE_SIZE);
    std::vector<std::string> attrNames;
    attrNames.push_back(attributeName);
    
    
    _rbfm->openFile(tableName, fileHandle);
    _im->openFile(indexFileName, ixFileHandle);
    rc = _rbfm->scan(fileHandle, attrs, "", NO_OP, NULL, attrNames, rbfmScanIterator);
    if(rc != 0){
        // std::cout << "[Error]: createIndex -> fail to initiate scan iterator" << std::endl;
        free(returnedData);
        return -1;
    }

    // do scan operation
    int nullIndicatorSize = ceil((double)attrNames.size()/CHAR_BIT);
    while (rbfmScanIterator.getNextRecord(rid, returnedData) != RM_EOF){
        char* nullIndicator = (char *)malloc(nullIndicatorSize);
        memcpy(nullIndicator, returnedData, nullIndicatorSize);
        if(nullIndicator[0] & (unsigned) 1 << (unsigned)7){
            // std::cout << "[Error]: createIndex -> index attribute is null" << std::endl;
            free(nullIndicator);
            return -1;
        }
        free(nullIndicator);
        memmove((char *)returnedData, (char *)returnedData+nullIndicatorSize, PAGE_SIZE-nullIndicatorSize);
        rc = _im->insertEntry(ixFileHandle, attribute, returnedData, rid);
        if(rc != 0){
            // std::cout << "[Error]: RelationManager::createIndex -> fail to insert into index file." << std::endl;
            return -1;
        }
    }
    
    free(returnedData);
    rbfmScanIterator.close();
//    _im->printBtree(ixFileHandle, attribute);
    // Open for table file and index file
    rc = _im->closeFile(ixFileHandle);
    if(rc != 0){
        // std::cout << "[Error]: createIndex -> fail to close index file" << std::endl;
        return -1;
    }
    return 0;
}

RC RelationManager::destroyIndex(const std::string &tableName, const std::string &attributeName){
    FileHandle fileHandle;
    IXFileHandle ixFileHandle;
    RBFM_ScanIterator rbfmScanIterator;
    
    RC rc;
    // destroy the file
    std::string indexFileName = tableName + "_" + attributeName;
    _rbfm->destroyFile(indexFileName);
    
    // prepare the value
    void *value = malloc(PAGE_SIZE);
    int indexLen = strlen(indexFileName.c_str());
    memcpy(value, &indexLen, 4);
    memcpy((char *)value+4, indexFileName.c_str(), indexLen);
    
    // prepare attrs vector
    std::vector<std::string> attrNames;
    attrNames.emplace_back("index-name");
    
    // initiate scan
    _rbfm->openFile(INDEX_NAME, fileHandle);
    rc = _rbfm->scan(fileHandle, _indexesDescriptor, "index-name", EQ_OP, value, attrNames, rbfmScanIterator);
    if(rc != 0){
        // std::cout << "[Error]: destroyIndex -> fail to initialize the scan iterator" << std::endl;
        return -1;
    }
    
    // do the scan
    RID rid;
    void *data = malloc(PAGE_SIZE);
    while(rbfmScanIterator.getNextRecord(rid, data) != RM_EOF){
        deleteTuple(INDEX_NAME, rid);
    }
    
    // close the scan
    rbfmScanIterator.close();
    
    // free the memory
    free(data);
    free(value);
    return 0;
}

// indexScan returns an iterator to allow the caller to go through qualified entries in index
RC RelationManager::indexScan(const std::string &tableName,
             const std::string &attributeName,
             const void *lowKey,
             const void *highKey,
             bool lowKeyInclusive,
             bool highKeyInclusive,
             RM_IndexScanIterator &rm_IndexScanIterator){
    std::vector<Attribute> attrs;
    Attribute attribute;
    RC rc;
    
    // get attribute
    rc = getAttributes(tableName, attrs);
    for(Attribute attr: attrs){
        if(attr.name == attributeName){
            attribute = attr;
//            std::cout << "Attribute name: " << attribute.name << std::endl;
        }
    }
    if(rc != 0 || attribute.name != attributeName){
        // std::cout << "[Error] indexScan -> getAttributes" << std::endl;
        return -1;
    }
    
    // initialize scan
    std::string indexFileName = tableName+"_"+attributeName;
    rc = _im->openFile(indexFileName, rm_IndexScanIterator.getIXFileHandle());
    if(rc != 0){
        // std::cout << "[Error]: indexScan -> fail to open index file" << std::endl;
        return -1;
    }
    rc = _im->scan(rm_IndexScanIterator.getIXFileHandle(), attribute, lowKey, highKey, lowKeyInclusive, highKeyInclusive, rm_IndexScanIterator.getIXScanIterator());
    if(rc != 0){
        // std::cout << "[Error]: indexScan -> fail to create IX_ScanItarator" << std::endl;
        return -1;
    }
    return 0;
}

// Extra credit work
RC RelationManager::dropAttribute(const std::string &tableName, const std::string &attributeName) {
    return -1;
}

// Extra credit work
RC RelationManager::addAttribute(const std::string &tableName, const Attribute &attr) {
    return -1;
}

// ========================================= //

RC RelationManager::prepareTablesDescriptor(){
    
    Attribute attr1 = {"table-id",  TypeInt, (AttrLength) TypeIntLen};
    _tablesDescriptor.push_back(attr1);
    Attribute attr2 = {"table-name",  TypeVarChar, (AttrLength) TypeVarCharLen};
    _tablesDescriptor.push_back(attr2);
    Attribute attr3 = {"file-name",  TypeVarChar, (AttrLength) TypeVarCharLen};
    _tablesDescriptor.push_back(attr3);
    
    return 0;
}

RC RelationManager::prepareColumnsDescriptor(){
    
    Attribute attr1 = {"table-id", TypeInt, (AttrLength) TypeIntLen};
    _columnsDescriptor.push_back(attr1);
    Attribute attr2 = {"column-name", TypeVarChar, (AttrLength) TypeVarCharLen};
    _columnsDescriptor.push_back(attr2);
    Attribute attr3 = {"column-type", TypeInt, (AttrLength) TypeIntLen};
    _columnsDescriptor.push_back(attr3);
    Attribute attr4 = {"column-length", TypeInt, (AttrLength) TypeIntLen};
    _columnsDescriptor.push_back(attr4);
    Attribute attr5 = {"column-position", TypeInt, (AttrLength) TypeIntLen};
    _columnsDescriptor.push_back(attr5);
    
    return 0;
}

RC RelationManager::prepareIndexesDescriptor(){
    
    Attribute attr1 = {"table-name", TypeVarChar,(AttrLength) TypeVarCharLen};
    _indexesDescriptor.push_back(attr1);
    Attribute attr2 = {"column-name", TypeVarChar,(AttrLength) TypeVarCharLen};
    _indexesDescriptor.push_back(attr2);
    Attribute attr3 = {"index-name", TypeVarChar,(AttrLength) TypeVarCharLen};
    _indexesDescriptor.push_back(attr3);
    
    return 0;
}

RC RelationManager::insertRecordToTables(FileHandle &fileHandle, RecordBasedFileManager &rbfm, int tableId, std::string tableName, std::string fileName){
    void *data = malloc(PAGE_SIZE);
    RID rid;
    prepareTablesRecord(tableId, tableName, fileName, data);
    rbfm.insertRecord(fileHandle, _tablesDescriptor, data, rid);
//    rbfm.readRecord(fileHandle, _tablesDescriptor, rid, data);
//    rbfm.printRecord(_tablesDescriptor, data);
    free(data);
    return 0;
}

RC RelationManager::insertDescriptorToColumns(FileHandle &fileHandle, RecordBasedFileManager &rbfm, int tableId, const std::vector<Attribute> targetDescriptor){
    
    int columnPos = 0;
    for(auto attr: targetDescriptor){
        columnPos++;
        insertRecordToColumns(fileHandle, rbfm, tableId, attr.name, attr.type, attr.length, columnPos);
        // _columnsDescriptor -> read record in Columns
        // retriveRecord(fileHandle, rbfm, _columnsDescriptor, rid, data);
    }
    return 0;
}

RC RelationManager::insertRecordToColumns(FileHandle &fileHandle, RecordBasedFileManager &rbfm, int tableId, std::string columnName, int columnType, int columnLength, int columnPosition){
    void *data = malloc(PAGE_SIZE);
    RID rid;
    prepareColumnsRecord(tableId, std::move(columnName), columnType, columnLength, columnPosition, data);
    rbfm.insertRecord(fileHandle, _columnsDescriptor, data, rid);
    // rbfm.readRecord(fileHandle, _columnsDescriptor, rid, data);
    // rbfm.printRecord(_columnsDescriptor, data);
    free(data);
    return 0;
}

RC RelationManager::insertRecordToIndexes(FileHandle &fileHandle, RecordBasedFileManager &rbfm, std::string tableName, std::string columnName, std::string indexName){
    void *data = malloc(PAGE_SIZE);
    RID rid;
    prepareIndexesRecord(std::move(tableName), std::move(columnName), std::move(indexName), data);
    rbfm.insertRecord(fileHandle, _indexesDescriptor, data, rid);
    // rbfm.readRecord(fileHandle, _indexesDescriptor, rid, data);
    // rbfm.printRecord(_indexesDescriptor, data);
    free(data);
    return 0;
}

RC RelationManager::prepareTablesRecord(int tableId, std::string tableName, std::string fileName, void *data){
    
    bool nullBit;
    int nullFieldsIndicatorActualSize = ceil((double)_tablesDescriptor.size()/CHAR_BIT);
    auto *nullsIndicator = (unsigned char *) malloc(nullFieldsIndicatorActualSize);
    memset(nullsIndicator, 0, nullFieldsIndicatorActualSize);
    
    int offset = 0;
    memcpy((char *)data+offset, nullsIndicator, nullFieldsIndicatorActualSize);
    offset += nullFieldsIndicatorActualSize;
    
    int varCharLen = 0;
    
    // table-id
    nullBit = nullsIndicator[0] & (unsigned) 1 << (unsigned) 7;
    if(!nullBit){
        prepareInt(tableId, data, offset);
    }
    
    // table-name
    nullBit = nullsIndicator[0] & (unsigned) 1 << (unsigned) 6;
    if(!nullBit){
        varCharLen = strlen(tableName.c_str());
        prepareVarChar(varCharLen, tableName, data, offset);
    }
    
    // file-name
    nullBit = nullsIndicator[0] & (unsigned) 1 << (unsigned) 5;
    if(!nullBit){
        varCharLen = strlen(fileName.c_str());
        prepareVarChar(varCharLen, fileName, data, offset);
    }
    
    free(nullsIndicator);
    nullsIndicator = nullptr;
    
    
    return 0;
}

// prepare the attribute as record in Columns as data format
RC RelationManager::prepareColumnsRecord(int tableId, std::string columnName, int columnType, int columnLength, int columnPosition, void *data){
    
    bool nullBit;
    int nullFieldsIndicatorActualSize = ceil((double)_columnsDescriptor.size()/CHAR_BIT);
    auto *nullsIndicator = (unsigned char *) malloc(nullFieldsIndicatorActualSize);
    memset(nullsIndicator, 0, nullFieldsIndicatorActualSize);
    
    int offset = 0;
    memcpy((char *)data+offset, nullsIndicator, nullFieldsIndicatorActualSize);
    offset += nullFieldsIndicatorActualSize;
    
    int varChanLen = 0;
    
    // table-id
    nullBit = nullsIndicator[0] & (unsigned) 1 << (unsigned) 7;
    if(!nullBit){
        prepareInt(tableId, data, offset);
    }
    
    // column-name
    nullBit = nullsIndicator[0] & (unsigned) 1 << (unsigned) 6;
    if(!nullBit){
        varChanLen = strlen(columnName.c_str());
        prepareVarChar(varChanLen, columnName, data, offset);
    }
    
    // column-type
    nullBit = nullsIndicator[0] & (unsigned) 1 << (unsigned) 5;
    if(!nullBit){
        prepareInt(columnType, data, offset);
    }
    
    // column-length
    nullBit = nullsIndicator[0] & (unsigned) 1 << (unsigned) 4;
    if(!nullBit){
        prepareInt(columnLength, data, offset);
    }
    
    // column-position
    nullBit = nullsIndicator[0] & (unsigned) 1 << (unsigned) 3;
    if(!nullBit){
        prepareInt(columnPosition, data, offset);
    }
    
    free(nullsIndicator);
    nullsIndicator = nullptr;
    return 0;
}

RC RelationManager::prepareIndexesRecord(std::string tableName, std::string columnName, std::string indexName, void *data){
    bool nullBit;
    int nullFieldsIndicatorActualSize = ceil((double)_indexesDescriptor.size()/CHAR_BIT);
    auto *nullsIndicator = (unsigned char *) malloc(nullFieldsIndicatorActualSize);
    memset(nullsIndicator, 0, nullFieldsIndicatorActualSize);
    
    int offset = 0;
    memcpy((char *)data+offset, nullsIndicator, nullFieldsIndicatorActualSize);
    offset += nullFieldsIndicatorActualSize;
    
    int varChanLen = 0;
    
    // table-name
    nullBit = nullsIndicator[0] & (unsigned) 1 << (unsigned) 7;
    if(!nullBit){
        // strlen doesn't include '\0'
        varChanLen = strlen(tableName.c_str());
        prepareVarChar(varChanLen, tableName, data, offset);
    }
    
    // column-name
    nullBit = nullsIndicator[0] & (unsigned) 1 << (unsigned) 6;
    if(!nullBit){
        varChanLen = strlen(columnName.c_str());
        prepareVarChar(varChanLen, columnName, data, offset);
    }
    
    // index-name
    nullBit = nullsIndicator[0] & (unsigned) 1 << (unsigned) 6;
    if(!nullBit){
        varChanLen = strlen(indexName.c_str());
        prepareVarChar(varChanLen, indexName, data, offset);
    }
    
    free(nullsIndicator);
    return 0;
}

RC RelationManager::prepareInt(int &value, void *data, int &offset){
    memcpy((char *)data + offset, &value, sizeof(int));
    offset += sizeof(int);
    return 0;
}

RC RelationManager::prepareFloat(float &value, void *data, int &offset){
    memcpy((char *)data + offset, &value, sizeof(float));
    offset += sizeof(float);
    return 0;
}

RC RelationManager::prepareVarChar(int &length, std::string &varChar, void *data, int &offset){
    memcpy((char *)data + offset, &length, sizeof(int));
    offset += sizeof(int);
    memcpy((char *)data + offset, varChar.c_str(), length);
    offset += length;
    return 0;
}

RC RelationManager::getTableIdForCustomTable(const std::string &tableName, int &tableId, RID &rid){
    
    RBFM_ScanIterator rbfmScanIterator;
    FileHandle fileHandle;
    
    RC rc;
    void *data = malloc(PAGE_SIZE);
    
    _rbfm->openFile(TABLE_NAME, fileHandle);
    
    // 1.1 prepare the value for table-name
    int length = strlen(tableName.c_str());
    void *table_name_value = malloc(length+4);
    memcpy(table_name_value, &length, 4);
    memcpy((char *)table_name_value+4, tableName.c_str(), length);
    
    // Only need to retrieve the tableId attribute.
    std::vector<std::string> attrNames;
    attrNames.emplace_back("table-id");
    
    // 1.3 initialize rbfmScanIterator
    rc = _rbfm->scan(fileHandle, _tablesDescriptor, "table-name", EQ_OP, table_name_value, attrNames, rbfmScanIterator);
    if(rc != 0){
        // std::cout << "[Error] getTableIdForCustomTable -> initiateRBFMScanIterator for Tabels" << std::endl;
        _rbfm->closeFile(fileHandle);
        free(table_name_value);
        free(data);
        return -1;
    }
    
    //1.4 get the record whose table-name is tableName, since there is no duplicate table names, so that the first result of getNextRecord is the final result.
    rc = rbfmScanIterator.getNextRecord(rid, data);
    if(rc != 0){
        // std::cout << "[Error] getTableIdForCustomTable -> getNextRecord for Tabels" << std::endl;
        _rbfm->closeFile(fileHandle);
        free(data);
        free(table_name_value);
        return -1;
    }
    rbfmScanIterator.close();
    
    // 1.5 retrieved data is in the original format, the first part is nullIndicator, then use memcpy to get tableId.
    int nullIndicatorSize = ceil(double(attrNames.size())/CHAR_BIT);
    memcpy(&tableId, (char *)data+nullIndicatorSize, 4);       // +1 -> 1 byte nullindicator
    
    free(data);
    free(table_name_value);
    return 0;
}

RC RelationManager::getAttributesGivenTableId(int tableId, std::vector<Attribute> &attrs) {
    
    RBFM_ScanIterator rbfmScanIterator;
    FileHandle fileHandle;
    
    RC rc;
    RID rid;
    void *data = malloc(PAGE_SIZE);
    
    rc = _rbfm->openFile(COLUMN_NAME, fileHandle);
    if(rc != 0){
        // std::cout << "[Error] getDescriptorForCustomTable -> openFile for Columns" << std::endl;
        free(data);
        return -1;
    }
    
    // 2.1 prepare attribute vector -> use the columndescriptor
    // hard-code attribute -> push and only push 3 attributes!!!!!
    std::vector<std::string> attrNames;
    attrNames.emplace_back("column-name");
    attrNames.emplace_back("column-type");
    attrNames.emplace_back("column-length");
    
    
    // 2.2 clear the customDescriptor
    attrs.clear();
    std::vector<Attribute>().swap(attrs);
    
    // 2.3 initialize rbfmScanIterator
    rc = _rbfm->scan(fileHandle, _columnsDescriptor, "table-id", EQ_OP, &tableId, attrNames, rbfmScanIterator);
    if(rc != 0){
        // std::cout << "[Error] getDescriptorForCustomTable -> initiateRBFMScanIterator for Columns" << std::endl;
        free(data);
        return -1;
    }
    
    while(true){
        rc = rbfmScanIterator.getNextRecord(rid, data);
        if(rc == 0){
            attrs.push_back(getAttributeFromData(attrNames, data));
        }
        else{
            break;
        }
    }
    
    rc = rbfmScanIterator.close();
    if(rc != 0){
        // std::cout << "[Error] getDescriptorForCustomTable -> closeFile for Columns" << std::endl;
        free(data);
        return -1;
    }
    free(data);
    return 0;
}

Attribute RelationManager::getAttributeFromData(std::vector<std::string> attrNames, void *data){
    // 1. clear the precious customDescriptor
    Attribute attr;
    // 2. Attribute struct has 3 attributes, name, type, length
    
    int fieldLength = attrNames.size();
//    get the nullIndicator size
    int nullFieldsIndicatorActualSize = ceil((double)fieldLength/CHAR_BIT);
    auto *nullsIndicator = (unsigned char *) malloc(nullFieldsIndicatorActualSize);
//    get the indicator
    memcpy((char *) nullsIndicator, data, nullFieldsIndicatorActualSize);
    
    int offset = nullFieldsIndicatorActualSize;
    int nullByteIndex, bitIndex, fieldIndex;
    
    int intData;
    int varCharLen;
    void * varChar;
    
    for(nullByteIndex = 0; nullByteIndex < nullFieldsIndicatorActualSize; nullByteIndex++)
    {
        for(bitIndex = 0; bitIndex < 8; bitIndex++)
        {
            fieldIndex = nullByteIndex*8 + bitIndex;
            
            if(fieldIndex == fieldLength)
                break;
            
            // if the corresponding field is null
            // nullsIndicator bit: 1 -> null, first field correspond the leftmost bit
            if(nullsIndicator[nullByteIndex] & (unsigned) 1 << (unsigned) (7-bitIndex))
            {
//                std::cout << attributes[fieldIndex] << '\t' << "null" << std::endl;
                continue;
            }
            
            if(attrNames[fieldIndex] == "column-name"){
                memcpy(&varCharLen, (char *)data+offset, sizeof(varCharLen));
                offset += sizeof(varCharLen);
                
                varChar = (char*)malloc(varCharLen+1);  // add one more byte for '\0'
                memcpy((char *)varChar, (char *)data+offset, varCharLen);
                ((char*)varChar)[varCharLen] = '\0';        // last byte should be '\0' for string
                offset += varCharLen;
                attr.name = (char *)varChar;
//                std::cout << attributes[fieldIndex] << '\t' << (char*)varChar << '\t';
                free(varChar);
                varChar = nullptr;
            }
            else if(attrNames[fieldIndex] == "column-type"){
                memcpy(&intData, (char *)data+offset, sizeof(intData));
                attr.type = (AttrType)intData;
                offset += sizeof(intData);
//                std::cout << attributes[fieldIndex] << '\t' << intData << '\t';
            }
            else if(attrNames[fieldIndex] == "column-length"){
                memcpy(&intData, (char *)data+offset, sizeof(intData));
                attr.length = intData;
                offset += sizeof(intData);
//                std::cout << attributes[fieldIndex] << '\t' << intData << '\t';
            }
            
        }
    }

//    std::cout << std::endl;
    free(nullsIndicator);
    return attr;
}

RC RelationManager::deleteTableInsideTables(const std::string tableName, int &tableId){
    FileHandle fileHandle;
    RC rc;
    RID rid;
    getTableIdForCustomTable(tableName, tableId, rid);
    rc = _rbfm->openFile(TABLE_NAME, fileHandle);
    if(rc != 0){
        // std::cout << "[Error]: deleteTableInsideTables -> fail to open file." << std::endl;
        return -1;
    }
    rc = _rbfm->deleteRecord(fileHandle, _tablesDescriptor, rid);
    if(rc != 0){
        // std::cout << "[Error]: deleteTableInsideTables -> fail to delete record in Tables." << std::endl;
        return -1;
    }
    rc = _rbfm->closeFile(fileHandle);
    if(rc != 0){
        // std::cout << "[Error]: deleteTableInsideTables -> fail to close file." << std::endl;
        return -1;
    }
    
    return 0;
}

RC RelationManager::deleteTableInsideColumns(int &tableId){
    RBFM_ScanIterator rbfmScanIterator;
    FileHandle fileHandle;
    
    RC rc;
    RID rid;
    void *data = malloc(PAGE_SIZE);
    
    rc = _rbfm->openFile(COLUMN_NAME, fileHandle);
    if(rc != 0){
         std::cout << "[Error] initiateRMScanIterator -> openFile for Columns" << std::endl;
        free(data);
        return -1;
    }
    
    // 2.1 prepare attribute vector
    std::vector<std::string> attrNames;
    attrNames.emplace_back("table-id");
    
    // 2.3 initialize rbfmScanIterator
    rc = _rbfm->scan(fileHandle, _columnsDescriptor, "table-id", EQ_OP, &tableId, attrNames, rbfmScanIterator);
    if(rc != 0){
         // std::cout << "[Error] initiateRMScanIterator -> initiateRBFMScanIterator for Columns" << std::endl;
        free(data);
        return -1;
    }
    
    while(true){
        rc = rbfmScanIterator.getNextRecord(rid, data);
        if(rc == 0){
            _rbfm->deleteRecord(fileHandle, _columnsDescriptor, rid);
        }
        else{
            break;
        }
    }
    
    rc = rbfmScanIterator.close();
    if(rc != 0){
         // std::cout << "[Error] initiateRMScanIterator -> closeFile for Columns" << std::endl;
        free(data);
        return -1;
    }
    return 0;
}

RC RelationManager::deleteTableInsideIndexes(const std::string tableName){
    FileHandle fileHandle;
    RC rc;
    
    // delete from index file
    std::map<std::pair<std::string, std::string>, RID> columnIndexMap;
    rc = generateCoumnIndexMapGivenTable(tableName, columnIndexMap);
    if(rc != 0){
        // std::cout << "[Error]: deleteTable -> fail to generateCoumnIndexMapGivenTable" << std::endl;
    }
    
    // destroy the index file
    for(auto &it : columnIndexMap){
        _rbfm->destroyFile(it.first.second);
    }
    
    // delete the tuple inside the Indexes
    _rbfm->openFile(INDEX_NAME, fileHandle);
    for(auto &it : columnIndexMap){
        _rbfm->deleteRecord(fileHandle, _indexesDescriptor, it.second);
    }
    _rbfm->closeFile(fileHandle);
    
    return 0;
}

RC RelationManager::generateCoumnIndexMapGivenTable(const std::string &tableName, std::map<std::pair<std::string, std::string>, RID> &columnIndexMap) {
    RBFM_ScanIterator rbfmScanIterator;
    FileHandle fileHandle;
    RC rc;
    
    _rbfm->openFile(INDEX_NAME, fileHandle);
    
    void *value = malloc(PAGE_SIZE);
    int indexLen = strlen(tableName.c_str());
    memcpy(value, &indexLen, 4);
    memcpy((char *) value + 4, tableName.c_str(), indexLen);
    
    std::vector<std::string> indexAttrNames;
    indexAttrNames.emplace_back("column-name");
    indexAttrNames.emplace_back("index-name");
    
    
    rc = _rbfm->scan(fileHandle, _indexesDescriptor, "table-name", EQ_OP, value, indexAttrNames, rbfmScanIterator);
    
    if (rc != 0) {
        // std::cout << "[Error]: generateCoumnIndexMapGivenTable -> fail to initialize scan iterator." << std::endl;
        free(value);
        return -1;
    }
    
    RID indexRid;
    void *indexData = malloc(PAGE_SIZE);
    int nullIndicatorSize = ceil((double)indexAttrNames.size()/CHAR_BIT);
    while (rbfmScanIterator.getNextRecord(indexRid, indexData) != RM_EOF) {
        int columnLen;
        memcpy(&columnLen, (char *) indexData + nullIndicatorSize, 4);
        char *columnData = (char *) malloc(columnLen + 1);
        memcpy(columnData, (char *) indexData + nullIndicatorSize + 4, columnLen);
        columnData[columnLen] = '\0';
        
        int indexNameLen;
        memcpy(&indexNameLen, (char *) indexData + nullIndicatorSize + 4 + columnLen, 4);
        auto *indexNameData = (char *) malloc(indexNameLen + 1);
        memcpy(indexNameData, (char *) indexData + nullIndicatorSize + 4 + columnLen + 4, indexNameLen);
        indexNameData[indexNameLen] = '\0';
        
        columnIndexMap.insert(std::pair<std::pair<std::string, std::string>, RID>(std::pair<std::string, std::string>(columnData, indexNameData), indexRid));
        free(columnData);
        free(indexNameData);
    }
    
    rbfmScanIterator.close();
    free(indexData);
    free(value);
    return 0;
}

RC RelationManager::indexOperationWhenTupleChanged(const std::string &tableName, const RID rid, const std::map<std::pair<std::string, std::string>, RID> columnIndexMap, const std::vector<Attribute> attrs, int operationFlag){
    FileHandle fileHandle;
    IXFileHandle ixFileHandle;
    
    RC rc;
    void *returnedKey = malloc(PAGE_SIZE);
    _rbfm->openFile(tableName, fileHandle);
    
    for(auto it = columnIndexMap.begin(); it != columnIndexMap.end(); it++){
        _rbfm->readAttribute(fileHandle, attrs, rid, it->first.first, returnedKey);
        char nullIndicator;
        memcpy(&nullIndicator, returnedKey, sizeof(char));
        int nullIndicatorSize = ceil((double(attrs.size())/CHAR_BIT));
        memmove(returnedKey, (char *)returnedKey+nullIndicatorSize, PAGE_SIZE-nullIndicatorSize);
        _im->openFile(it->first.second, ixFileHandle);
        for(auto &attribute : attrs){
            if(attribute.name == it->first.first){
                if(operationFlag == 1){
                    rc = _im->insertEntry(ixFileHandle, attribute, returnedKey, rid);
                    if(rc != 0){
                        // std::cout << "[Error]: RelationManager::indexOperationWhenTupleChanged -> fail to _im->insertEntry" << std::endl;
                    }
                }
                else if(operationFlag == 2){
                    rc = _im->deleteEntry(ixFileHandle, attribute, returnedKey, rid);
                }
                
                if(rc != 0){
                    // std::cout << "[Error]: insertTuple -> insertEntry into index file" << std::endl;
                    return -1;
                }
//                _im->printBtree(ixFileHandle, attribute);
            }
        }
        _im->closeFile(ixFileHandle);
    }
    _rbfm->closeFile(fileHandle);
    free(returnedKey);
    return 0;
}