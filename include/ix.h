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

struct IX_NodeHeader{
	bool isLeaf;
	bool isEmpty;
	int num_entries; //the number of nonempty entries

	int firstSlot; //the pointer to the first slot, -1 if none
	int freeSlot; //the pointer to the first free slot

	PageNum parentPage; //convenient for adjusting tree, adjust when parent need splitting
	int myindex; //index in parent entry, assigned when parent add its entry
	//MBR mbr;
	//PageNum prevPage;
};

struct IX_BucketHeader{
	int num_entries;
	
	int firstSlot;
	int freeSlot;
	PageNum nextBucket;
};

struct IX_NodeEntry{
	int status; //if the entry contains duplicate values(1) or
	            //empty(-1) or
                //singe value(0)	
	int nextSlot; //the pointer to the next slot (in the node),-1 if reach the end, 0 if not assigned
	PageNum page; //page number it points to (next level)
	
	SlotNum slot; //slot number, not needed for internal node
    MBR mbr;
};

struct IX_BucketEntry{
	int nextSlot;
	PageNum page;
	SlotNum slot;
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
private:
	PF_FileHandle pfh;
	struct IX_IndexHeader header;
	PF_PageHandle rootPH;
	//int (*comparator) (void*, void*, int);
	bool dirtyHeader;
	
	//some helper methods
	RC ChooseLeaf(struct IX_NodeHeader *rHeader, void *pData, struct IX_NodeHeader &result);
    RC CreateNode(PF_PageHandle &ph, PageNum &page, char *nData, bool isLeaf, PageNum parent);
	RC InsertToBucket(PageNum bucketPage, const RID &rid);
	RC CreateBucket(PageNum &page);

	float getArea(void *pData);
	float getExpansion(void *pData1, void *pData2);

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
private:
	PF_Manager &pfm;
	RC initIxHandle(IX_IndexHanle &ih, PF_FileHandle &fh, struct IX_IndexHeader* header);
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
