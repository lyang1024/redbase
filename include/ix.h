//
// ix.h
//
//   Index Manager Component Interface
//

#ifndef IX_H
#define IX_H

#include "redbase.h"  // Please don't change these lines
#include "rm_rid.h"  // Please don't change these lines
#include "pf.h"
#include <string>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <stack>
#include "mbr.h"

struct IX_IndexHeader{
	AttrType attr_type;
	int attr_length;
	int entryOffset_N;
	int entryOffset_B;

	//int numEntryOffset_N;

	int M;
	int m;
	int Mb;

	PageNum rootPage;

	
};


//
// IX_IndexHandle: IX Index File interface
//
class IX_IndexHandle {
public:
    IX_IndexHandle();
    ~IX_IndexHandle();

    // Insert a new index entry
    RC InsertEntry(void *pData, const RID &rid);

    // Delete a new index entry
    RC DeleteEntry(void *pData, const RID &rid);

    // Force index files to disk
    RC ForcePages();
    PF_FileHandle pfh;
	PF_PageHandle rootPH;
	struct IX_IndexHeader header;
    int (*comparator) (void*, void*, int);
	//bool dirtyHeader;
	bool isOpenHandle;
	bool header_modified;
private:
	
	
	
	//some helper methods
	RC ChooseLeaf(PageNum rPN, void *pData, PageNum &result);
	RC ChooseNode(struct IX_NodeEntry &nEntry, int depth, int height, PageNum &result);
    RC CreateNode(PF_PageHandle &ph, PageNum &page, char *&nData, bool isLeaf, PageNum parent, int myindex);
	RC InsertToBucket(PageNum bucketPage, const RID &rid);
	RC CreateBucket(PageNum &page);
    RC FindPrevIndex(struct IX_NodeHeader *nHeader, int thisindex, int &previndex);
    //RC InsertToBucket(PageNum bucketPage, const RID &rid);
    
	float getArea(void *pData);
	float getExpansion(void *pData1, void *pData2);
    void adjustMBR(struct MBR &m1, struct MBR m2);
    RC SplitNode(struct IX_NodeHeader *h1, struct IX_NodeHeader *newHeader, struct IX_NodeEntry newentry, PageNum &newPageNum);
    int PickNext(struct IX_NodeEntry *entries, std::vector<bool> &table, void* pData, struct MBR m1, struct MBR m2);
    RC PickSeeds(struct IX_NodeHeader *h1,void *pData, int &index1, int &index2);
    void getMBR(struct IX_NodeEntry *entries, int numEntries, struct MBR &resultmbr);
    RC AdjustTree(struct IX_NodeHeader *chosenleaf, struct MBR mbr);
    RC AdjustTree(PageNum pn1, PageNum pn2);
	RC DeleteFromBucket(struct IX_BucketHeader *bHeader, const RID &rid, bool &deletePage, RID &lastRID, PageNum &nextPage);
    RC DeleteFromNode(PageNum nodePage, void* pData, const RID &rid);
    RC FindLeaf(PageNum rootPage, void *pData, const RID &rid, PageNum &result);
	RC CondenseTree(PageNum LeafPN);
    bool IsEqualMBR(struct MBR m1, struct MBR m2);
    bool Overlap(struct MBR rt1, struct MBR rt2);

};

//
// IX_IndexScan: condition-based scan of index entries
//
class IX_IndexScan {
public:
    IX_IndexScan();
    ~IX_IndexScan();

    // Open index scan
    RC OpenScan(const IX_IndexHandle &indexHandle,
                CompOp compOp,
                void *value,
                ClientHint  pinHint = NO_HINT);


    // Get the next matching entry return IX_EOF if no more matching
    // entries.
    RC GetNextEntry(RID &rid);

    // Close index scan
    RC CloseScan();
private:
	//bool isOpen;
    RC BeginScan(PF_PageHandle &leafPH, PageNum &pageNum);
    // Sets up the scan private variables to the first entry within the given leaf
    RC GetFirstEntryInLeaf(PF_PageHandle &leafPH);
    // Sets up the scan private variables to the appropriate entry within the given leaf
    RC GetAppropriateEntryInLeaf(PF_PageHandle &leafPH);
    // Sets up the scan private variables to the first entry within this bucket
    RC GetFirstBucketEntry(PageNum nextBucket, PF_PageHandle &bucketPH);
    // Sets up the scan private variables to the next entry in the index
    RC FindNextValue();
	RC FindNextEntry(struct IX_NodeHeader *nh, int current, int &result);
    // Sets the RID
    RC SetRID(bool setCurrent);
	RC FindFirstLeaf();


    bool openScan;              // Indicator for whether the scan is being used
    IX_IndexHandle *indexHandle;// Pointer to the indexHandle that modifies the
    // file that the scan will try to traverse

    // The comparison to determine whether a record satisfies given scan conditions
    bool (*comparator) (void *, void*, AttrType, int);
    int attrLength;     // Comparison type, and information about the value
    void *value;        // to compare to
    AttrType attrType;
    CompOp compOp;

    bool scanEnded;     // Indicators for whether the scan has started or
    bool scanStarted;   // ended

    //PF_PageHandle currLeafPH;   // Currently pinned Leaf and Bucket PageHandles
    //PF_PageHandle currBucketPH; // that the scan is accessing

    char *currKey;              // the keys of the current record, and the following
    char *nextKey;              // two records after that
    char *nextNextKey;
    struct IX_NodeHeader_L *leafHeader;     // the scan's current leaf and bucket header
    struct IX_BucketHeader *bucketHeader;   // and entry and key pointers
    struct Node_Entry *leafEntries;
    struct Bucket_Entry *bucketEntries;
    char * leafKeys;
    int leafSlot;               // the current leaf and bucket slots of the scan
    int bucketSlot;
    PageNum currLeafNum;        // the current and next bucket slots of the scan
    PageNum currBucketNum;
    PageNum nextBucketNum;

    RID currRID;    // the current RID and the next RID in the scan
    RID nextRID;

    bool hasBucketPinned; // whether the scan has pinned a bucket or a leaf page
    bool hasLeafPinned;
    bool initializedValue; // Whether value variable has been initialized (malloced)
    bool endOfIndexReached; // Whether the end of the scan has been reached


    bool foundFirstValue;
    bool foundLastValue;
    bool useFirstLeaf;

	//std::stack<IX_NodeHeader*> path;
	std::stack<PageNum> path;
	std::vector<int> indices;
};

//
// IX_Manager: provides IX index file management
//
class IX_Manager {
public:
    IX_Manager(PF_Manager &pfm);
    ~IX_Manager();

    // Create a new Index
    RC CreateIndex(const char *fileName, int indexNo,
                   AttrType attrType, int attrLength);

    // Destroy and Index
    RC DestroyIndex(const char *fileName, int indexNo);

    // Open an Index
    RC OpenIndex(const char *fileName, int indexNo,
                 IX_IndexHandle &indexHandle);

    // Close an Index
    RC CloseIndex(IX_IndexHandle &indexHandle);

	PF_Manager &pfm;
private:
	RC initIxHandle(IX_IndexHandle &ih, PF_FileHandle &fh, struct IX_IndexHeader* header);
	RC clearIxHandle(IX_IndexHandle &indexHandle);
	RC CleanUpIH(IX_IndexHandle &indexHandle);
    RC SetUpIH(IX_IndexHandle &ih, PF_FileHandle &fh, struct IX_IndexHeader *header);
};

//
// Print-error function
//
void IX_PrintError(RC rc);

#define IX_BADINDEXSPEC         (START_IX_WARN + 0) // Bad Specification for Index File
#define IX_BADINDEXNAME         (START_IX_WARN + 1) // Bad index name
#define IX_INVALIDINDEXHANDLE   (START_IX_WARN + 2) // FileHandle used is invalid
#define IX_INVALIDINDEXFILE     (START_IX_WARN + 3) // Bad index file
#define IX_NODEFULL             (START_IX_WARN + 4) // A node in the file is full
#define IX_BADFILENAME          (START_IX_WARN + 5) // Bad file name
#define IX_INVALIDBUCKET        (START_IX_WARN + 6) // Bucket trying to access is invalid
#define IX_DUPLICATEENTRY       (START_IX_WARN + 7) // Trying to enter a duplicate entry
#define IX_INVALIDSCAN          (START_IX_WARN + 8) // Invalid IX_Indexscsan
#define IX_INVALIDENTRY         (START_IX_WARN + 9) // Entry not in the index
#define IX_EOF                  (START_IX_WARN + 10)// End of index file
#define IX_LASTWARN             IX_EOF

#define IX_ERROR                (START_IX_ERR - 0) // error
#define IX_LASTERROR            IX_ERROR

#endif
