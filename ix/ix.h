#ifndef _ix_h_
#define _ix_h_

#include <vector>
#include <string>

#include "../rbf/rbfm.h"

# define IX_EOF (-1)  // end of the index scan

// avoid the issue of align
# define IM_DIR_SIZE 12     // Root and im both use this
# define LEAF_DIR_SIZE 16
/**
 * Flag:
    0: empty
    1: root/intermediate
    2: root-leaf/leaf
 */
# define EMPTY_FLAG 0
# define ROOT_PTR_FLAG 1    // root node stores pointer
# define ROOT_FLAG 2        // root node
# define IM_FLAG 3          //intermediate node
# define LEAF_FLAG 4        // leaf node

# define ROOT_PAGE 0

class IX_ScanIterator;

class IXFileHandle;

// we assume write Directory at the beginning of each page

// Intermediate node PageDirectory
typedef struct
{
    int flag;
    int freeSpace;
    int numOfRecords;
} imPageDirectory;

// Leaf node PageDirectory
typedef struct
{
    int flag;
    int freeSpace;
    int numOfRecords;
    int nextNode; // pageNum, Linklist
} leafPageDirectory;

class IndexManager {

public:
    static IndexManager &instance();

    // Create an index file.
    RC createFile(const std::string &fileName);

    // Delete an index file.
    RC destroyFile(const std::string &fileName);

    // Open an index and return an ixFileHandle.
    RC openFile(const std::string &fileName, IXFileHandle &ixFileHandle);

    // Close an ixFileHandle for an index.
    RC closeFile(IXFileHandle &ixFileHandle);

    // Insert an entry into the given index that is indicated by the given ixFileHandle.
    RC insertEntry(IXFileHandle &ixFileHandle, const Attribute &attribute, const void *key, const RID &rid);

    // Delete an entry from the given index that is indicated by the given ixFileHandle.
    RC deleteEntry(IXFileHandle &ixFileHandle, const Attribute &attribute, const void *key, const RID &rid);

    // Initialize and IX_ScanIterator to support a range search
    RC scan(IXFileHandle &ixFileHandle,
            const Attribute &attribute,
            const void *lowKey,
            const void *highKey,
            bool lowKeyInclusive,
            bool highKeyInclusive,
            IX_ScanIterator &ix_ScanIterator);

    // Print the B+ tree in pre-order (in a JSON record format)
    void printBtree(IXFileHandle &ixFileHandle, const Attribute &attribute) const;
    
//    RC searchInside

protected:
    IndexManager() = default;                                                   // Prevent construction
    ~IndexManager() = default;                                                  // Prevent unwanted destruction
    IndexManager(const IndexManager &) = default;                               // Prevent construction by copying
    IndexManager &operator=(const IndexManager &) = default;                    // Prevent assignment
    
    RC insertion(IXFileHandle &ixFileHandle, const Attribute &attribute, unsigned curNode, const void *key, const RID &rid, void *newChildKey, void *newChildData, bool &splitFlag);
    
    RC getRequiredLength(const Attribute &attribute, const void *key, int sizeOfData);
    
    RC insertEntryToNode(int pageFlag, void *page, const Attribute &attribute, const void *key, const void *data, int sizeOfData);
    
    RC searchEntry(IXFileHandle &ixFileHandle, int &pageNum, int &offset, int &recordId, const Attribute &attribute, const void *key, bool inclusive);
    
    RC getSplitInNode(int pageFlag, void *page, const Attribute &attribute, int &splitOffset, int &splitNumOfRecords, void *splitKey, int sizeOfData);
    
    RC compareAndInsertToNode(int pageFlag, void *oldPage, void *newPage, const Attribute &attribute, const void *key, void *splitKey, const void *data, int sizeOfData);
    
    RC redistributeNode(IXFileHandle &ixFileHandle, int pageFlag, void *oldPage, void *newPage, int splitOffset, int splitNumOfRecords, const Attribute &attribute);
    
    // Above functions are helper function
    
    RC insertEntrytoNodeWithSplitting(IXFileHandle &ixFileHandle, int pageFlag, unsigned pageNum, unsigned &newPageNum, void *page, void *splitKey, const Attribute &attribute, const void *key, const void *data, int sizeOfData);
    
    RC insertEntrytoNodeWithoutSplitting(IXFileHandle &ixFileHandle, unsigned pageNum, int pageFlag, void *page, const Attribute &attribute, const void *key, const void *data, int sizeOfData);
    
    RC appendRootLeafPage(IXFileHandle &ixFileHandle, const Attribute &attribute, const void *key, const RID &rid);
    
    RC splitRootLeafPage(IXFileHandle &ixFileHandle, const Attribute &attribute, const void *key, const RID &rid);
    
    RC splitRootPage(IXFileHandle &ixFileHandle, const Attribute &attribute, unsigned rootPageNum, const void *key, const void *data);
    
    RC generateNewRootNode(IXFileHandle &ixFileHandle, unsigned leftPageNum, unsigned rightPageNum, void *rootPage, const Attribute &attribute, const void *splitkey);
    
    RC getNextNode(IXFileHandle &ixFileHandle, unsigned curNode, unsigned &nextNode, const Attribute &attribute, const void *key);
    
    RC searchInsideLeafNode(IXFileHandle &ixFileHandle, unsigned curNode, int &offset, int &recordId, const Attribute &attribute, const void *key, bool inclusiveKey);
    
    RC printNode(IXFileHandle &ixFileHandle, int pageFlag, void *page, int numOfRecords, int level, const Attribute &attribute) const;
    
    RC printNormalBtree(IXFileHandle &ixFileHandle, int curNode, int level, const Attribute &attribute, bool isLastKey) const;

};

class IX_ScanIterator {
public:

    // Constructor
    IX_ScanIterator();

    // Destructor
    ~IX_ScanIterator();

    RC initializeScanIterator(IXFileHandle &ixFileHandle,
                              const Attribute &attribute,
                              const void *lowKey,
                              const void *highKey,
                              bool lowKeyInclusive,
                              bool highKeyInclusive,
                              int curNode,
                              int curOffset,
                              int curRecordId);

    // Get next matching entry
    RC getNextEntry(RID &rid, void *key);

    // Terminate index scan
    RC close();
    
    RC checkDeleteToUpdateCurOffset(void *page);

private:
    int curNode;
    int curOffset;
    // previous offset inside each page
    int preOffset;
    RID preRid;
    int curRecordId;
    char *curPage;
    leafPageDirectory curLeafPageDir;
    IXFileHandle *ixFileHandlePtr;
    Attribute attribute;
    const void *lowKey;
    const void *highKey;
    bool lowKeyInclusive;
    bool highKeyInclusive;

};

class IXFileHandle {
public:

    // variables to keep counter for each operation
    unsigned ixReadPageCounter;
    unsigned ixWritePageCounter;
    unsigned ixAppendPageCounter;
    
    static IXFileHandle &instance();                          // Access to the IXFileHandle instance
    // Constructor
    IXFileHandle();

    // Destructor
    ~IXFileHandle();

    // Put the current counter values of associated PF FileHandles into variables
    RC collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount);

    FileHandle& getFileHandle();

private:
    FileHandle fileHandle;
};

#endif
