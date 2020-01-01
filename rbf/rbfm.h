#ifndef _rbfm_h_
#define _rbfm_h_

#include <string>
#include <vector>
#include <climits>
#include <map>

#include "pfm.h"

// Record ID
typedef struct {
    unsigned pageNum;    // page number
    unsigned slotNum;    // slot number in the page
} RID;

// Attribute
typedef enum {
    TypeInt = 0, TypeReal, TypeVarChar
} AttrType;

typedef unsigned AttrLength;

struct Attribute {
    std::string name;     // attribute name
    AttrType type;     // attribute type
    AttrLength length; // attribute length
};

// each slot entry has two properties which is corresponding to each record's offset and length
typedef struct
{
    char16_t offset;
    char16_t length;
} SlotDirectory;

// Each page has one PageDirectory and one PageDirectory contains many slots -> SlotDirectory.
typedef struct
{
    char16_t numberofslot;  // start from 0
    char16_t freespace;
} PageDirectory;

// Comparison Operator (NOT needed for part 1 of the project)
typedef enum {
    EQ_OP = 0, // no condition// =
    LT_OP,      // <
    LE_OP,      // <=
    GT_OP,      // >
    GE_OP,      // >=
    NE_OP,      // !=
    NO_OP       // no condition
} CompOp;

// Field Len of each part of the record
#define fieldSizeLen 2
#define flagLen 1
#define varFieldLengthLen 2
#define varFieldOffsetLen 2
#define varFieldLen (varFieldLengthLen + varFieldOffsetLen)
#define intFieldLen 4
#define realFieldLen 4
#define tombstoneLen 9     // 4 for pageNUm, 4 for slotNum, 1 one pointer

#define recordFlag 1
#define ptrFlag 2

/********************************************************************
* The scan iterator is NOT required to be implemented for Project 1 *
********************************************************************/

# define RBFM_EOF (-1)  // end of a scan operator

// RBFM_ScanIterator is an iterator to go through records
// The way to use it is like the following:
//  RBFM_ScanIterator rbfmScanIterator;
//  rbfm.open(..., rbfmScanIterator);
//  while (rbfmScanIterator(rid, data) != RBFM_EOF) {
//    process the data;
//  }
//  rbfmScanIterator.close();

class RBFM_ScanIterator {
public:
    RBFM_ScanIterator();
    ~RBFM_ScanIterator() = default;

    // Never keep the results in the memory. When getNextRecord() is called,
    // a satisfying record needs to be fetched from the file.
    // "data" follows the same format as RecordBasedFileManager::insertRecord().
    RC initiateRBFMScanIterator(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                             const std::string &conditionAttribute, const CompOp compOp, const void *value,
                             const std::vector<std::string> &attributeNames);
    RC getNextRecord(RID &rid, void *data);
    RC getNextRID(RID &rid);
    RC close();



private:
    FileHandle *fileHandlePtr;
    CompOp compOp;
    const void *value;
    std::vector<Attribute> recordDescriptor;
    std::vector<Attribute> attrVector;
    std::string conditionAttribute;
    AttrType conditionAttributeType;
    std::vector<std::string> attributeNames;

    unsigned numOfPages;
    char16_t curNumOfSlotsInPage;
    RID curRID;
    unsigned maxAttrLength;
    unsigned maxRecordLength;

    RC getConditionAttributeType(const std::string& attribute);
    RC generateAttributesDescriptor();                      // This function is used to generate attrVector which could be used to print retrived data.
    RC updateNumOfSlots();
    RC updateCurRIDAndCurNumOfSlotsInPage();
    RC doOp(int nullSize, void *attribute);

};

class RecordBasedFileManager {
public:
    static RecordBasedFileManager &instance();                          // Access to the _rbf_manager instance

    RC createFile(const std::string &fileName);                         // Create a new record-based file

    RC destroyFile(const std::string &fileName);                        // Destroy a record-based file

    RC openFile(const std::string &fileName, FileHandle &fileHandle);   // Open a record-based file

    RC closeFile(FileHandle &fileHandle);                               // Close a record-based file

    RC getTotalLenAndLenAheadOfVariableField(const std::vector<Attribute> &recordDescriptor, const void *data, void *info,
            char16_t fieldLength, int nullsIndicatorLen, unsigned char *nullsIndicator);

    RC formatRecord(const std::vector<Attribute> &recordDescriptor, const void *data, void *info, void *record,
                    char16_t fieldLength, int nullsIndicatorLen, unsigned char *nullsIndicator);

    RC deFormatRecord(const std::vector<Attribute> &recordDescriptor, void *data, void *record);

    //  Format of the data passed into the function is the following:
    //  [n byte-null-indicators for y fields] [actual value for the first field] [actual value for the second field] ...
    //  1) For y fields, there is n-byte-null-indicators in the beginning of each record.
    //     The value n can be calculated as: ceil(y / 8). (e.g., 5 fields => ceil(5 / 8) = 1. 12 fields => ceil(12 / 8) = 2.)
    //     Each bit represents whether each field value is null or not.
    //     If k-th bit from the left is set to 1, k-th field value is null. We do not include anything in the actual data part.
    //     If k-th bit from the left is set to 0, k-th field contains non-null values.
    //     If there are more than 8 fields, then you need to find the corresponding byte first,
    //     then find a corresponding bit inside that byte.
    //  2) Actual data is a concatenation of values of the attributes.
    //  3) For Int and Real: use 4 bytes to store the value;
    //     For Varchar: use 4 bytes to store the length of characters, then store the actual characters.
    //  !!! The same format is used for updateRecord(), the returned data of readRecord(), and readAttribute().
    // For example, refer to the Q8 of Project 1 wiki page.

    // Insert a record into a file
    RC insertRecord(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor, const void *data, RID &rid);

    // Read a record identified by the given rid.
    RC readRecord(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor, const RID &rid, void *data);

    RC getRecord(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                         const RID &rid, void *record);

    // Print the record that is passed to this utility method.
    // This method will be mainly used for debugging/testing.
    // The format is as follows:
    // field1-name: field1-value  field2-name: field2-value ... \n
    // (e.g., age: 24  height: 6.1  salary: 9000
    //        age: NULL  height: 7.5  salary: 7500)
    RC printRecord(const std::vector<Attribute> &recordDescriptor, const void *data);

    // Search all pages in the file to find rid and read the info to page buffer.
    RC findAvailablePage(FileHandle &fileHandle, char16_t recordLength, RID &rid, void* page);
    /*****************************************************************************************************
    * IMPORTANT, PLEASE READ: All methods below this comment (other than the constructor and destructor) *
    * are NOT required to be implemented for Project 1                                                   *
    *****************************************************************************************************/
    // Delete a record identified by the given rid.
    RC deleteRecord(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor, const RID &rid);

    // Assume the RID does not change after an update
    RC updateRecord(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor, const void *data,
                    const RID &rid);

    // check the record corresponding to the rid whether is a tombstone
    RC checkRecordFlag(FileHandle &fileHandle, const RID &rid);

    RC readAttributeFromRecord(const std::vector<Attribute> &recordDescriptor, const std::string &attributeName, void *data, void *record);

    // Read an attribute given its name and the rid.
    RC readAttribute(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor, const RID &rid,
                     const std::string &attributeName, void *data);

    RC readAttributesFromRecord(const std::vector<Attribute> &recordDescriptor, const std::vector<std::string> &attributeNames, void *data, void *record);

    RC readAttributes(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                             const RID &rid, const std::vector<std::string> &attributeNames, void *data);
    // Scan returns an iterator to allow the caller to go through the results one by one.
    RC scan(FileHandle &fileHandle,
            const std::vector<Attribute> &recordDescriptor,
            const std::string &conditionAttribute,
            const CompOp compOp,                  // comparision type such as "<" and "="
            const void *value,                    // used in the comparison
            const std::vector<std::string> &attributeNames, // a list of projected attributes
            RBFM_ScanIterator &rbfm_ScanIterator);

    RC varCharFromDataToRecord(int &offsetRecord, int &offsetData, char16_t &varCharLen_16, int &varCharLen, char16_t &offsetVariableData, void *record, const void *data);
    RC varCharFromRecordToData(int &offsetRecord, int &offsetData, char16_t &varCharLen_16, int &varCharLen, char16_t &offsetVariableData, void *record, void *data);
    // This function is used for when you delete a record and need to compact the page.
   RC shiftAllRecords(char16_t distance, const RID &rid, void* page);


protected:
    RecordBasedFileManager();                                                   // Prevent construction
    ~RecordBasedFileManager();                                                  // Prevent unwanted destruction
    RecordBasedFileManager(const RecordBasedFileManager &);                     // Prevent construction by copying
    RecordBasedFileManager &operator=(const RecordBasedFileManager &);          // Prevent assignment


private:
    static RecordBasedFileManager *_rbf_manager;
};

#endif