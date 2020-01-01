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
    
    RC createCatalog();
    
    RC deleteCatalog();
    
    RC createTable(const std::string &tableName, const std::vector<Attribute> &attrs);
    
    RC deleteTable(const std::string &tableName);
    
    RC getAttributes(const std::string &tableName, std::vector<Attribute> &attrs);
    
    RC insertTuple(const std::string &tableName, const void *data, RID &rid);
    
    RC deleteTuple(const std::string &tableName, const RID &rid);
    
    RC updateTuple(const std::string &tableName, const void *data, const RID &rid);
    
    RC readTuple(const std::string &tableName, const RID &rid, void *data);
    
    // Print a tuple that is passed to this utility method.
    // The format is the same as printRecord().
    RC printTuple(const std::vector<Attribute> &attrs, const void *data);
    
    RC readAttribute(const std::string &tableName, const RID &rid, const std::string &attributeName, void *data);
    
    // Scan returns an iterator to allow the caller to go through the results one by one.
    // Do not store entire results in the scan iterator.
    RC scan(const std::string &tableName,
            const std::string &conditionAttribute,
            const CompOp compOp,                  // comparison type such as "<" and "="
            const void *value,                    // used in the comparison
            const std::vector<std::string> &attributeNames, // a list of projected attributes
            RM_ScanIterator &rm_ScanIterator);
    
    RC createIndex(const std::string &tableName, const std::string &attributeName);
    
    RC destroyIndex(const std::string &tableName, const std::string &attributeName);
    
    // indexScan returns an iterator to allow the caller to go through qualified entries in index
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
    
    RC prepareTablesDescriptor();
    RC prepareColumnsDescriptor();
    RC prepareIndexesDescriptor();
    
    RC insertRecordToTables(FileHandle &fileHandle, RecordBasedFileManager &rbfm, int tableId, std::string tableName, std::string fileName);
    RC insertDescriptorToColumns(FileHandle &fileHandle, RecordBasedFileManager &rbfm, int tableId, std::vector<Attribute> targetDescriptor);
    RC insertRecordToColumns(FileHandle &fileHandle, RecordBasedFileManager &rbfm, int tableId, std::string columnName, int columnType, int columnLength, int columnPosition);
    RC insertRecordToIndexes(FileHandle &fileHandle, RecordBasedFileManager &rbfm, std::string tableName, std::string columnName, std::string indexName);
    
    RC prepareTablesRecord(int tableId, std::string tableName, std::string fileName, void *data);
    RC prepareColumnsRecord(int tableId, std::string columnName, int columnType, int columnLength, int columnPosition, void *data);
    RC prepareIndexesRecord(std::string tableName, std::string columnName, std::string indexName, void *data);
    
    RC prepareVarChar(int &length, std::string &varChar, void *data, int &offset);
    RC prepareInt(int &value, void *data, int &offset);
    RC prepareFloat(float &value, void *data, int &offset);
    
    RC getTableIdForCustomTable(const std::string &tableName, int &tableId, RID &rid);
    RC getAttributesGivenTableId(int tableId, std::vector<Attribute> &attrs);
    Attribute getAttributeFromData(std::vector<std::string> attrNames, void *data);
    RC deleteTableInsideTables(const std::string tableName, int &tableId);
    RC deleteTableInsideColumns(int &tableId);
    RC deleteTableInsideIndexes(const std::string tableName);
    
    RC generateCoumnIndexMapGivenTable(const std::string &tableName, std::map<std::pair<std::string, std::string>, RID> &columnIndexMap);
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