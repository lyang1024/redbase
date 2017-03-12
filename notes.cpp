typedef int PageNum

void* //when there are different pointer types ;in the same code

class RID {
  static const PageNum INVALID_PAGE = -1;
  static const SlotNum INVALID_SLOT = -1;
public:
    RID();                                         // Default constructor
    RID(PageNum pageNum, SlotNum slotNum);
    ~RID();                                        // Destructor
    RID& operator= (const RID &rid);               // Copies RID
    bool operator== (const RID &rid) const;

    RC GetPageNum(PageNum &pageNum) const;         // Return page number
    RC GetSlotNum(SlotNum &slotNum) const;         // Return slot number

    RC isValidRID() const; // checks if it is a valid RID

private:
  PageNum page;
  SlotNum slot;
};

//********************in the ix file***********************


//indexhandle: algorithms
//bucket is used to store duplicate values
//need to consider duplicate records in one index (leaf) entry
//the dimension of attribute -- inside IX_IndexHeader class
//pin page???


//*********************my design*************************


//struct Rect in parser.h --> extra MBR.h
//function define in parse.cc: << for value, CompOp(create "OVERLAP"), AttrInfo, AttrrType, Rect
//add comparator for type point in file comparators.h, e.g. overlap
//probably: redbase.h, AttrType, CompOp

//
//in parse.y
//value, add rect type. Add operator << for rect, add in attrtype op?
//NO NEED TO MODIFY parse.cc !!!

//in nodes.cc
//--in value_node, add support for mbr
//--in 

// in ql_node.cc
//add mbr

//in node_comps.h
//add overlap logic

//in interp.cc
//--in mk_value, add mbr
/*
//ix_internal.h

//!!!definitions of different nodes are in nodes.cc
//while declarations are in parser_internal.h

//input format: {[3.4, 4.5],[2.3, 3.1]}
// in scan.l support for mbr input format
// in scanhelper.c parse input string into mbr object
// the overlap key word in input should change to _overlap??

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
    EntryStatus entry_stat; //if the entry contains duplicate values
    int nextSlot; //the pointer to the next slot (in the same node)
    PageNum page; //page number it resides in
    SlotNum slot; //slot number
};

#endif

//end ix_internal.h
*/


//functions may need to be modified



where is print_int etc??? -- used in ql_node.cc

In sm_manager:
recInsert_MBR


