#include "rbfm.h"

RBFM_ScanIterator::RBFM_ScanIterator(){
    maxAttrLength = 0;
    maxRecordLength = 0;
    curNumOfSlotsInPage = 0;
}

RC RBFM_ScanIterator::initiateRBFMScanIterator(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                            const std::string &conditionAttribute, const CompOp compOp, const void *value,
                                            const std::vector<std::string> &attributeNames){

    this->fileHandlePtr = &fileHandle;
    this->compOp = compOp;
    this->value = value;
    this->recordDescriptor = recordDescriptor;
    this->conditionAttribute = conditionAttribute;
    this->attributeNames = attributeNames;
    // clear the Attr vector
    this->attrVector.clear();
    std::vector<Attribute>().swap(this->attrVector);    // clear the memory

    RC rc;

    // 1. initiate numOfPages
    numOfPages = fileHandlePtr -> getNumberOfPages();
    if(numOfPages <= 0){
//        std::cout << "[Warning]: initiateAfterAssigned -> there is no pages in the file." << std::endl;
        return 0;
    }

    // 2. initiate curRID;
    curRID.pageNum = 0;
    curRID.slotNum = -1;

    // 3. initiate conditionAttributeType
    rc = getConditionAttributeType(this->conditionAttribute);
    if(rc == -1){
         std::cout << "[Error]: initiateAfterAssigned -> can't find the attribute" << std::endl;
        return -1;
    }
    else if(rc == -2){
//         std::cout << "[warning]: initiateAfterAssigned -> condition attribute is null" << std::endl;
    }

    // 4. generate attributes descriptor for print result
    // recordDescriptor is the descriptor for retrived attributes
    generateAttributesDescriptor();

    // 4. get maxAttrLength and maxRecordLength
    for(auto & it : this->recordDescriptor){
        maxRecordLength += it.length;
        if(it.length > maxAttrLength)
            maxAttrLength = it.length;
    }

    // 5. initiate curNumOfSlotsInPage
    void *page = malloc(PAGE_SIZE);
    fileHandlePtr->readPage(curRID.pageNum, page);
    auto *pageDirPtr = (PageDirectory *)((char *)page + PAGE_SIZE - sizeof(PageDirectory));
    curNumOfSlotsInPage = pageDirPtr->numberofslot;

//    std::cout << "initiateRBFMScanIterator -> curNumOfSlotsInPage: "  << curNumOfSlotsInPage << std::endl;
    free(page);

    return 0;
}

RC RBFM_ScanIterator::getConditionAttributeType(const std::string& attribute){

    for(auto & it : recordDescriptor) {
        if(it.name == attribute){
            conditionAttributeType = it.type;
            return 0;
        }
    }
    if(attribute.empty()){
        // if there is no conditionattribute
        conditionAttributeType = (AttrType)-1;
        return -2;
    }
    return -1;
}

RC RBFM_ScanIterator::generateAttributesDescriptor(){
    int indexForAttrName = 0;
    for(auto & i : recordDescriptor){
        if((unsigned)indexForAttrName < attributeNames.size()){
            if(i.name == attributeNames[indexForAttrName]){
                attrVector.push_back(i);
                indexForAttrName++;
            }
        }
    }
    return 0;
}

RC RBFM_ScanIterator::getNextRecord(RID &rid, void *data) {

    RC rc = getNextRID(rid);
    if(rc == 0){
        RecordBasedFileManager &rbfm = RecordBasedFileManager::instance();
        rbfm.readAttributes(*fileHandlePtr, recordDescriptor, rid, attributeNames, data);
//        rbfm.printRecord(attrVector, data);
        return 0;
    }
    else if(rc == RBFM_EOF){
        // std::cout << "[Warning] getNextRecord -> get end of file" << std::endl;
        return RBFM_EOF;
    }
    else{
        // std::cout << "[Error] getNextRecord -> getNextRID" << std::endl;
        return -2;
    }

    return RBFM_EOF;
};


RC RBFM_ScanIterator::getNextRID(RID &rid){

    RecordBasedFileManager &rbfm = RecordBasedFileManager::instance();
    void *attribute = malloc(maxAttrLength);
    RC rc;
    while(true) {
        // original curRID.slotNum = -1 -> first update
        rc = updateCurRIDAndCurNumOfSlotsInPage();
        if(rc == RBFM_EOF){
//            std::cout << "[Warning] getNextRID -> get end of file" << std::endl;
            free(attribute);
            return RBFM_EOF;
        }
        else if(rc == -2){
            // std::cout << "[Error] getNextRID -> update numOfSlots fails" << std::endl;
            free(attribute);
            return -2;
        }
        else{
            // check whether the flag is record flag, in order to prevent redundant record print.
            rc = rbfm.checkRecordFlag(*fileHandlePtr, curRID);
            if(rc != 0){
                continue;
            }
            else{
                if(conditionAttribute.empty()){
                    rid.pageNum = curRID.pageNum;
                    rid.slotNum = curRID.slotNum;
                    // if condition attribute is "", then there is no need to do Comporation.
                    free(attribute);
                    return 0;
                }

                rc = rbfm.readAttribute(*fileHandlePtr, recordDescriptor, curRID, conditionAttribute, attribute);
                if(rc == -1){
                    free(attribute);
                    // std::cout << "[Error] : getNextRID -> readAttribute error." << std::endl;
                    return -2;
                }
                else if(rc == -2){
                    // if the record is deleted, continue, continue, read the next record.
                    continue;
//                std::cout << "[Warning] : getNextRID -> scan deleted record." << std::endl;
                }
                else if(rc == 0){
                    int nullSize = ceil((double)recordDescriptor.size()/CHAR_BIT);
                    if(doOp(nullSize, attribute)>0){
                        rid.pageNum = curRID.pageNum;
                        rid.slotNum = curRID.slotNum;

                        free(attribute);
                        return 0;
                    }
                    else{
                        // if the result doesn't meet the requirement, continue, check the next record.
                        continue;
                    }
                }
            }
        }
    }
}

RC RBFM_ScanIterator::updateCurRIDAndCurNumOfSlotsInPage(){
    // In the constructor, initial curRID.slotNum is set to -1. So each time we should first increment curNode.slotNum
    curRID.slotNum++;
    if(curRID.slotNum >= curNumOfSlotsInPage){
        curRID.slotNum = 0;
        curRID.pageNum += 1;
        if(curRID.pageNum >= numOfPages){
            curNumOfSlotsInPage = 0;
//            std::cout << "[Warning] updateCurRIDAndCurNumOfSlotsInPage -> get end of file" << std::endl;
            return RBFM_EOF;
        }
        else{
            if(updateNumOfSlots() != 0){
                curNumOfSlotsInPage = 0;
                // std::cout << "[Error] updateCurRIDAndCurNumOfSlotsInPage -> update numOfSlots fails" << std::endl;
                return -2;
            }
        }
    }
    return 0;
};

RC RBFM_ScanIterator::doOp(int nullSize, void *attribute){

    int result;

    if(conditionAttributeType == TypeInt){
        int valueOfAttr;
        memcpy(&valueOfAttr, (char *)attribute+nullSize, sizeof(int));

        int valueOfCondtion;
        memcpy(&valueOfCondtion, value, sizeof(int));

//        std::cout << "get attribute value " <<  valueOfAttr << " and " << valueOfCondtion << std::endl;

        switch (compOp){
            case EQ_OP:{
                return valueOfAttr == valueOfCondtion;
            }
            case LT_OP:{
                return valueOfAttr < valueOfCondtion;
            }
            case LE_OP:{
                return valueOfAttr <= valueOfCondtion;
            }
            case GT_OP:{
                return valueOfAttr > valueOfCondtion;
            }
            case GE_OP:{
                return valueOfAttr >= valueOfCondtion;
            };
            case NE_OP:{
                return valueOfAttr != valueOfCondtion;
            }
            case NO_OP:{
                return 1;
            }
        }

    } else if(conditionAttributeType == TypeReal){
        float valueOfAttr;
        memcpy(&valueOfAttr, (char *)attribute+nullSize, sizeof(float));

        float valueOfCondtion;
        memcpy(&valueOfCondtion, value, sizeof(float));

//        std::cout << "get attribute value " <<  valueOfAttr << " and " << valueOfCondtion << std::endl;

        switch (compOp){
            case EQ_OP:{
                return valueOfAttr == valueOfCondtion;
            }
            case LT_OP:{
                return valueOfAttr < valueOfCondtion;
            }
            case LE_OP:{
                return valueOfAttr <= valueOfCondtion;
            }
            case GT_OP:{
                return valueOfAttr > valueOfCondtion;
            }
            case GE_OP:{
                return valueOfAttr >= valueOfCondtion;
            };
            case NE_OP:{
                return valueOfAttr != valueOfCondtion;
            }
            case NO_OP:{
                return 1;
            }
        }

    }
    else if(conditionAttributeType == TypeVarChar){
        int varCharLen;
        memcpy(&varCharLen, (char *)attribute+nullSize, 4);

//        std::cout << varCharLen << std::endl;
        void *valueOfAttr = malloc(varCharLen+1);
        memcpy((char *)valueOfAttr, (char *)attribute+nullSize+4, varCharLen);
        ((char *)valueOfAttr)[varCharLen] = '\0';

        memcpy(&varCharLen, value, 4);
        void *valueOfCondtion = malloc(varCharLen+1);
        memcpy((char *)valueOfCondtion, (char *)value+4, varCharLen);
        ((char *)valueOfCondtion)[varCharLen] = '\0';

//        std::cout << "get attribute value " <<  (char*)valueOfAttr << " and " << (char*)valueOfCondtion << std::endl;

        switch (compOp){
            case EQ_OP:{
                result = strcmp((const char *)valueOfAttr, (const char *)valueOfCondtion) == 0;
                free(valueOfAttr);
                free(valueOfCondtion);
                return result;
            }
            case LT_OP:{
                result = strcmp((const char *)valueOfAttr, (const char *)valueOfCondtion) < 0;
                free(valueOfAttr);
                free(valueOfCondtion);
                return result;
            }
            case LE_OP:{
                result = strcmp((const char *)valueOfAttr, (const char *)valueOfCondtion) <= 0;
                free(valueOfAttr);
                free(valueOfCondtion);
                return result;
            }
            case GT_OP:{
                free(valueOfAttr);
                free(valueOfCondtion);
                return strcmp((const char *)valueOfAttr, (const char *)valueOfCondtion) > 0;
            }
            case GE_OP:{
                result = strcmp((const char *)valueOfAttr, (const char *)valueOfCondtion) >= 0;
                free(valueOfAttr);
                free(valueOfCondtion);
                return result;
            };
            case NE_OP:{
                result = strcmp((const char *)valueOfAttr, (const char *)valueOfCondtion) != 0;
                free(valueOfAttr);
                free(valueOfCondtion);
                return result;
            }
            case NO_OP:{
                free(valueOfAttr);
                free(valueOfCondtion);
                return 1;
            }
        }


    }
    return 0;
}

RC RBFM_ScanIterator::updateNumOfSlots(){

    void *page = malloc(PAGE_SIZE);
    if(fileHandlePtr->readPage(curRID.pageNum, page) == 0){
        auto *pageDirPtr = (PageDirectory *)((char *)page + PAGE_SIZE - sizeof(PageDirectory));
        curNumOfSlotsInPage = pageDirPtr->numberofslot;
        free(page);
        return 0;
    }
    else{
        free(page);
        // std::cout << "[Error] updateNumOfSlots -> read page fails." << std::endl;
        return -1;
    }
}

RC RBFM_ScanIterator::close() {
    RecordBasedFileManager::instance().closeFile(*fileHandlePtr);
    return 0; };

RecordBasedFileManager *RecordBasedFileManager::_rbf_manager = nullptr;

RecordBasedFileManager &RecordBasedFileManager::instance() {
    static RecordBasedFileManager _rbf_manager = RecordBasedFileManager();
    return _rbf_manager;
}

RecordBasedFileManager::RecordBasedFileManager() = default;

RecordBasedFileManager::~RecordBasedFileManager() { delete _rbf_manager; }

RecordBasedFileManager::RecordBasedFileManager(const RecordBasedFileManager &) = default;

RecordBasedFileManager &RecordBasedFileManager::operator=(const RecordBasedFileManager &) = default;

RC RecordBasedFileManager::createFile(const std::string &fileName) {
    return PagedFileManager::instance().createFile(fileName);
}

RC RecordBasedFileManager::destroyFile(const std::string &fileName) {
    return PagedFileManager::instance().destroyFile(fileName);
}

RC RecordBasedFileManager::openFile(const std::string &fileName, FileHandle &fileHandle) {
    return PagedFileManager::instance().openFile(fileName, fileHandle);
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
    return PagedFileManager::instance().closeFile(fileHandle);
}

RC RecordBasedFileManager::getTotalLenAndLenAheadOfVariableField(const std::vector<Attribute> &recordDescriptor, const void *data, void *info,
                                                                 char16_t fieldLength, int nullFieldsIndicatorActualSize, unsigned char *nullsIndicator){

    // fieldNumber, offset, length, fieldlength, fieldoffset -> 2 Byte -> char16_t

    int offset = nullFieldsIndicatorActualSize;
    int lengthBeforeVariableData = nullFieldsIndicatorActualSize;
    int nullByteIndex, bitIndex, fieldIndex;
    int varCharLen;


    for(nullByteIndex = 0; nullByteIndex < nullFieldsIndicatorActualSize; nullByteIndex++){
        for(bitIndex = 0; bitIndex < 8; bitIndex++){
            fieldIndex = nullByteIndex*8 + bitIndex;
            if(fieldIndex == fieldLength)
                break;
            // if the corresponding field is null
            // nullsIndicator bit: 1 -> null, first field correspond the leftmost bit
            if(nullsIndicator[nullByteIndex] & (unsigned) 1 << (unsigned) (7-bitIndex)){
                continue;
            }
            switch(recordDescriptor[fieldIndex].type){
                case TypeVarChar:
//                    data = int(len) + string(name)
                    memcpy(&varCharLen, (char *)data+offset, sizeof(varCharLen));
                    offset += 4;    // use two byte to store length, use two byte to store offset
                    offset += varCharLen;
                    lengthBeforeVariableData += 4;
                    break;
                case TypeInt:
                    offset += 4;    // fixed-length field, in-line record field
                    lengthBeforeVariableData += 4;
                    break;
                case TypeReal:
                    offset += 4;    // fixed-length field, in-line record field
                    lengthBeforeVariableData += 4;
                    break;
                default:
                    // std::cout << "[Error] None-specified data type." << std::endl;
                    break;
            }
        }
    }

    // +2 ->Use 2 bytes to represent number of field of each record.
    offset+=fieldSizeLen;
    lengthBeforeVariableData += fieldSizeLen;

    // +1 -> use 1 bit to represent the flag which indicates whether the record is record or pointer

    offset += flagLen;
    lengthBeforeVariableData += flagLen;

    memcpy((char *)info, &offset, 4);
    memcpy((char *)info+4, &lengthBeforeVariableData, 4);
    // recordLength = offset + 2 -> 2 (store number of fields)
    return 0;
}

RC RecordBasedFileManager::formatRecord(const std::vector<Attribute> &recordDescriptor, const void *data, void *info, void *record,
                                        char16_t fieldLength, int nullFieldsIndicatorActualSize, unsigned char *nullsIndicator){

    // char16_t recordLength = ((int *)info)[0];
    int offsetRecord = 0;
    int offsetData = 0;
    char16_t offsetVariableData = ((int *)info)[1];

    // 1. insert the flag
    memset((char *)record, recordFlag, 1);
    offsetRecord += flagLen;

    // 2. insert num of field
    memcpy((char *)record+offsetRecord, &fieldLength, 2);
    offsetRecord += fieldSizeLen;      // First 2 byte to store the field number


    // 3. insert nullIndicator
    memcpy((char *) record+offsetRecord, (char *)data+offsetData, nullFieldsIndicatorActualSize);
    offsetData += nullFieldsIndicatorActualSize;
    offsetRecord += nullFieldsIndicatorActualSize;      // store nullIndicator

    // 4. insert attributes
    int nullByteIndex, bitIndex, fieldIndex;

    int varCharLen;
    char16_t varCharLen_16;

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
                continue;
            }
            switch(recordDescriptor[fieldIndex].type)
            {
                case TypeVarChar:
//                    data = int(len) + string(name)
                    varCharFromDataToRecord(offsetRecord, offsetData, varCharLen_16, varCharLen, offsetVariableData, record, data);
                    break;
                case TypeInt:
                    memcpy((char *)record+offsetRecord, (char *)data+offsetData, 4);
                    offsetData += 4;
                    offsetRecord += intFieldLen;  // 4-byte inline storage of integer data
                    break;
                case TypeReal:
                    memcpy((char *)record+offsetRecord, (char *)data+offsetData, 4);
                    offsetData += 4;
                    offsetRecord += realFieldLen;  // 4-byte inline storage of float data
                    break;
                default:
                    // std::cout << "[Error] None-specofied data type." << std::endl;
                    break;
            }

        }
    }

    // std::cout << "[Success] formatRecord()." << std::endl;
    return 0;
}

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                        const void *data, RID &rid) {

    /*
      * Record format:
      * We follow the inline of fixed-size field displayed in the class.
      *
      * 1. the first 2 byte is to store the number of field of record.
      * 2. then according to the length of the record, we get the number of bytes to represent the null indicator.
      * 3. according to the type of the field, we store the data to record differently:
      *      1. for integer and float data type, we use 4 bytes and store in-line
      *      2. for variable length data: 2 byte to store the length, 2 bytes to store the offset, then we store the data using the offset value.
      */

    // 0. initialize null indicator
    char16_t fieldLength = recordDescriptor.size();
    int nullFieldsIndicatorActualSize = ceil((double(fieldLength)/CHAR_BIT));
    auto *nullsIndicator = (unsigned char *) malloc(nullFieldsIndicatorActualSize);
    memcpy((char *) nullsIndicator, (char *)data, nullFieldsIndicatorActualSize);


    // 1. get record length and the length ahead of variable data.
    void *LenAndValidField = (char *)malloc(8);
    getTotalLenAndLenAheadOfVariableField(recordDescriptor, data, LenAndValidField, fieldLength, nullFieldsIndicatorActualSize, nullsIndicator);

    char16_t recordLength = ((int *)LenAndValidField)[0];

    // we need to allocate at least 9bytes (4 for pageNum and 4 for slotNum and 1 for flag) for a tombstone.
    void *record;
    if(recordLength > tombstoneLen){
        record = malloc(recordLength);        // allocate a space for record with recordLength
    }
    else{
        record = malloc(tombstoneLen);
        recordLength = tombstoneLen;
    }

//    std::cout << "insertRecord() " << "this record length is " << recordLength << std::endl;

    // 2. We format data to record format and insert it, and we need to make sure that the recordLength >= 9 bytes (pointer).
    formatRecord(recordDescriptor, data, LenAndValidField, record, fieldLength, nullFieldsIndicatorActualSize, nullsIndicator);     // convert the data to record with the format stated above.

    // 3. First, we find an available page to insert this record
    void *page = malloc(PAGE_SIZE);
    RC rc = findAvailablePage(fileHandle, recordLength, rid, page);

    if(rc == 0){
        // update page information
        char* _pPage = (char*) page + PAGE_SIZE - sizeof(PageDirectory);
        auto* thisPage = (PageDirectory*) _pPage;
        char16_t record_offset = PAGE_SIZE - sizeof(PageDirectory) - (thisPage->numberofslot) * sizeof(SlotDirectory) - thisPage->freespace;

        // update slot information
        SlotDirectory slotDirectory;
        slotDirectory.length = recordLength;
        slotDirectory.offset = record_offset;

        char16_t slot_offset = PAGE_SIZE - sizeof(PageDirectory) - (rid.slotNum + 1) * sizeof(SlotDirectory);
        // insert slot
        memcpy((char*) page + slot_offset, &slotDirectory, sizeof(SlotDirectory));

        // insert record
        memcpy((char*) page + record_offset, record, recordLength);

        thisPage->numberofslot++;
        thisPage->freespace -= (sizeof(SlotDirectory) + recordLength);

        if(fileHandle.writePage(rid.pageNum, page) == 0){
            // success
            free(record);
            free(LenAndValidField);
            free(nullsIndicator);
            free(page);
        }
        else{
//            std::cout << "[Error] Fail to write when inserting the record." << std::endl;
            free(record);
            free(LenAndValidField);
            free(nullsIndicator);
            free(page);
            return -1;
        }

    }
    else if (rc == 1){
        // this slot is reused, and we need to reset the offset and length of this slot.

        char* _pPage = (char*)page + PAGE_SIZE - sizeof(PageDirectory);
        auto* thisPage = (PageDirectory*) _pPage;
        char16_t record_offset = PAGE_SIZE - sizeof(PageDirectory) - (thisPage->numberofslot) * sizeof(SlotDirectory) - thisPage->freespace;

        // update slot information
        SlotDirectory slotDirectory;
        slotDirectory.length = recordLength;
        slotDirectory.offset = record_offset;

        char16_t slot_offset = PAGE_SIZE - sizeof(PageDirectory) - (rid.slotNum + 1) * sizeof(SlotDirectory);
        // update slot
        memcpy((char*) page + slot_offset, &slotDirectory, sizeof(SlotDirectory));
        // insert record
        memcpy((char*) page + record_offset, record, recordLength);
        // update freespace
        thisPage->freespace -= recordLength;

        if(fileHandle.writePage(rid.pageNum, page) == 0){
            // success
            free(record);
            free(LenAndValidField);
            free(nullsIndicator);
            free(page);
        }
        else{
//            std::cout << "[Error] Fail to write when inserting the record." << std::endl;
            free(record);
            free(LenAndValidField);
            free(nullsIndicator);
            free(page);
            return -1;
        }

    }
    else{
        // std::cout << "[Error] There is no available page to insert the record." << std::endl;
        free(record);
        free(LenAndValidField);
        free(nullsIndicator);
        free(page);
        return -1;
    }

//    std::cout << "[Success] Insert Record completed." << std::endl;
    return 0;

}

RC RecordBasedFileManager::deFormatRecord(const std::vector<Attribute> &recordDescriptor, void *data, void *record){

    int offsetRecord = 0;
    int offsetData = 0;
    char16_t offsetVariableData = 0;

    // 1. get flag
    char flag;
    memcpy(&flag, (char *)record, 1);
    offsetRecord += flagLen;

    // 2. get num of field
    char16_t fieldLength;
    memcpy(&fieldLength, (char *)record+offsetRecord, 2);
    offsetRecord += fieldSizeLen;

    // 3. get null indicator
    int nullFieldsIndicatorActualSize = ceil((double(fieldLength)/CHAR_BIT));
    auto *nullsIndicator = (unsigned char *) malloc(nullFieldsIndicatorActualSize);
    memcpy((char *) nullsIndicator, (char *)record+offsetRecord, nullFieldsIndicatorActualSize);
    memcpy((char *)data+offsetData, (char *) record+offsetRecord, nullFieldsIndicatorActualSize);
    offsetData += nullFieldsIndicatorActualSize;
    offsetRecord += nullFieldsIndicatorActualSize;

    // 4. get attributes
    int nullByteIndex, bitIndex, fieldIndex;
    int varCharLen;
    char16_t varCharLen_16;

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
                continue;
            }
            switch(recordDescriptor[fieldIndex].type)
            {
                case TypeVarChar:
//                    data = int(len) + string(name)
                    varCharFromRecordToData(offsetRecord, offsetData, varCharLen_16, varCharLen, offsetVariableData, record, data);
                    break;
                case TypeInt:
                    memcpy((char *)data+offsetData, (char *)record+offsetRecord, 4);
                    offsetData += 4;
                    offsetRecord += intFieldLen;
                    break;
                case TypeReal:
                    memcpy((char *)data+offsetData, (char *)record+offsetRecord, 4);
                    offsetData += 4;
                    offsetRecord += realFieldLen;
                    break;
                default:
                    // std::cout << "[Error] None-specofied data type." << std::endl;
                    break;
            }

        }
    }
    free(nullsIndicator);
    return 0;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                      const RID &rid, void *data) {
    void *record = malloc(PAGE_SIZE);
    RC rc = getRecord(fileHandle, recordDescriptor, rid, record);
    if(rc != 0){
//        std::cout << "[Error]: readRecord -> can't get the record." << std::endl;
        free(record);
        return rc;
    }
    else{
        rc = deFormatRecord(recordDescriptor, data, record);
        free(record);
        return rc;
    }
}

/*
 * return:  -2 -> record for this rid is been deleted
 *          -1 -> error
 *          0 -> get the record
 */
RC RecordBasedFileManager::getRecord(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                      const RID &rid, void *record) {

    char *page = (char *)malloc(PAGE_SIZE);
    char* slot;
    SlotDirectory* thisSlot;
    char flag = ptrFlag;
    RID tempRid = rid;

    while(flag == ptrFlag){
        if(fileHandle.readPage(tempRid.pageNum, page) == 0){
            // success
            slot = page + PAGE_SIZE - sizeof(PageDirectory) - (tempRid.slotNum + 1) * sizeof(SlotDirectory);
            thisSlot = (SlotDirectory*)slot;

//            std::cout << "thisSlot->length" << thisSlot->length << std::endl;
//            std::cout << "rid.pageNum: " << rid.pageNum << ", rid.slotNum: " << rid.slotNum << std::endl;

            if(thisSlot->length == 0){
                // this record has been deleted.
//                std::cout << "[Warning] getRecord -> This record has been deleted when reading the record." << std::endl;
                // Important!!!! when scan the table, thisSlot->length == 0 is normal.!!!!
                free(page);
                return -2;
            }

            memcpy(&flag, page+thisSlot->offset, 1);

            if(flag == recordFlag){
//                std::cout << "get record" << std::endl;
                memcpy((char *)record, page + thisSlot->offset, thisSlot->length);
                free(page);
                return 0;
            }

            else if(flag == ptrFlag){
                // due to update, this record has been moved to another page.
                // so, we update rid and continue to search

                // std::cout << "readRecord() find a tombstone: temp_rid " << tempRid.pageNum << " , " << tempRid.slotNum << std::endl;

                char* thisTombStone = page + thisSlot->offset + 1;
                auto* newRID = (RID*)thisTombStone;
                tempRid.pageNum = newRID->pageNum;
                tempRid.slotNum = newRID->slotNum;

                // std::cout << "readRecord() next destination: temp_rid " << tempRid.pageNum << " , " << tempRid.slotNum << std::endl;
            }

            else{
                // std::cout << "[Error] readRecord()-> getRecord() wrong format record. No flag byte set. " << std::endl;
                free(page);
                return -1;
            }
        }
        else{
            free(page);
            // std::cout << "[Error] getRecord -> Fail to read when reading the record." << std::endl;
            return -1;
        }
    }

    free(page);
//    std::cout << "[Success] Read Record completed." << std::endl;
    return 0;

}

RC RecordBasedFileManager::deleteRecord(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                        const RID &rid) {
    // find record and set the recordLength to 0, and then shift all the remaining records forward to update freespace.

    void* dpage = malloc(PAGE_SIZE);

    // we need to judge whether it is a tombstone or a record
    char flag = ptrFlag;
    RID temp_rid = rid;

    // we first assign it to tombstone and check
    while(flag == ptrFlag){
        if(fileHandle.readPage(temp_rid.pageNum, dpage) == 0){

            // std::cout << "Prepare to delete: " << temp_rid.pageNum << " , " << temp_rid.slotNum << std::endl;

            // check whether this record is a tombstone
            char* _page = (char*)dpage + PAGE_SIZE - sizeof(PageDirectory);
            char* _slot = (char*)dpage + PAGE_SIZE - sizeof(PageDirectory) - (temp_rid.slotNum + 1) * sizeof(SlotDirectory);
            auto* thisPage = (PageDirectory*)_page;
            auto* thisSlot = (SlotDirectory*)_slot;

            if(thisSlot->length == 0){
                // this record has been deleted.
                // std::cout << "[Error] This record has already been deleted when we want to delete a record." << std::endl;
                free(dpage);
                return -1;
            }

            // get the flag.
            memcpy(&flag, (char*)dpage + thisSlot->offset, 1);

            if(flag == recordFlag){

                char16_t recordLength = thisSlot->length;
                thisSlot->length = 0;

                // add the recordLength to freespace
                thisPage->freespace += recordLength;

                // free this record's space
                memset((char*)dpage + thisSlot->offset, 0, recordLength);

                if(temp_rid.slotNum + 1 > thisPage->numberofslot){
                    // std::cout << "[Error] Fail to delete because the slotNum is out of limit." << std::endl;
                    free(dpage);
                    return -1;
                }
                else{
                    // compact this page, call the shift function.
                    shiftAllRecords(recordLength, temp_rid, dpage);
                }

                // After shift, write the info back to disk.
                if(fileHandle.writePage(temp_rid.pageNum, dpage) == 0){
                    // std::cout << "Delete a record with RID: " << temp_rid.pageNum << " , "<< temp_rid.slotNum << std::endl;
                }
                else{
                    // std::cout << "[Error] Fail to write page when delete record." << std::endl;
                    free(dpage);
                    return -1;
                }

                break;
            }
            else if (flag == ptrFlag){
                // Due to update, this record has been moved to another page and this is a tombstone.
                // so, we update rid and continue to search

                RID old_rid;
                old_rid.pageNum = temp_rid.pageNum;
                old_rid.slotNum = temp_rid.slotNum;

                char* thisTombStone = (char*)dpage + thisSlot->offset + 1;
                auto* newRID = (RID*)thisTombStone;
                temp_rid.pageNum = newRID->pageNum;
                temp_rid.slotNum = newRID->slotNum;

                // std::cout << "deleteRecord() next destination->RID " << temp_rid.pageNum << " , " << temp_rid.slotNum << std::endl;

                // delete the record
                char16_t recordLength = thisSlot->length;
                // std::cout << "deleteRecord() tombstone length " << thisSlot->length << std::endl;
                thisSlot->length = 0;

                // free this record's space
                memset((char*)dpage + thisSlot->offset, 0, recordLength);

                // add the recordLength to freespace
                thisPage->freespace += recordLength;

                if(old_rid.slotNum + 1 > thisPage->numberofslot){
                    // std::cout << "[Error] Fail to delete because the slotNum is out of limit." << std::endl;
                    free(dpage);
                    return -1;
                }
                else{
                    // compact this page, call the shift function.
                    shiftAllRecords(recordLength, old_rid, dpage);
                }

                // After shift, write the info back to disk.
                if(fileHandle.writePage(old_rid.pageNum, dpage) == 0){
                    // std::cout << "Delete a tombstone with RID: " << old_rid.pageNum << " , "<< old_rid.slotNum << std::endl;
                    // std::cout << "deleteRecord() next destination->RID " << temp_rid.pageNum << " , " << temp_rid.slotNum << std::endl;
                }
                else{
                    // std::cout << "[Error] Fail to write page when delete record." << std::endl;
                    free(dpage);
                    return -1;
                }

                // so we continue to search and delete
                // continue;
            }
            else{
                // std::cout << "[Error] deleteRecord() wrong format record. No flag byte set. " << std::endl;
                free(dpage);
                return -1;
            }
        }
        else{
            // std::cout << "[Error] Fail to read page when delete the record." << std::endl;
            free(dpage);
            return -1;
        }
    }   // end while loop


//    std::cout<< "[Success] Delete Record completed." << std::endl;
    free(dpage);
    return 0;

}


RC RecordBasedFileManager::shiftAllRecords(char16_t distance, const RID &rid, void *page) {
    
    // we need to order all the records and shift all the records whose offset is bigger than this record
    
    char* _PageDir = (char*) page + PAGE_SIZE - sizeof(PageDirectory);
    // get the slot that we want to delete.
    char* _SlotDir = (char*) page + PAGE_SIZE - sizeof(PageDirectory) - (rid.slotNum + 1) * sizeof(SlotDirectory);
    auto* thisPage = (PageDirectory*)_PageDir;
    auto* thisSlot = (SlotDirectory*)_SlotDir;

    // get the location where we judge whether we shift records or not.
    char16_t pivot = thisSlot->offset + thisSlot->length;

    // a sorted hashmap to store each record's <offset, #slot>.
    std::map<char16_t, unsigned> recordsMap;

    // iterate through all slots and compare it with this records.offset
    for(unsigned int i=0; i < thisPage->numberofslot; i++){
        char* _slot = (char*) page + PAGE_SIZE - sizeof(PageDirectory) - (i + 1) * sizeof(SlotDirectory);
        auto* this_slot = (SlotDirectory*)_slot;

        // There are two requirements which we need to satisfy:
        // 1. its offset is behind
        // 2. it is not been deleted.

        if((this_slot->offset > pivot) && (this_slot->length > 0)){
            // this record needs to be inserted in map and shift afterward.
            // And they will be stored in ascending order (default).
            recordsMap.insert(std::pair<char16_t, unsigned>(this_slot->offset, i));
        }
    }

    // shift all the records in the map
    std::map<char16_t, unsigned>::iterator it;
//    std::cout << "Map: " << std::endl;

    for(it = recordsMap.begin(); it != recordsMap.end(); it++) {
        // use iterator to shift each record.
//        std::cout << " < " << it->first << " , " << it->second << " > " << std::endl;

        char* _slot = (char*) page + PAGE_SIZE - sizeof(PageDirectory) - (it->second + 1) * sizeof(SlotDirectory);
        auto* this_slot = (SlotDirectory*)_slot;

        // DO NOT directly shift with overlapping memory and it will cause error on openlab machine.
        void *oldRecord = malloc(this_slot->length);
        memcpy(oldRecord, (char *)page+this_slot->offset, this_slot->length);
        memcpy((char *)page + this_slot->offset - distance, oldRecord, this_slot->length);

        free(oldRecord);

        this_slot->offset -= distance;
        // reset or free this space, beginning from the end of current record.
        memset((char*)page + this_slot->offset + this_slot->length, 0, distance);
    }

    // free the last space
    // memset((char*) page + PAGE_SIZE - sizeof(PageDirectory) - (thisPage->numberofslot) * sizeof(SlotDirectory) - thisPage->freespace, 0, distance);

//    std::cout << "Shift all the Records." << std::endl;

    return 0;

}

RC RecordBasedFileManager::printRecord(const std::vector<Attribute> &recordDescriptor, const void *data) {


    int fieldLength = recordDescriptor.size();
//    get the nullIndicator size
    int nullFieldsIndicatorActualSize = ceil((double(fieldLength)/CHAR_BIT));
    auto *nullsIndicator = (unsigned char *) malloc(nullFieldsIndicatorActualSize);
//    get the indicator
    memcpy((char *) nullsIndicator, data, nullFieldsIndicatorActualSize);

    int offset = nullFieldsIndicatorActualSize;
    int nullByteIndex, bitIndex, fieldIndex;

    int intData;
    float floatData;
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
                std::cout << recordDescriptor[fieldIndex].name << '\t' << "NULL" << '\t';
                continue;
            }
            switch(recordDescriptor[fieldIndex].type)
            {
                case TypeVarChar:
//                    data = int(len) + string(name)
                    memcpy(&varCharLen, (char *)data+offset, sizeof(varCharLen));
                    offset += sizeof(varCharLen);

                    varChar = (char*)malloc(varCharLen+1);  // add one more byte for '\0'
                    memcpy((char *)varChar, (char *)data+offset, varCharLen);
                    ((char*)varChar)[varCharLen] = '\0';        // last byte should be '\0' for string
                    offset += varCharLen;

                    std::cout << recordDescriptor[fieldIndex].name << '\t' << (char*)varChar << '\t';
                    free(varChar);
                    varChar = nullptr;
                    break;
                case TypeInt:
                    memcpy(&intData, (char *)data+offset, sizeof(intData));
                    offset += sizeof(intData);

                    std::cout << recordDescriptor[fieldIndex].name << '\t' << intData << '\t';
                    break;
                case TypeReal:
                    memcpy(&floatData, (char *)data+offset, sizeof(floatData));
                    offset += sizeof(floatData);

                    std::cout << recordDescriptor[fieldIndex].name << '\t' << floatData << '\t';
                    break;
                default:
                    std::cout << "[Error] None-specofied data type." << std::endl;
                    free(nullsIndicator);
                    return -1;
            }

        }
    }

    std::cout << std::endl;

    free(nullsIndicator);

    return 0;
}

RC RecordBasedFileManager::updateRecord(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                        const void *data, const RID &rid) {
    // Given a record descriptor, update the record identified by the given rid with the passed data.

    char16_t fieldLength = recordDescriptor.size();
    int nullFieldsIndicatorActualSize = ceil((double(fieldLength)/CHAR_BIT));
    auto *nullsIndicator = (unsigned char *) malloc(nullFieldsIndicatorActualSize);
    memcpy((char *) nullsIndicator, (char *)data, nullFieldsIndicatorActualSize);

    // 1. get record length and the length ahead of variable data.
    void *LenAndValidField = (char *)malloc(8);
    getTotalLenAndLenAheadOfVariableField(recordDescriptor, data, LenAndValidField, fieldLength, nullFieldsIndicatorActualSize, nullsIndicator);

    char16_t recordLength = ((int *)LenAndValidField)[0];

    // we need to allocate at least 9bytes (4 for pageNum and 4 for slotNum and 1 for flag) for a tombstone.
    void *record;
    if(recordLength > tombstoneLen){
        record = malloc(recordLength);        // allocate a space for record with recordLength
    }
    else{
        record = malloc(tombstoneLen);
        recordLength = tombstoneLen;
    }

    // 2. We format data to record format and update it, and we need to make sure that the recordLength >= 9 bytes (pointer).
    formatRecord(recordDescriptor, data, LenAndValidField, record, fieldLength, nullFieldsIndicatorActualSize, nullsIndicator);     // convert the data to record with the format stated above.

    // 4. we find this record and deal with it.
    void *page = malloc(PAGE_SIZE);

    // we need to judge whether this record is a tombstone or not:
    char flag = ptrFlag;
    RID temp_rid = rid;

    // we first assign it to ptrFlag and check
    while (flag == ptrFlag) {
        if (fileHandle.readPage(temp_rid.pageNum, page) == 0) {
            // check whether this record is a tombstone
            char *_pSlot =
                    (char *) page + PAGE_SIZE - sizeof(PageDirectory) - (temp_rid.slotNum + 1) * sizeof(SlotDirectory);
            auto *thisSlot = (SlotDirectory *) _pSlot;

            // std::cout << "Prepare to update: " << temp_rid.pageNum << " , " << temp_rid.slotNum << std::endl;

            if (thisSlot->length == 0) {
                // this record has been deleted.
                // std::cout << "[Error] This record has already been deleted when we want to update a record." << std::endl;

                free(nullsIndicator);
                free(LenAndValidField);
                free(record);
                free(page);

                return -1;
            }

            // get the flag.
            memcpy(&flag, (char *) page + thisSlot->offset, 1);

            if (flag == recordFlag) {
                // this the the real record lives, break the loop
                break;
            } else if (flag == ptrFlag) {
                // continue to search
                char *thisTombStone = (char *) page + thisSlot->offset + 1;
                auto *newRID = (RID *) thisTombStone;
                temp_rid.pageNum = newRID->pageNum;
                temp_rid.slotNum = newRID->slotNum;

                // clear buffer
                free(thisTombStone);
                memset(page, 0, PAGE_SIZE);
            } else {
                // std::cout << "[Error] updateRecord() wrong format record. No flag byte set. " << std::endl;
                free(nullsIndicator);
                free(LenAndValidField);
                free(record);
                free(page);
                return -1;
            }
        } else {
            // std::cout << "[Error] Fail to update when reading the record." << std::endl;
            free(nullsIndicator);
            free(LenAndValidField);
            free(record);
            free(page);
            return -1;
        }
    }

    char *_pPage = (char *) page + PAGE_SIZE - sizeof(PageDirectory);
    char *_pSlot = (char *) page + PAGE_SIZE - sizeof(PageDirectory) - (temp_rid.slotNum + 1) * sizeof(SlotDirectory);
    auto *thisPage = (PageDirectory *) _pPage;
    auto *thisSlot = (SlotDirectory *) _pSlot;
    char16_t oldRecordLength = thisSlot->length;

    if (temp_rid.slotNum + 1 > thisPage->numberofslot) {
        // std::cout << "[Error] Fail to update because the slotNum is out of limit." << std::endl;
        free(nullsIndicator);
        free(LenAndValidField);
        free(record);
        free(page);
        return -1;
    }

    // manipulate in different situations.
    if (recordLength < oldRecordLength) {
        // update the record and freespace

        // free this record's space
        memset((char *) page + thisSlot->offset, 0, oldRecordLength);

        // update this new record
        memcpy((char *) page + thisSlot->offset, record, recordLength);

        thisSlot->length = recordLength;
        char16_t distance = oldRecordLength - recordLength;
        thisPage->freespace += distance;

        // shift the records
        shiftAllRecords(distance, temp_rid, page);

        // std::cout << "[Success] update a record < with RID: " << temp_rid.pageNum << " , " << temp_rid.slotNum << std::endl;

    }
    else if (recordLength == oldRecordLength) {
        // free this record's space
        memset((char *) page + thisSlot->offset, 0, oldRecordLength);

        // do nothing, just re-memcpy the length.
        memcpy((char *) page + thisSlot->offset, record, recordLength);

        // std::cout << "[Success] update a record = with RID: " << temp_rid.pageNum << " , " << temp_rid.slotNum << std::endl;

    }
    else {
        // --complicated scenario--: new record is bigger than old, so we need to delete old and insert new one.

        // First, we delete old record and shift to update freespace
        thisSlot->length = 0;
        thisPage->freespace += oldRecordLength;

        // free this record's space
        memset((char *) page + thisSlot->offset, 0, oldRecordLength);

        // shift
        shiftAllRecords(oldRecordLength, temp_rid, page);

        if (thisPage->freespace > recordLength) {
            // we could still place this new record in this page with old slot.
            thisSlot->length = recordLength;
            thisSlot->offset = PAGE_SIZE - sizeof(PageDirectory) - (thisPage->numberofslot) * sizeof(SlotDirectory) -
                               thisPage->freespace;

            // insert record
            memcpy((char *) page + thisSlot->offset, record, recordLength);
            // update freespace
            thisPage->freespace -= recordLength;

            // std::cout << "[Success] update a record with old page new space with RID: " << temp_rid.pageNum << " , " << temp_rid.slotNum << std::endl;
        }
        else {
            // the record must be migrated to a new page with enough free space.
            RID newRid;
            thisSlot->offset = PAGE_SIZE - sizeof(PageDirectory) - (thisPage->numberofslot) * sizeof(SlotDirectory) -
                               thisPage->freespace;

            if (insertRecord(fileHandle, recordDescriptor, data, newRid) == 0) {
                // put the newRID into this tombstone pointer.
                memset((char *) page + thisSlot->offset, ptrFlag, 1);

                // std::cout << "newRID: " << (int)newRid.pageNum << " , " << newRid.slotNum << std::endl;
                memcpy((char *) page + thisSlot->offset + 1, &newRid, sizeof(RID));
                // std::cout << "sizeof newRID " << sizeof(newRid) << std::endl;

                // update freespace and recordLength
                thisPage->freespace -= tombstoneLen;
                thisSlot->length = tombstoneLen;

                // std::cout << "[Success] update a record with new page with RID: " << temp_rid.pageNum << " , " << temp_rid.slotNum << std::endl;

            }
            else {
                // std::cout << "[Error] Fail to update when insert this record." << std::endl;
                free(nullsIndicator);
                free(LenAndValidField);
                free(record);
                free(page);
                return -1;
            }
        }
    }

    if (fileHandle.writePage(temp_rid.pageNum, page) == 0) {
        // success
//            std::cout << "[Success] update a record." << std::endl;

    } else {
        // std::cout << "[Error] Fail to write when update the record." << std::endl;
        free(nullsIndicator);
        free(LenAndValidField);
        free(record);
        free(page);
        return -1;
    }

    free(nullsIndicator);
    free(LenAndValidField);
    free(record);
    free(page);
    return 0;

}

RC RecordBasedFileManager::checkRecordFlag(FileHandle &fileHandle, const RID &rid){
    char *page = (char *)malloc(PAGE_SIZE);
    SlotDirectory* thisSlot;
    char flag;
    if(fileHandle.readPage(rid.pageNum, page) == 0){
        thisSlot = (SlotDirectory*)(page + PAGE_SIZE - sizeof(PageDirectory) - (rid.slotNum + 1) * sizeof(SlotDirectory));
        if(thisSlot->length == 0) {
            free(page);
            return -1;
        }
        else{
            memcpy(&flag, page+thisSlot->offset, 1);
            if(flag == recordFlag) {
                free(page);
                return 0;
            }
            else{
                free(page);
                return -1;
            }
        }
    }
    free(page);
    return -1;
}


RC RecordBasedFileManager::readAttributeFromRecord(const std::vector<Attribute> &recordDescriptor, const std::string &attributeName, void *data, void *record){

    int fieldLength = recordDescriptor.size();
    // 1. First byte is the flag
    int offsetRecord = flagLen;
    int offsetData = 0;

    //2. Next two bytes represents field length
    offsetRecord += fieldSizeLen;
    char16_t offsetVariableData;

    int nullFieldsIndicatorActualSize = ceil((double(fieldLength)/CHAR_BIT));
    auto *nullsIndicator = (unsigned char *) malloc(nullFieldsIndicatorActualSize);
//    get the indicator
    memcpy((char *) nullsIndicator, (char *)record + offsetRecord, nullFieldsIndicatorActualSize);
    offsetRecord += nullFieldsIndicatorActualSize;

    auto *nullsIndicatorForAttr = (unsigned char *) malloc(nullFieldsIndicatorActualSize);
    memset(nullsIndicatorForAttr, -1, nullFieldsIndicatorActualSize);
    offsetData += nullFieldsIndicatorActualSize;
//    std::cout << "nullFieldsIndicatorActualSize: " << nullFieldsIndicatorActualSize << std::endl;

    int nullByteIndex, bitIndex, fieldIndex;
    int varCharLen;
    char16_t varCharLen_16;

    for(nullByteIndex = 0; nullByteIndex < nullFieldsIndicatorActualSize; nullByteIndex++)
    {
        for(bitIndex = 0; bitIndex < 8; bitIndex++)
        {
            fieldIndex = nullByteIndex*8 + bitIndex;

            if(fieldIndex == fieldLength)
                break;

            // if the corresponding field is null
            // nullsIndicator bit: 1 -> null, first field correspond the leftmost bit
            if(nullsIndicator[nullByteIndex] & (unsigned) 1 << (unsigned) (7-bitIndex) && recordDescriptor[fieldIndex].name == attributeName)
            {
                // std::cout << "[Warning]: readAttributeFromRecord -> get null value." << std::endl;
                free(nullsIndicatorForAttr);
                free(nullsIndicator);
                // set the null indicator to all null
                memset(data, -1, nullFieldsIndicatorActualSize);
                return 0;
            }
            switch(recordDescriptor[fieldIndex].type)
            {
                case TypeVarChar:
//                    data = int(len) + string(name)
                    if(recordDescriptor[fieldIndex].name == attributeName)
                    {
                        varCharFromRecordToData(offsetRecord, offsetData, varCharLen_16, varCharLen, offsetVariableData, record, data);
                        nullsIndicatorForAttr[nullByteIndex] &= (~((unsigned) 1 << (unsigned) (7-bitIndex)));
                        memcpy(data, nullsIndicatorForAttr, nullFieldsIndicatorActualSize);
                        free(nullsIndicatorForAttr);
                        free(nullsIndicator);
                        return 0;
                    }
                    offsetRecord += varFieldLen;
                    break;
                case TypeInt:
                    if(recordDescriptor[fieldIndex].name == attributeName)
                    {
                        memcpy((char *)data+offsetData, (char *)record+offsetRecord, intFieldLen);
                        nullsIndicatorForAttr[nullByteIndex] &= (~((unsigned) 1 << (unsigned) (7-bitIndex)));
                        memcpy(data, nullsIndicatorForAttr, nullFieldsIndicatorActualSize);
                        free(nullsIndicatorForAttr);
                        free(nullsIndicator);
                        return 0;
                    }
                    offsetRecord += intFieldLen;
                    break;
                case TypeReal:
                    if(recordDescriptor[fieldIndex].name == attributeName)
                    {
                        memcpy((char *)data+offsetData, (char *)record+offsetRecord, realFieldLen);
                        nullsIndicatorForAttr[nullByteIndex] &= (~((unsigned) 1 << (unsigned) (7-bitIndex)));
                        memcpy(data, nullsIndicatorForAttr, nullFieldsIndicatorActualSize);
                        free(nullsIndicatorForAttr);
                        free(nullsIndicator);
                        return 0;
                    }
                    offsetRecord += realFieldLen;
                    break;
                default:
                    free(nullsIndicatorForAttr);
                    free(nullsIndicator);
                    // std::cout << "[Error] None-specofied data type." << std::endl;
                    break;
            }

        }
    }
    // std::cout << "[Error]: readAttributeFromRecord -> can't get this attribute." << std::endl;
    free(nullsIndicatorForAttr);
    free(nullsIndicator);
    return -1;
}

/*
 * returned data has nullIndicator which is consistent with recordDescriptor
 */
RC RecordBasedFileManager::readAttribute(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                         const RID &rid, const std::string &attributeName, void *data) {
    void *record = malloc(PAGE_SIZE);
    RC rc = getRecord(fileHandle, recordDescriptor, rid, record);
    if(rc == -1){
        // std::cout << "[Error]: readAttribute -> can't get the record." << std::endl;
        free(record);
        return -1;
    }
    else if(rc == -2){
//        std::cout << "[Warning]: readAttribute -> record has been deleted." << std::endl;
        free(record);
        return -2;
    }
    else{
        rc = readAttributeFromRecord(recordDescriptor, attributeName, data, record);
        if(rc == 0){
            free(record);
            return 0;
        }
        else if(rc == 1){
            free(record);
            // std::cout << "[Warning]: readAttribute -> get value 'null' for this attribute." << std::endl;
            return 1;
        }
        else{
            free(record);
            // std::cout << "[Error]: readAttribute -> can't get this attribute." << std::endl;
            return -1;
        }
    }
}

/*
 * returned data has nullIndicator which is consistent with attributeNames
 */
RC RecordBasedFileManager::readAttributes(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                         const RID &rid, const std::vector<std::string> &attributeNames, void *data){
    void *record = malloc(PAGE_SIZE);
    RC rc = getRecord(fileHandle, recordDescriptor, rid, record);
    if(rc != 0){
        // std::cout << "[Error]: readAttribute -> can't get the record." << std::endl;
        free(record);
        return -1;
    }
    else{
        readAttributesFromRecord(recordDescriptor, attributeNames, data, record);
        free(record);
        return 0;
    }
}

/*
 * Assume the order in attributeNames is the same as recordDescriptor
 * Need to format the data as the input data of insertRecord Function
 */
RC RecordBasedFileManager::readAttributesFromRecord(const std::vector<Attribute> &recordDescriptor, const std::vector<std::string> &attributeNames, void *data, void *record){

    int offsetRecord = 0;
    int offsetData = 0;
    int indexForAttributes = 0;

    int fieldLength = recordDescriptor.size();
    int attributesLength = attributeNames.size();

    // initialize null indicator for attributes
    int nullFieldsIndicatorForAttributesSize = ceil((double(attributesLength)/CHAR_BIT));
    auto *nullIndicatorForAttributes = (unsigned char *) malloc(nullFieldsIndicatorForAttributesSize);
    // must set the memory to 0
    memset(nullIndicatorForAttributes, 0, nullFieldsIndicatorForAttributesSize);
    offsetData += nullFieldsIndicatorForAttributesSize;

    // 1. First byte is the flag
    offsetRecord += flagLen;

    //2. Next two bytes represents field length
    offsetRecord += fieldSizeLen;
    char16_t offsetVariableData;

    // 3. NullIndicator
    int nullFieldsIndicatorActualSize = ceil((double(fieldLength)/CHAR_BIT));
    auto *nullsIndicator = (unsigned char *) malloc(nullFieldsIndicatorActualSize);
//    get the indicator
    memcpy((char *) nullsIndicator, (char *)record + offsetRecord, nullFieldsIndicatorActualSize);
    offsetRecord += nullFieldsIndicatorActualSize;

    // 4. iterate over attributes
    int nullByteIndex, bitIndex, fieldIndex;
    int varCharLen;
    char16_t varCharLen_16;

    for(nullByteIndex = 0; nullByteIndex < nullFieldsIndicatorActualSize; nullByteIndex++)
    {
        for(bitIndex = 0; bitIndex < 8; bitIndex++)
        {
            fieldIndex = nullByteIndex*8 + bitIndex;

            if((fieldIndex == fieldLength) || (indexForAttributes == attributesLength))
                break;

            // if the corresponding field is null
            // nullsIndicator bit: 1 -> null, first field correspond the leftmost bit
            if(nullsIndicator[nullByteIndex] & (unsigned) 1 << (unsigned) (7-bitIndex) && recordDescriptor[fieldIndex].name == attributeNames[indexForAttributes])
            {
                /*
                 * set the bit corresponding to this field to 1
                 * indexForAttributes / 8 -> byteindex
                 * 7 - indexForAttributes % 8 -> num of shifts for that bit
                 */
                nullIndicatorForAttributes[indexForAttributes/CHAR_BIT] |= ((unsigned char) 1 << (unsigned) (7 - indexForAttributes%CHAR_BIT));
                indexForAttributes++;
                continue;
            }
            if(nullsIndicator[nullByteIndex] & (unsigned) 1 << (unsigned) (7-bitIndex))
            {
                continue;
            }
            switch(recordDescriptor[fieldIndex].type)
            {
                case TypeVarChar:
//                    data = int(len) + string(name)
                    if(recordDescriptor[fieldIndex].name == attributeNames[indexForAttributes])
                    {
                        varCharFromRecordToData(offsetRecord, offsetData, varCharLen_16, varCharLen, offsetVariableData, record, data);
                        indexForAttributes++;
                    }
                    else{
                        offsetRecord += varFieldLen;
                    }

                    break;
                case TypeInt:
                    if(recordDescriptor[fieldIndex].name == attributeNames[indexForAttributes])
                    {
                        memcpy((char *)data+offsetData, (char *)record+offsetRecord, intFieldLen);
                        offsetData += 4;
                        indexForAttributes++;
                    }
                    offsetRecord += intFieldLen;
                    break;
                case TypeReal:
                    if(recordDescriptor[fieldIndex].name == attributeNames[indexForAttributes])
                    {
                        memcpy((char *)data+offsetData, (char *)record+offsetRecord, realFieldLen);
                        offsetData += 4;
                        indexForAttributes++;
                    }
                    offsetRecord += realFieldLen;
                    break;
                default:
//                    std::cout << "[Error] None-specofied data type." << std::endl;
                    break;
            }

        }
    }
    // write the nullsIndicatorForAttributes back to data
    memcpy(data, nullIndicatorForAttributes, nullFieldsIndicatorForAttributesSize);
    free(nullsIndicator);
    free(nullIndicatorForAttributes);
    return 0;
}


RC RecordBasedFileManager::scan(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                const std::string &conditionAttribute, const CompOp compOp, const void *value,
                                const std::vector<std::string> &attributeNames, RBFM_ScanIterator &rbfm_ScanIterator) {

    RC rc = rbfm_ScanIterator.initiateRBFMScanIterator(fileHandle, recordDescriptor, conditionAttribute, compOp, value, attributeNames);
    if(rc != 0){
        std::cout << "[Error]: scan -> initiateRBFMScanIterator" << std::endl;
        return -1;
    }
    return 0;
}

RC RecordBasedFileManager::findAvailablePage(FileHandle &fileHandle, char16_t recordLength, RID &rid, void* page){

    if( recordLength > (PAGE_SIZE - sizeof(PageDirectory) - sizeof(SlotDirectory)) ){
         //std::cout << "[Error] The recordLength is too large to fit in a PAGE_SIZE." << std::endl;
        return -1;
    }

    // iterate through all pages to find available space

    if(fileHandle.getNumberOfPages() > 0){
        // search from the end of all pages, which is getNumberOfPages() - 1.
        for(int i=(int)(fileHandle.getNumberOfPages() - 1); i >= 0; i--){
            // void *page = malloc(PAGE_SIZE);
            // readPage: page num starts from 0;

            if(fileHandle.readPage(i, page) == 0){
                // check freespace
                char* _pPageDir = (char*) page + PAGE_SIZE - sizeof(PageDirectory);
                auto* pageDir = (PageDirectory*) _pPageDir;

                if( pageDir->freespace >= (recordLength + sizeof(SlotDirectory)) ){
                    // this page is available
                    // check all the slotDirectory to see if exists deleted records.

                    for(unsigned int j = 0; j < pageDir->numberofslot; j++){
                        char* _pSlot = _pPageDir - (j + 1) * sizeof(SlotDirectory);
                        auto* thisSlot = (SlotDirectory*)_pSlot;
                        if(thisSlot->length == 0){
                            // this record has been deleted, we could reuse this slot.
                            // std::cout << "[Notice] Find a deleted slot which could be reused." << std::endl;
                            rid.slotNum = j;
                            rid.pageNum = i;

                            // return 1 means this slot existed and we just reuse it at this time.
                            return 1;
                        }
                    }

                    // no empty(deleted) slot. Just create a new slot.
                    rid.slotNum = pageDir->numberofslot;
                    rid.pageNum = i;

                    // return 0 means it is a new slot which we need to create.
                    return 0;
                }
            }
            else{
                return -1;
            }

        }
    }


    // Otherwise, there is no pages in the file or no available pages. So we append new page and initialize it.
    unsigned currentPageNum = fileHandle.getNumberOfPages();
    // clear buffer
    memset(page, 0, PAGE_SIZE);

    // append a new page
    if(fileHandle.appendPage(page) == 0){
        rid.pageNum = currentPageNum;
        rid.slotNum = 0;

        // add PageDirectory when append a new page
        PageDirectory pageDirectory;
        pageDirectory.numberofslot = 0;
        pageDirectory.freespace = PAGE_SIZE - sizeof(PageDirectory);

        char16_t page_offset = PAGE_SIZE - sizeof(PageDirectory);
        memcpy((char*)page + page_offset, &pageDirectory, sizeof(PageDirectory));

        return 0;
    }
    else{
        // std::cout << "[Error] appendPage() fail when findAvailablePage()" << std::endl;
        return -1;
    }

}

RC RecordBasedFileManager::varCharFromDataToRecord(int &offsetRecord, int &offsetData, char16_t &varCharLen_16, int &varCharLen, char16_t &offsetVariableData, void *record, const void *data){
    memcpy(&varCharLen, (char *)data+offsetData, 4);
    varCharLen_16 = varCharLen;
    memcpy((char *)record+offsetRecord, &varCharLen_16, 2);
    offsetData += 4;
    offsetRecord += varFieldLengthLen;  // two byte to represent length of variable field.

    memcpy((char *)record+offsetRecord, &offsetVariableData, 2);
    memcpy((char *)record+offsetVariableData, (char *)data+offsetData, varCharLen);
    offsetData += varCharLen;
    offsetRecord += varFieldOffsetLen;  // two byte to represent offset of variable data.

    offsetVariableData += varCharLen;
    return 0;
}
RC RecordBasedFileManager::varCharFromRecordToData(int &offsetRecord, int &offsetData, char16_t &varCharLen_16, int &varCharLen, char16_t &offsetVariableData, void *record, void *data){

    memcpy(&varCharLen_16, (char *)record+offsetRecord, varFieldLengthLen);
    varCharLen = varCharLen_16;
    memcpy((char *)data+offsetData, &varCharLen, 4);
    offsetRecord += varFieldLengthLen;
    offsetData += 4;

    memcpy(&offsetVariableData, (char *)record+offsetRecord, varFieldOffsetLen);
    memcpy((char *)data+offsetData, (char *)record+offsetVariableData, varCharLen);
    offsetRecord += varFieldOffsetLen;
    offsetData += varCharLen;
    return 0;
}