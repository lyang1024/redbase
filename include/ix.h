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
	bool dirtyHeader;
private:
	
	
	
	//some helper methods
	RC ChooseLeaf(struct IX_NodeHeader *rHeader, void *pData, PageNum &result);
    RC CreateNode(PF_PageHandle &ph, PageNum &page, char *nData, bool isLeaf, PageNum parent, int myindex);
	RC InsertToBucket(PageNum bucketPage, const RID &rid);
	RC CreateBucket(PageNum &page);
    RC FindPrevIndex(struct IX_NodeHeader *nHeader, int thisindex, int &previndex);
    //RC InsertToBucket(PageNum bucketPage, const RID &rid);
    
	float getArea(void *pData);
	float getExpansion(void *pData1, void *pData2);
    void adjustMBR(struct MBR &m1, struct MBR m2);
    RC SplitNode(struct IX_NodeHeader *h1, struct IX_NodeHeader *newHeader, struct IX_NodeEntry newentry, PageNum &newPageNum);
    int PickNext(struct IX_NodeEntry *entries, std::vector<bool> &table, void* pData, struct MBR m1, struct MBR m2);
    RC PickSeeds(struct IX_NodeEntry *entries, int NumEntries, void *pData, int &index1, int &index2);
    void getMBR(struct IX_NodeEntry *entries, int numEntries, struct MBR &resultmbr);
    RC AdjustTree(struct IX_NodeHeader *chosenleaf, struct MBR mbr);
    RC AdjustTree(PageNum pn1, PageNum pn2);

    /*
    RC DeleteFromNode(PageNum nodePage, void* pData, const RID &rid, bool &toDelete);
    RC FindLeaf(PageNum rootPage, void *pData, const RID &rid, PageNum &result);
     */
    bool IsEqualMBR(struct MBR m1, struct MBR m2);
    bool Overlap(struct MBR rt1, struct MBR rt2);
	RC DeleteFromBucket(struct IX_BucketHeader *bHeader, const RID &rid, bool &deletePage, RID &lastRID, PageNum &nextPage);
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
	bool isOpen;
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
