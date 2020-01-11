#ifndef _rm_h_
#define _rm_h_

#include <string>
#include <vector>

#include "../rbf/rbfm.h"
#include "../ix/ix.h"

# define TABLE_NAME "Tables"
# define COLUMN_NAME "Columns"
# define INDEX_NAME "Indexes"

# define RM_EOF (-1)  // end of a scan operator
# define TypeVarCharLen 50
# define TypeIntLen 4
# define TypeRealLen 4

typedef enum {
    systemFlag = 0,
    customFlag
} Flag;

// RM_ScanIterator is an iterator to go through tuples
class RM_ScanIterator {
public:
    RM_ScanIterator() = default;
    
    ~RM_ScanIterator() = default;
    
    // "data" follows the same format as RelationManager::insertTuple()
    RC getNextTuple(RID &rid, void *data);
    
    RC close();
    
    FileHandle &getFileHandle(){
        return _fileHandle;
    }
    
    RBFM_ScanIterator &getRBFMScanIterator(){
        return _rbfmScanItearator;
    };
private:
    RBFM_ScanIterator _rbfmScanItearator;
    FileHandle _fileHandle;
};

// RM_IndexScanIterator is an iterator to go through index entries
class RM_IndexScanIterator {
public:
    RM_IndexScanIterator() = default;    // Constructor
    ~RM_IndexScanIterator() = default;    // Destructor
    
    // "key" follows the same format as in IndexManager::insertEntry()
    RC getNextEntry(RID &rid, void *key);    // Get next matching entry
    RC close();                        // Terminate index scan
    
    IXFileHandle &getIXFileHandle(){
        return _ixFileHandle;
    }
    
    IX_ScanIterator &getIXScanIterator(){
        return _ixScanItearator;
    };
private:
    IX_ScanIterator _ixScanItearator;
    IXFileHandle _ixFileHandle;
};

// Relation Manager
class RelationManager {
public:
    static RelationManager &instance();
    
    /*
     * system catalog:
     * Tables (table-id:int, table-name:varchar(50), file-name:varchar(50))
     * Columns(table-id:int, column-name:varchar(50), column-type:int, column-length:int, column-position:int)
     * Indexes(table-name:varchar(50), column-name:varchar(50), index-name:varchar(50))
     *
     * Insert three record into Tables, each one is corresponding to a catalog table.
     * Insert three descriptor into Columns.
     */
    RC createCatalog();
    
    /*
     * destroy three catalog files.
     */
    RC deleteCatalog();
    
    /*
     * get tableId. tableId is stored in hidden page after three pageCounter: readPageCounter, writePageCounter and appendPageCounter.
     * Increase the tableId as table id for new table.
     * Then insert into Tables and Columns.
     */
    RC createTable(const std::string &tableName, const std::vector<Attribute> &attrs);
    
    /*
     * Key: when delete a table, need to delete record in three catalog files and also associated index file.
     * deleteTableInsideTables(...), deleteTableInsideColumns(...), deleteTableInsideIndexes(...)
     */
    RC deleteTable(const std::string &tableName);
    
    /*
     * get all the attributes of this table name.
     * call getTableIdForCustomTable(...) to get tableId and getAttributesGivenTableId(...) to get attributes.
     */
    RC getAttributes(const std::string &tableName, std::vector<Attribute> &attrs);
    
    /*
     * when insert a record into a table. fisrt insert into heap file, then index file.
     * First call getAttributes(...) to get the descriptor.
     * Then _rbfm->insertRecord(...) to insert the record
     * Finally, also need to check whether need to update the index files. generateCoumnIndexMapGivenTable(...) and indexOperationWhenTupleChanged
     */
    RC insertTuple(const std::string &tableName, const void *data, RID &rid);
    
    /*
     * Difference from insert is first to delete from index files then heap file.
     * reason is simple, heap file is first used to retrieve the key value which is used in _im->deleteEntry.
     * After all the potential index deleted, then delete from heap file.
     */
    RC deleteTuple(const std::string &tableName, const RID &rid);
    
    /*
     * Update is like the combination of deleteTuple and insertTuple. But use _rbfm->updateRecord instead of _rbfm->insertRecord
     */
    RC updateTuple(const std::string &tableName, const void *data, const RID &rid);
    
    /*
     * use getAttributes(...) to get the custom descriptor.
     * _rbfm->readRecord() to read the record
     */
    RC readTuple(const std::string &tableName, const RID &rid, void *data);
    
    /*
     * Print a tuple that is passed to this utility method.
     * The format is the same as printRecord().
    */
    RC printTuple(const std::vector<Attribute> &attrs, const void *data);
    
    /*
     * use getAttributes(...) to get the custom descriptor.
     * _rbfm->readAttribute() to read attribute
     */
    RC readAttribute(const std::string &tableName, const RID &rid, const std::string &attributeName, void *data);
    
    /*
     * Scan returns an iterator to allow the caller to go through the results one by one.
     * Do not store entire results in the scan iterator.
     * Using RM_ScanIterator is essentially using RBFM_ScanIterator
    */
    RC scan(const std::string &tableName,
            const std::string &conditionAttribute,
            const CompOp compOp,                  // comparison type such as "<" and "="
            const void *value,                    // used in the comparison
            const std::vector<std::string> &attributeNames, // a list of projected attributes
            RM_ScanIterator &rm_ScanIterator);
    
    /*
     * This method creates an index on a given attribute of a given table. (It should also reflect its existence in the catalogs.)
     * This function could and only could be called once for this tableName and attributeName combination.
     * First, insert record into Indexes catalog file.
     * Then scan all the record inside heap file, build a index file on this attributeName.
     */
    RC createIndex(const std::string &tableName, const std::string &attributeName);
    
    /*
     * This method destroys an index on a given attribute of a given table. (It should also reflect its non-existence in the catalogs.)
     * delete the index file and use RBFMScanIterator to delete the record inside Indexes catalog file.
     */
    RC destroyIndex(const std::string &tableName, const std::string &attributeName);
    
    /*
     * indexScan returns an iterator to allow the caller to go through qualified entries in index
     * Using RM_IndexScanIterator is essentially using IX_ScanIterator.
    */
    RC indexScan(const std::string &tableName,
                 const std::string &attributeName,
                 const void *lowKey,
                 const void *highKey,
                 bool lowKeyInclusive,
                 bool highKeyInclusive,
                 RM_IndexScanIterator &rm_IndexScanIterator);

// Extra credit work (10 points)
    RC addAttribute(const std::string &tableName, const Attribute &attr);
    
    RC dropAttribute(const std::string &tableName, const std::string &attributeName);

protected:
    RelationManager();                                                  // Prevent construction
    ~RelationManager();                                                 // Prevent unwanted destruction
    RelationManager(const RelationManager &);                           // Prevent construction by copying
    RelationManager &operator=(const RelationManager &);                // Prevent assignment
    
    /*
     * hard code the attributes of three catalog table descriptors.
     * Tables (table-id:int, table-name:varchar(50), file-name:varchar(50))
     * Columns(table-id:int, column-name:varchar(50), column-type:int, column-length:int, column-position:int)
     * Indexes(table-name:varchar(50), column-name:varchar(50), index-name:varchar(50))
     */
    RC prepareTablesDescriptor();
    RC prepareColumnsDescriptor();
    RC prepareIndexesDescriptor();
    
    /*
     * insert record into Tables
     * fileHandle already opens table Tables, call prepareTablesRecord(...) to prepare the record given tableId, tableName and fileName.
     * Then call rbfm.insertRecord(...) to insert the record into Tables.
     */
    RC insertRecordToTables(FileHandle &fileHandle, RecordBasedFileManager &rbfm, int tableId, std::string tableName, std::string fileName);
    
    /*
     * insert the attributes from a targetDescriptor into Columns.
     * for loop call insertRecordToColumns(...) to do the job.
     */
    RC insertDescriptorToColumns(FileHandle &fileHandle, RecordBasedFileManager &rbfm, int tableId, std::vector<Attribute> targetDescriptor);
    
    /*
     * Same as insertRecordToTables(...), but prepare Columns record and insert the record into Columns catalog file.
     * fileHandle already opens table Columns, call prepareColumnsRecord(...) to prepare the record given tableId, columnName, columnType, columnLength and columnPosition.
     * Then call rbfm.insertRecord(...) to insert the record into Columns.
    */
    RC insertRecordToColumns(FileHandle &fileHandle, RecordBasedFileManager &rbfm, int tableId, std::string columnName, int columnType, int columnLength, int columnPosition);
    
    /*
     * Same as insertRecordToTables(...), but prepare Indexes record and insert the record into Indexes catalog file.
     * fileHandle already opens table INdexes, call prepareIndexesRecord(...) to prepare the record given tableName, columnName and indexName.
     * Then call rbfm.insertRecord(...) to insert the record into Indexes.
    */
    RC insertRecordToIndexes(FileHandle &fileHandle, RecordBasedFileManager &rbfm, std::string tableName, std::string columnName, std::string indexName);
    
    /*
     * The following three function are used to format the data into target format, as the original input data format in rbfm.cc. That is nullIndicator followed by attributes. And for varChar, varCharLen is followed by the actual varChar data.
     * Tables (table-id:int, table-name:varchar(50), file-name:varchar(50))
     * Columns(table-id:int, column-name:varchar(50), column-type:int, column-length:int, column-position:int)
     * Indexes(table-name:varchar(50), column-name:varchar(50), index-name:varchar(50))
     */
    RC prepareTablesRecord(int tableId, std::string tableName, std::string fileName, void *data);
    RC prepareColumnsRecord(int tableId, std::string columnName, int columnType, int columnLength, int columnPosition, void *data);
    RC prepareIndexesRecord(std::string tableName, std::string columnName, std::string indexName, void *data);
    
    /*
     * The following three functions are used in the upper functions to format the data into target format.
     */
    RC prepareVarChar(int &length, std::string &varChar, void *data, int &offset);
    RC prepareInt(int &value, void *data, int &offset);
    RC prepareFloat(float &value, void *data, int &offset);
    
    /*
     * Use RBFM_ScanIterator to scan the catalog file - Tables to get the tableId for this table.
     * This function also return a rid which maybe used in other function like, deleteTableInsideTables(...)
     */
    RC getTableIdForCustomTable(const std::string &tableName, int &tableId, RID &rid);
    
    /*
     * Since we already get tableId by last function, then we can use this function to get a customDescriptor which is attrs.
     * Similarly, we use RBFM_ScanIterator to get the target result, there are multiple output corresponding to different attributes of this table.
     * For each output, call getAttributeFromData to generate an Attribute, added into customDescriptor which is attrs
     */
    RC getAttributesGivenTableId(int tableId, std::vector<Attribute> &attrs);
    
    /*
     * Use attrNames and data to generate an Attribute.
     * Attribute struct has three variables: name, type and length.
     * data also contains three value from RBFM_ScanIterator: column-name, column-type and column-length.
     */
    Attribute getAttributeFromData(std::vector<std::string> attrNames, void *data);
    
    /*
     * seach tableID inside Tables with tableName, and delete this row.
     * getTableIdForCustomTable(...) to get the tableId and rid, then _rbfm->deleteRecord(...) to delete this record with rid.
     */
    RC deleteTableInsideTables(const std::string tableName, int &tableId);
    
    /*
     * Given tableId, use RBFM_ScanIterator to get rid where the retrieved tableId == tableId, then delete this row with rid.
     */
    RC deleteTableInsideColumns(int &tableId);
    
    /*
     * Use generateCoumnIndexMapGivenTable(...) to retrieve all the available rids then delete the record in Indexes file using these rids.
     * Not only delete record inside Indexes catalog file, also delete the index files.
     */
    RC deleteTableInsideIndexes(const std::string tableName);
    
    /*
     * This function generate a map which is <<columnName, IndexName>, rid>
     * <columnsName, IndexName> could be used in indexOperationWhenTupleChanged(...)
     */
    RC generateCoumnIndexMapGivenTable(const std::string &tableName, std::map<std::pair<std::string, std::string>, RID> &columnIndexMap);
    
    /*
     * This function is used for Indexes catalog file operation.
     * call _rbfm->readAttribute(...) to get the target attribute value given columnName from map.
     * Change all the index files associate to this table.
     * if operationFlag == 0: insertion.
     * if operationFlag == 1: deletion.
     */
    RC indexOperationWhenTupleChanged(const std::string &tableName, const RID rid, const std::map<std::pair<std::string, std::string>, RID> columnIndexMap, const std::vector<Attribute> attrs, int operationFlag);
    
private:
    static RelationManager *_relation_manager;
    RecordBasedFileManager *_rbfm;
    IndexManager *_im;
    
    std::vector<Attribute> _tablesDescriptor;
    std::vector<Attribute> _columnsDescriptor;
    std::vector<Attribute> _indexesDescriptor;
};

#endif