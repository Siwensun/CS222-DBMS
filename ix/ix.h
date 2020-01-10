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

    /*
     * Insert an entry into the given index that is indicated by the given ixFileHandle.
     *
     * If the index file is empty, we need to append the root page and insert the <key, rid> pair into rootLeaf page(here the root page is actually a leaf page, page flag is LEAF_FLAG).
     * If there is only one page, the page is rootLeaf page.
     * If one page is not enough, page 0 stores the pointer which points to the real root page, file is in the tree format.
     */
    RC insertEntry(IXFileHandle &ixFileHandle, const Attribute &attribute, const void *key, const RID &rid);

    /*
     * Delete an entry from the given index that is indicated by the given ixFileHandle.
     * Use searchEntry(...) to retrieve the desired <key, rid> pair.
     * If key == key, rid == rid, then delete the entry.
    */
    RC deleteEntry(IXFileHandle &ixFileHandle, const Attribute &attribute, const void *key, const RID &rid);

    /*
     * Initialize and IX_ScanIterator to support a range search
     * Use searchEntry(...) to get where the scan should start
     * and then ix_ScanIterator.initializeScanIterator(...) to initialize the scan.
    */
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
    
    /*
     * This method is the implementation of the pseduo-code in textbook. It is used to recursively.
     * Check textbook "Database Management System" chapter 10.5: Insert
     */
    RC insertion(IXFileHandle &ixFileHandle, const Attribute &attribute, unsigned curNode, const void *key, const RID &rid, void *newChildKey, void *newChildData, bool &splitFlag);
    
    /*
     * No matter in im node or leaf node, this function could be used to get the length for data pair by input parameter sizeOfData.
     * <key, ptr> in im node, input parameter sizeOfData is the length for ptr
     * <key, rid> for lead node, input parameter sizeOfData is the length for rid
     */
    RC getRequiredLength(const Attribute &attribute, const void *key, int sizeOfData);
    
    /*
     * Insert entry into a node page, this function is called after we get one available page which has enough space for insertion.
     * data is ptr(pageNum) for im node and rid for lead node.
     * This function also assure a order when insertion, that is key_n <= key_n+1
     * This function doesn't write back to disk, but update buffer page.
     *
    */
    RC insertEntryToNode(int pageFlag, void *page, const Attribute &attribute, const void *key, const void *data, int sizeOfData);
    
    /*
     * When we need to split a page in im node or leaf node, this function could get the split points including 3 values:
     * splitOffset: offset inside a node page where the split happens
     * splitNumOfRecords: numOfRecords ahead of the splitOffset
     * splitKey: the first pair after the splitOffset
     */
    RC getSplitInNode(int pageFlag, void *page, const Attribute &attribute, int &splitOffset, int &splitNumOfRecords, void *splitKey, int sizeOfData);
    
    /*
     * By comparing key and splitKey to determine which page the key should be allocated.
     * if key >= splitKey, go to the new page
     * else insert into the old page
     */
    RC compareAndInsertToNode(int pageFlag, void *oldPage, void *newPage, const Attribute &attribute, const void *key, void *splitKey, const void *data, int sizeOfData);
    
    /*
     * redistribute old page into oldPage and newPage for leaf node and im node.
     * im node: old page only contain the data ahead of splitOffset, new page contains the data after the splitOffset especially after the splitKey, new page doesn't contain the splitKey
     * leaf node: difference occurs in new page, new page for leaf node contains the splitKey
    */
    RC redistributeNode(IXFileHandle &ixFileHandle, int pageFlag, void *oldPage, void *newPage, int splitOffset, int splitNumOfRecords, const Attribute &attribute);
    
    /*
     * Insert <key, data> to this node with splitting, return newPageNum and splitKey for upper level process.
     * Steps:
     * 1. call getSplitInNode(...) to get the split detail
     * 2. call redistributeNode(...) to redistribute the records into old and new page
     * 3. call compareAndInsertToNode(...) to insert the key into B+ node
     * 4. append the newPage into B+ tree.
     * 5. update nextNode attribute of old page to newPageNum
    */
    RC insertEntrytoNodeWithSplitting(IXFileHandle &ixFileHandle, int pageFlag, unsigned pageNum, unsigned &newPageNum, void *page, void *splitKey, const Attribute &attribute, const void *key, const void *data, int sizeOfData);
    
    /*
     * Insert <key, data> to this node without splitting and write back the page
    */
    RC insertEntrytoNodeWithoutSplitting(IXFileHandle &ixFileHandle, unsigned pageNum, int pageFlag, void *page, const Attribute &attribute, const void *key, const void *data, int sizeOfData);
    
    /*
     * First time insert a <key, data>.
     * initialize the root-leaf page and insert it. write back the root-leaf page.
    */
    RC appendRootLeafPage(IXFileHandle &ixFileHandle, const Attribute &attribute, const void *key, const RID &rid);
    
    /*
     * If the root-leaf page is full, then we need to
     *      - create two leaf pages
            - create a root page
            - rewrite the page 0 as pointer to root page
    */
    RC splitRootLeafPage(IXFileHandle &ixFileHandle, const Attribute &attribute, const void *key, const RID &rid);
    
    /*
     * If the root page is full, then we need to
     * 1. split the old root page and this old page and new page are now two im node
     * 2. create a new root page node storing this pageNum of old and new pages. Also rootPtr stores the new root page number.
    */
    RC splitRootPage(IXFileHandle &ixFileHandle, const Attribute &attribute, unsigned rootPageNum, const void *key, const void *data);
    
    /*
     * Generate root page.
     * root page now stores two pointers: (leftPageNum | <splitKey, rightPageNum>)
    */
    RC generateNewRootNode(IXFileHandle &ixFileHandle, unsigned leftPageNum, unsigned rightPageNum, void *rootPage, const Attribute &attribute, const void *splitkey);
    
    /*
     * This function is used to search inside the B+tree to get an desired value whether the key is inclusive or not.
     * This function is used in deleteEntry and scan.
     * Loop call getNextNode to arrive get the pageNum of leafNode
     * then call searchInsideLeafNode to get the offset and recordId of this <key, rid> pair.
     */
    RC searchEntry(IXFileHandle &ixFileHandle, int &pageNum, int &offset, int &recordId, const Attribute &attribute, const void *key, bool inclusive);
    
    /*
     * This function is used in insertion to get the next node when inserting an entry (call insertion(...))
     * return a pageNum which is the node of next level to check in B+tree.
     */
    RC getNextNode(IXFileHandle &ixFileHandle, unsigned curNode, unsigned &nextNode, const Attribute &attribute, const void *key);
    
    /*
     * search inside the leaf node.
     * If inclusive is true, then offset and recordId indicates the <newKey, rid> pair where key <= newKey
     * if inclusive is false, then key < newKey strictly.
     */
    RC searchInsideLeafNode(IXFileHandle &ixFileHandle, unsigned curNode, int &offset, int &recordId, const Attribute &attribute, const void *key, bool inclusiveKey);
    
    /*
     * print one single node.
     */
    RC printNode(IXFileHandle &ixFileHandle, int pageFlag, void *page, int numOfRecords, int level, const Attribute &attribute) const;
    
    /*
     * Recursively call this function to print the B+tree.
    */
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

    /*
     * Get next matching entry
     * There are two important attributes modified inside this function, curOffset and preOffset
     * curOffset shows where the retrieve should happen
     * preOffset shows the location of last retrieve.
     * These two attributes are used in checkDeleteToUpdateCurOffset, if deletion happens, curOffset is reset to preOffset.
    */
    RC getNextEntry(RID &rid, void *key);

    // Terminate index scan
    RC close();
    
    /*
     * check whether deletion happens during the scan.
     * If deletion happens, change the curOffset to preOffset and decrement curRecordId, so that curOffset points to the next new pair and curRecordId show num of records that has been visited
     */
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
