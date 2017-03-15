#ifndef IX_INTERNAL_H
#define IX_INTERNAL_H

#include "ix.h"

struct IX_NodeHeader{
	bool isLeaf;
	bool isEmpty;
	int num_entries; //the number of nonempty entries

	int firstSlot; //the pointer to the first slot, -1 if none
	int freeSlot; //the pointer to the first free slot

	PageNum childPage; //not needed in leaf node
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
	int nextSlot; //the pointer to the next slot (in the node), -1 if none
	PageNum page; //page number it points to (next level)
	SlotNum slot; //slot number, not needed for internal node

};

struct IX_BucketEntry{
	int nextSlot;
	PageNum page;
	SlotNum slot;
};

#endif
