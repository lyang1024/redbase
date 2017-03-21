#ifndef IX_INTERNAL_H
#define IX_INTERNAL_H

#include "ix.h"



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
	int nextSlot; //the pointer to the next slot (in the node),-1 if reach the end
	PageNum page; //Record page number it points to
    //PageNum childpage;
	
	SlotNum slot; //slot number, not needed for internal node
    //just set it to -1 for internal node
    struct MBR mbr;
};

struct IX_BucketEntry{
	int nextSlot;
	PageNum page;
	SlotNum slot;
};

//bool rectOverlap(void **point1, void **point2, int dim, AttrType

#endif
