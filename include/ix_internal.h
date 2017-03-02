#ifndef IX_INTERNAL_H
#define IX_INTERNAL_H

#include "ix.h"

enum EntryStatus{SINGLE, DUPLICATE, EMPTY};

struct IX_NodeHeader{
	bool isLeafNode;'
	bool isEmpty;
	int num_entries; //the number of nonempty entries

	int headSlot; //the pointer to the first slot
	int freeSlot; //the pointer to the first free slot

	PageNum pn;
};

struct IX_Entry{
	EntryStatus entry_stat; //if the entry contains duplicate values or empty
	int nextSlot; //the pointer to the next slot (in the same node)
	PageNum page; //page number it resides in
	SlotNum slot; //slot number

};

//bool rectOverlap(void **point1, void **point2, int dim, AttrType

#endif
