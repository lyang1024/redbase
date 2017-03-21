//
// File:        ix_indexscan.cc
// Description: IX_IndexHandle handles scanning through the index for a 
//              certain value.
// Author:      <Your name here>
//

#include <unistd.h>
#include <sys/types.h>
#include "pf.h"
#include "ix.h"
#include <iostream>
#include "ix_internal.h"
#include <cstdio>

bool ioverlap(void * value1, void * value2, AttrType attrtype, int attrLength){
    if(attrtype == MBR){
        struct MBR m1 = *(struct MBR*)value1;
        struct MBR m2 = *(struct MBR*)value2;
        if (m1.x1 > m2.x2 || m2.x1 > m1.x2) return 0;
        if (m1.y2 < m2.y1 || m2.y2 < m1.y1) return 0;
        return 1;
    }
    else{
        printf("wrong type!\n");
    }
}

IX_IndexScan::IX_IndexScan()
{
  // Implement this
    openScan = false;  // Initialize all values
    value = NULL;
    //initializedValue = false;
    //hasBucketPinned = false;
    //hasLeafPinned = false;
    scanEnded = true;
    scanStarted = false;

    //foundFirstValue = false;
    //foundLastValue = false;
    //useFirstLeaf = false;
}

IX_IndexScan::~IX_IndexScan()
{
  // Implement this
    /*
    if(scanEnded == false && hasBucketPinned == true)
        indexHandle->pfh.UnpinPage(currBucketNum);
    if(scanEnded == false && hasLeafPinned == true &&(currLeafNum != (indexHandle->header).rootPage))
        indexHandle->pfh.UnpinPage(currLeafNum);
    if(initializedValue == true){
        free(value);
        initializedValue = false;
    }
     */
    //openScan = false;
    //scanEnded = true;
    //scanStarted = false;
    //indices.clear();
}

RC IX_IndexScan::OpenScan(const IX_IndexHandle &indexHandle,
                CompOp compOp,
                void *value,
                ClientHint  pinHint)
{
  // Implement this
    RC rc = 0;

    if(openScan == true || compOp != OVERLAP_OP) // makes sure that the scan is not already open
        return (IX_INVALIDSCAN);              // and disallows NE_OP comparator
/*
    if(indexHandle.isValidIndexHeader()) // makes sure that the indexHanlde is valid
        this->indexHandle = const_cast<IX_IndexHandle*>(&indexHandle);
    else
        return (IX_INVALIDSCAN);
*/
    //this->value = NULL;
    //useFirstLeaf = true;
    this->compOp = compOp; // sets up the comparator values, and the comparator
    comparator = &ioverlap;

    // sets up attribute length and type
    this->attrType = (indexHandle.header).attr_type;
    attrLength = ((this->indexHandle)->header).attr_length;
    if(compOp != NO_OP){
        this->value = (void *) malloc(attrLength); // sets up the value
        memcpy(this->value, value, attrLength);
        //initializedValue = true;
    }

    openScan = true; // sets up all indicators
    scanEnded = false;
    //hasLeafPinned = false;
    scanStarted = false;
    FindFirstLeaf();
    //endOfIndexReached = false;
    //foundFirstValue = false;
    //foundLastValue = false;

    return (rc);
}

RC IX_IndexScan::GetNextEntry(RID &rid)
{
  // Implemenet this
    RC rc = 0;
    if(path.empty()){
        return rc;
    }
    PageNum currentPN;
    struct IX_NodeHeader *currentNH;
    PF_PageHandle currentPH;

    //currentNH = path.top();
    while(!path.empty()){
        currentPN= path.top();
        if ((rc = indexHandle->pfh.GetThisPage(currentPN, currentPH)) || (rc = currentPH.GetData((char *&) currentNH)))
            return rc;
        //currentNH = path.top();
        int result;
        FindNextEntry(currentNH, indices.back(), result);
        if(result < 0){
            path.pop();
            indices.pop_back();
            if(path.empty()){
                return (IX_EOF);
            }
        }
        else{
            indices[indices.size()-1] = result + 1;
            struct IX_NodeEntry *entries = (struct IX_NodeEntry*)((char*)currentNH + indexHandle->header.entryOffset_N);
            PageNum nextPN = entries[result].page;
            if(currentNH->isLeaf){
                RID temprid(nextPN,entries[result].slot);
                rid = temprid;
                indexHandle->pfh.UnpinPage(nextPN);
                return rc;
            }
            else {
                /*
                if ((rc = indexHandle->pfh.GetThisPage(nextPN, currentPH)) ||
                    (rc = currentPH.GetData((char *&) currentNH)))
                    return rc;
                    */
                path.push(nextPN);
                indices.push_back(0);
            }
            indexHandle->pfh.UnpinPage(nextPN);
        }
        indexHandle->pfh.UnpinPage(currentPN);
    }
    return rc;
}

RC IX_IndexScan::FindFirstLeaf(){
    RC rc = 0;
    PF_PageHandle rPH = indexHandle->rootPH;
    struct IX_NodeHeader *currentNH;
    if(rc = rPH.GetData((char *&)currentNH))
        return rc;
    PageNum rPN;
    PageNum nextPN;
    indexHandle->rootPH.GetPageNum(rPN);
    path.push(rPN);
    indices.push_back(0);
    while(!currentNH->isLeaf){
        int firstEntry;
        FindNextEntry(currentNH,indices.back(),firstEntry);
        if(firstEntry < 0){

            path.pop();
            indices.pop_back();
            if(path.empty()){
                indexHandle->pfh.UnpinPage(rPN);
                return (IX_EOF);
            }
            indexHandle->pfh.UnpinPage(rPN);
            rPN = path.top();
            PF_PageHandle currentHandle;
            if((rc = indexHandle->pfh.GetThisPage(rPN, currentHandle)) || (rc = currentHandle.GetData((char *&)currentNH))){
                indexHandle->pfh.UnpinPage(rPN);
                return rc;
            }

        }
        else{
            indices[indices.size()-1] = firstEntry + 1;
            struct IX_NodeEntry *entries = (struct IX_NodeEntry*)((char*)currentNH + indexHandle->header.entryOffset_N);
            indexHandle->pfh.UnpinPage(nextPN);
            nextPN = entries[firstEntry].page;

            PF_PageHandle currentHandle;
            if((rc = indexHandle->pfh.GetThisPage(nextPN, currentHandle)) || (rc = currentHandle.GetData((char *&)currentNH))){
                indexHandle->pfh.UnpinPage(nextPN);
                return rc;
            }
            path.push(nextPN);
            indices.push_back(0);
        }
    }
    indexHandle->pfh.UnpinPage(nextPN);
    indexHandle->pfh.UnpinPage(rPN);
    std::cout<<"scan is open"<<"\n";
    return rc;
}

RC IX_IndexScan::FindNextEntry(struct IX_NodeHeader *nh, int current, int &result){
    RC rc = 0;
    struct IX_NodeEntry *entries = (struct IX_NodeEntry*)((char*)nh + indexHandle->header.entryOffset_N);
    while(current < nh->num_entries){
        if(comparator((void*)&entries[current].mbr,value,attrType,attrLength)){
            break;
        }
        current++;
    }
    if(current == nh->num_entries){
        result = -1;
    }
    else{
        result = current;
    }
    return rc;
}

RC IX_IndexScan::CloseScan()
{
  // Implement this
    RC rc = 0;
    if(openScan == false)
        return (IX_INVALIDSCAN);
    //if(scanEnded == false && hasBucketPinned == true)
        //indexHandle->pfh.UnpinPage(currBucketNum);
    //if(scanEnded == false && hasLeafPinned == true && (currLeafNum != (indexHandle->header).rootPage))
        indexHandle->pfh.UnpinPage(currLeafNum);
    //if(initializedValue == true){
        free(value);
        //initializedValue = false;
    //}
    openScan = false;
    scanStarted = false;
    indices.clear();
    std::stack<PageNum>().swap(path);
    return (rc);
}



