//
// File:        ix_indexhandle.cc
// Description: IX_IndexHandle handles manipulations within the index
// Author:      <Your Name Here>
//

#include <unistd.h>
#include <sys/types.h>
#include "ix.h"
#include "ix_internal.h"
#include "pf.h"
#include "mbr.h"
#include "comparators.h"
#include <cstdio>
#include <vector>
#include <string>
//#include <math.h>


//public interface
IX_IndexHandle::IX_IndexHandle()
{
  // Implement this
}

IX_IndexHandle::~IX_IndexHandle()
{
  // Implement this
}

RC IX_IndexHandle::InsertEntry(void *pData, const RID &rid)
{
  // Implement this
	RC rc = 0;
	struct IX_NodeHeader *rootheader;
	if((rc = rootPH.GetData((char *&)rootheader)))
		return rc;
	struct IX_NodeHeader *chosenleaf;
    PageNum leafPN;
    PageNum rootPN = header.rootPage;
	if((rc = ChooseLeaf(rootPN, pData, leafPN)))
		return rc;
    PF_PageHandle leafPH;
	if((rc = pfh.GetThisPage(leafPN, leafPH)) || (rc = leafPH.GetData((char *&)chosenleaf)))
		return rc;
	//check if there are duplicate mbr in the leaf node
	struct IX_NodeEntry *entries = (struct IX_NodeEntry*)((char*)chosenleaf + header.entryOffset_N);
	int index = chosenleaf->firstSlot;
	while(index != -1){
		char *value = (char*)&entries[index].mbr;
		float tempExpansion = getExpansion((void*)value, pData);
		if(tempExpansion == 0){
			break;
		}
		index = entries[index].nextSlot;
	}
	//if there are duplicates
    header_modified = true;
	if(index != -1){
		struct IX_NodeEntry dupEntry = entries[index];
		//if it already contain duplicate value
		PageNum bucketPage;
		//already a bucket
		if(dupEntry.status == 1){
			bucketPage = dupEntry.page;
			if((rc = InsertToBucket(bucketPage, rid)))
				return rc;
            if((rc = pfh.MarkDirty(bucketPage)) || (rc = pfh.UnpinPage(bucketPage)))
                return (rc);
		}
		else {
			//create a bucket
			if((rc = CreateBucket(bucketPage)))
				return rc;
			entries[index].status = 1;
			RID newrid(entries[index].page, entries[index].slot);
			if((rc = InsertToBucket(bucketPage, newrid)) || (rc = InsertToBucket(bucketPage, rid)))
				return rc;
			entries[index].page = bucketPage;
            if((rc = pfh.MarkDirty(bucketPage)) || (rc = pfh.UnpinPage(bucketPage)))
                return (rc);
            //if((rc = pfh.MarkDirty(leafPN)) || (rc = pfh.UnpinPage(leafPN)))
            //    return (rc);
		}
	}
	else{
	//no duplicate, insert to node directly
		if(chosenleaf->num_entries == header.M){
		//unfortunately, node is full
			//PageNum newPage;
            struct IX_NodeHeader *newHeader;
            struct IX_NodeEntry newentry;
            newentry.status = 0;
            if((rc = rid.GetPageNum(newentry.page)) || (rc = rid.GetSlotNum(newentry.slot))){
                return rc;
            }
            newentry.mbr = *(struct MBR*)pData;
            PageNum newPageNum;
            SplitNode(chosenleaf,newHeader,newentry,newPageNum);
            AdjustTree(leafPN, newPageNum);
            if((rc = pfh.MarkDirty(newPageNum)) || (rc = pfh.UnpinPage(newPageNum)))
                return (rc);
            //if((rc = pfh.MarkDirty(leafPN)) || (rc = pfh.UnpinPage(leafPN)))
            //    return (rc);
			//PageNum parentp = choosenleaf->parentPage;
			//int newindex = entries[index].nextSlot;
			//if((rc = CreateNode(newPH, newPage, newData, true, parentp)))
        }
        else{
        //not full
            int newindex = chosenleaf->freeSlot;
            entries[newindex].status = 0;
            if((rc = rid.GetPageNum(entries[newindex].page)) || (rc = rid.GetSlotNum(entries[newindex].slot))){
                return rc;
            }
            chosenleaf->isEmpty = false;
            chosenleaf->num_entries++;
            chosenleaf->freeSlot = entries[newindex].nextSlot;
            int previndex;
            FindPrevIndex(chosenleaf, newindex, previndex);
            if(previndex < 0){
                chosenleaf->firstSlot = newindex;
            }
            entries[newindex].mbr = *(struct MBR *)pData;
            //need adjust tree to update parent mbr
            AdjustTree(chosenleaf,entries[newindex].mbr);
        }
    }
    if((rc = pfh.MarkDirty(leafPN)) || (rc = pfh.UnpinPage(leafPN)))
        return (rc);
    if((rc = pfh.MarkDirty(header.rootPage)))
        return (rc);
    return rc;
		
}

RC IX_IndexHandle::DeleteEntry(void *pData, const RID &rid)
{
  // Implement this
 /*
    PageNum rootP = header.rootPage;
    PageNum leafP;
    RC rc = 0;
    rc = FindLeaf(rootP, pData, rid, leafP);
    if(rc) return rc;
*/

}

RC IX_IndexHandle::ForcePages()
{
  // Implement this
  RC rc = 0;
  pfh.ForcePages();
  return rc;
}

//private helpers
//for deletion
/*
RC IX_IndexHandle::DeleteFromNode(PageNum nodePage, void* pData, const RID &rid, bool &toDelete){
    PF_PageHandle nodePH;
    struct IX_NodeHeader *nodeNH;
    RC rc = 0;
    PageNum recordPN;
    SlotNum recordSN;
    rid.GetPageNum(recordPN);
    rid.GetSlotNum(recordSN);
    struct MBR tempmbr = *(struct MBR*)pData;
    if((rc = pfh.GetThisPage(nodePage, nodePH)) || (rc = nodePH.GetData((char*&)nodeNH)))
        return rc;
    struct IX_NodeEntry *entries = (struct IX_NodeEntry*)((char*&)nodeNH + header.entryOffset_N);
    int i;
    for(i = 0; i < nodeNH->num_entries; i++){
        if(IsEqualMBR(tempmbr,entries[i].mbr) && entries[i].page == recordPN){
            break;
        }
    }
    if(i == nodeNH->num_entries) return 1;
    int prevIndex;
    if(i == nodeNH->firstSlot){
        prevIndex = i;
    }
    else {
        prevIndex = i - 1;
    }
    if(entries[i].status == 0){
        if(recordPN != entries[i].page || recordSN != entries[i].slot){
            return IX_INVALIDENTRY;
        }
        if(i == nodeNH->firstSlot){
            nodeNH->firstSlot = entries[i].nextSlot;
        }
        else{
            entries[prevIndex].nextSlot = entries[i].nextSlot;
        }
        entries[i].nextSlot = nodeNH->freeSlot;
        nodeNH->freeSlot = i;
        entries[i].status = -1;
        nodeNH->num_entries--;

    }
    else if(entries[i].status == 1){
        PageNum bucketNum = entries[i].page;
        PF_PageHandle bucketPH;
        struct IX_BucketHeader *bHeader;
        bool deletePage = false;
        RID lastRID;
        PageNum nextBucketNum; // Retrieve the bucket header
        if((rc = pfh.GetThisPage(bucketNum, bucketPH)) || (rc = bucketPH.GetData((char *&)bHeader))){
            return (rc);
        }
        RC rc2 = 0;
        if((rc2 = pfh.MarkDirty(bucketNum)) || (rc = pfh.UnpinPage(bucketNum)))
            return (rc2);

        if(rc == IX_INVALIDENTRY) // If it doesnt exist in the bucket, notify the function caller
            return (IX_INVALIDENTRY);

        if(deletePage){ // If we need to delete the bucket,
            if((rc = pfh.DisposePage(bucketNum) ))
                return (rc);
            // If there are no more buckets, then place the last RID into the leaf page, and
            // update the isValid flag to OCCUPIED_NEW
            if(nextBucketNum == -1){
                entries[i].status = 0;
                if((rc = lastRID.GetPageNum(entries[i].page)) ||
                   (rc = lastRID.GetSlotNum(entries[i].slot)))
                    return (rc);
            }
            else // otherwise, set the bucketPointer to the next bucket
                entries[i].page = nextBucketNum;
        }
    }
    if(nodeNH->num_entries == 0){ // If the leaf is now empty,
        toDelete = true;          // return the indicator to delete
        // Update the leaf pointers of its previous and next neighbors
    }
    return 0;
}
*/
/*
RC IX_IndexHandle::DeleteFromBucket(struct IX_BucketHeader *bHeader, const RID &rid,
                                    bool &deletePage, RID &lastRID, PageNum &nextPage){
    RC rc = 0;
    PageNum nextPageNum = bHeader->nextBucket;
    nextPage = bHeader->nextBucket; // set the nextBucket pointer

    struct Bucket_Entry *entries = (struct Bucket_Entry *)((char *)bHeader + header.entryOffset_B);

    if((nextPageNum != -1)){ // If there's a bucket after this one, search in it first
        bool toDelete = false; // whether to delete the following bucket
        PF_PageHandle nextBucketPH;
        struct IX_BucketHeader *nextHeader;
        RID last;
        PageNum nextNextPage; // Get this next bucket
        if((rc = pfh.GetThisPage(nextPageNum, nextBucketPH)) || (rc = nextBucketPH.GetData((char *&)nextHeader)))
            return (rc);
        // Recursively call go delete from this bucket
        rc = DeleteFromBucket(nextHeader, rid, toDelete, last, nextNextPage);

        int numKeysInNext = nextHeader->num_entries;
        RC rc2 = 0;
        if((rc2 = pfh.MarkDirty(nextPageNum)) || (rc2 = pfh.UnpinPage(nextPageNum)))
            return (rc2);

        // If the following bucket only has one key remaining, and there is space in this
        // bucket, then put the lastRID into this bucket, and delete the following bucket
        if(toDelete && bHeader->num_entries < header.Mb && numKeysInNext == 1){
            int loc = bHeader->freeSlot;
            if((rc2 = last.GetPageNum(entries[loc].page)) || (rc2 = last.GetSlotNum(entries[loc].slot)))
                return (rc2);

            bHeader->freeSlot = entries[loc].nextSlot;
            entries[loc].nextSlot = bHeader->firstSlot;
            bHeader->firstSlot = loc;

            bHeader->num_entries++;
            numKeysInNext = 0;
        }
        if(toDelete && numKeysInNext == 0){
            if((rc2 = pfh.DisposePage(nextPageNum)))
                return (rc2);
            bHeader->nextBucket = nextNextPage; // make this bucket point to the bucket
            // the deleted bucket pointed to
        }

        if(rc == 0) // if we found the value, return
            return (0);
    }

    // Otherwise, search in this bucket
    PageNum ridPage;
    SlotNum ridSlot;
    if((rc = rid.GetPageNum(ridPage))|| (rc = rid.GetSlotNum(ridSlot)))
        return (rc);

    // Search through entire values
    int prevIndex = -1;
    int currIndex = bHeader->firstSlot;
    bool found = false;
    while(currIndex != -1){
        if(entries[currIndex].page == ridPage && entries[currIndex].slot == ridSlot){
            found = true;
            break;
        }
        prevIndex = currIndex;
        currIndex = entries[prevIndex].nextSlot;
    }

    // if found, delete from it
    if(found){
        if (prevIndex == -1)
            bHeader->firstSlot = entries[currIndex].nextSlot;
        else
            entries[prevIndex].nextSlot = entries[currIndex].nextSlot;
        entries[currIndex].nextSlot = bHeader->freeSlot;
        bHeader->freeSlot = currIndex;

        bHeader->num_entries--;
        // If there is one or less keys in this bucket, mark it for deletion.
        if(bHeader->num_entries == 1 || bHeader->num_entries == 0){
            int firstSlot = bHeader->firstSlot;
            RID last(entries[firstSlot].page, entries[firstSlot].slot);
            lastRID = last; // Return the last RID to move to the previous bucket
            deletePage = true;
        }

        return (0);

    }
    return (IX_INVALIDENTRY); // if not found, return IX_INVALIDENTRY

}
*/
/*
RC IX_IndexHandle::FindLeaf(PageNum rootPage, void* pData, const RID &rid, PageNum &result){
    PF_PageHandle rootPH;
    struct IX_NodeHeader *rootNH;
    RC rc = 0;
    PageNum recordPN;
    rid.GetPageNum(recordPN);
    struct MBR tempmbr = *(struct MBR*)pData;
    if((rc = pfh.GetThisPage(rootPage, rootPH)) || (rc = rootPH.GetData((char*&)rootNH)))
        return rc;
    struct IX_NodeEntry *entries = (struct IX_NodeEntry*)((char*&)rootNH + header.entryOffset_N);
    if(rootNH->isLeaf){
        for(int i = 0; i < rootNH->num_entries; i++){
            if(IsEqualMBR(tempmbr,entries[i].mbr) && entries[i].page == recordPN){
                result = rootPage;
                return rc;
            }
        }
        rc = 1; //not found
        return rc;
    }
    else{
        for(int i = 0; i < rootNH->num_entries; i++){
            if(Overlap(tempmbr,entries[i].mbr)){
                rc = FindLeaf(entries[i].page,pData,rid,result);
                if(rc == 0){
                    return rc;
                }
            }
        }
        rc = 1;
        return rc;
    }

}
 */
//for insertion
RC IX_IndexHandle::CreateNode(PF_PageHandle &ph, PageNum &page, char *nData, bool isLeaf, PageNum parent, int myindex){
    RC rc = 0;
    if((rc = pfh.AllocatePage(ph)) || (rc = ph.GetPageNum(page)))
  	    return rc;
	if((rc = ph.GetData(nData)))
		return rc;
	struct IX_NodeHeader *myheader = (struct IX_NodeHeader *)nData;

	myheader->isLeaf = isLeaf;
	myheader->isEmpty = true;

	myheader->num_entries = 0;
	myheader->parentPage = parent;
	myheader->firstSlot = -1;
	myheader->freeSlot = 0;
	myheader->myindex = myindex;

	struct IX_NodeEntry *entries = (struct IX_NodeEntry *)((char*)myheader + header.entryOffset_N);
	int i;
	for(i = 0; i < header.M - 1; i++){
		entries[i].status = -1;
		entries[i].page = -1;
		entries[i].nextSlot = i + 1;
	}
	entries[i].nextSlot = -1;

	return rc;
}

RC IX_IndexHandle::FindPrevIndex(struct IX_NodeHeader *nHeader, int thisindex, int &previndex){
    struct IX_NodeEntry *entries = (struct IX_NodeEntry*)((char*)nHeader + header.entryOffset_N);
    int prev_idx;
    int cur_idx = nHeader->firstSlot;
    while(cur_idx != thisindex){
        prev_idx = cur_idx;
        cur_idx = entries[prev_idx].nextSlot;
    }
    previndex = prev_idx;
    return 0;
}

RC IX_IndexHandle::ChooseLeaf(PageNum rPN, void *pData, PageNum &result){
	RC rc = 0;
    IX_NodeHeader *rHeader;
    PF_PageHandle rHandle;
    if((rc = pfh.GetThisPage(rPN, rHandle)) || (rc = rHandle.GetData((char *&)rHeader)))
        return rc;
    if(rHeader->isLeaf){
        result = rPN;
        return rc;
    }
	struct IX_NodeHeader *current_h;
	current_h = rHeader;
    PageNum nextPageNum;

	while(!current_h->isLeaf){
		struct IX_NodeEntry *entries = (struct IX_NodeEntry*)((char*)current_h + header.entryOffset_N);
		int index = current_h->firstSlot;
        if(index == -1){

        }
		int bestindex = index;
		float minExpansion;
		char *value = (char*)&entries[index].mbr;
		minExpansion = getExpansion((void*)value, pData);
		index++;
		while(index != -1){
			char *value = (char*)&entries[index].mbr;
			int tempExpansion = getExpansion((void*)value, pData);
			if(tempExpansion <= minExpansion){
				if((tempExpansion == minExpansion && getArea((void*)value) < getArea((void*)&entries[bestindex])) || tempExpansion < minExpansion){
					bestindex = index;
					minExpansion = tempExpansion;
				}
			}
			index++;
		}
		nextPageNum = entries[bestindex].page;
		PF_PageHandle nextPH;
		if((rc = pfh.GetThisPage(nextPageNum, nextPH)) || (rc = nextPH.GetData((char *&)current_h)))
			return rc;
		
	}
    if((rc = pfh.MarkDirty(nextPageNum)) || (rc = pfh.UnpinPage(nextPageNum)))
        return (rc);
	result = nextPageNum;
    return rc;
}

RC IX_IndexHandle::CreateBucket(PageNum &page){
	char *nData;
	PF_PageHandle ph;
	RC rc = 0;
	if((rc = pfh.AllocatePage(ph)) || (rc = ph.GetPageNum(page)))
		return rc;
	if((rc = ph.GetData(nData))){
		RC flag;
		if((flag = pfh.UnpinPage(page)))
			return flag;
		return flag;
	}

	struct IX_BucketHeader *bHeader = (struct IX_BucketHeader*) nData;

	bHeader->num_entries = 0;
	bHeader->firstSlot = -1;
	bHeader->freeSlot = 0;
	bHeader->nextBucket = -1;

	struct IX_BucketEntry *entries = (struct IX_BucketEntry*)((char*)bHeader + header.entryOffset_B);
	int i = 0;
	for(;i < header.Mb - 1; i++){
		entries[i].nextSlot = i + 1;
	}
	entries[i].nextSlot = -1;
	if((rc = pfh.MarkDirty(page)) || (rc = pfh.UnpinPage(page)))
		return rc;
	return rc;
}

RC IX_IndexHandle::InsertToBucket(PageNum bucketPage, const RID &rid){
	RC rc = 0;
	PageNum recordPage;
	SlotNum recordSlot;
	if((rc = rid.GetPageNum(recordPage)) || (rc = rid.GetSlotNum(recordSlot)))
		return rc;
	
	PageNum curBucketPage = bucketPage;
	PF_PageHandle bucketPH;
	struct IX_BucketHeader *bucketHeader;
	bool done = false;
	while(!done){
		if((rc = pfh.GetThisPage(curBucketPage, bucketPH)) || (rc = bucketPH.GetData((char *&)bucketHeader)))
			return rc;
		//could check if duplicate record is entered, not necessary
		//struct IX_BucketEntry *entries = (struct IX_BucketEntry*)((char*)bucketHeader + header.entryOffset_B);
		if(bucketHeader->nextBucket == -1 && bucketHeader->num_entries == header.Mb){
			done = true;
			PageNum newBucketPage;
			PF_PageHandle newBucketPH;
			if((rc = CreateBucket(newBucketPage)))
				return rc;
			bucketHeader->nextBucket = newBucketPage;
			if((rc = pfh.MarkDirty(curBucketPage)) || (rc = pfh.UnpinPage(curBucketPage)))
				return rc;
			curBucketPage = newBucketPage;
			if((rc = pfh.GetThisPage(curBucketPage, bucketPH)) || (rc = bucketPH.GetData((char*&)bucketHeader)))
				return rc;
		}
		if(bucketHeader->nextBucket == -1){
			done = true;
			struct IX_BucketEntry *entries = (struct IX_BucketEntry*)((char*)bucketHeader + header.entryOffset_B);
			int insert_index = bucketHeader->freeSlot;
			entries[insert_index].slot = recordSlot;
			entries[insert_index].page = recordPage;
			bucketHeader->freeSlot = entries[insert_index].nextSlot;
			entries[insert_index].nextSlot = bucketHeader->firstSlot;
			bucketHeader->firstSlot = insert_index;
			bucketHeader->num_entries++;
		}

		PageNum nextPage = bucketHeader->nextBucket;
		if((rc = pfh.MarkDirty(curBucketPage)) || (rc = pfh.UnpinPage(curBucketPage)))
			return rc;
		curBucketPage = nextPage;
	}
	return rc;
}


float IX_IndexHandle::getArea(void *pData){
	struct MBR mbr = *(struct MBR*)pData;
	return (mbr.x2-mbr.x1)*(mbr.y2-mbr.y1);
}

float IX_IndexHandle::getExpansion(void *pData1, void *pData2){
	struct MBR m1 = *(struct MBR*)pData1;
	struct MBR m2 = *(struct MBR*)pData2;

	float newx2 = m1.x2 > m2.x2 ? m1.x2 : m2.x2;
    float newx1 = m1.x1 < m2.x1 ? m1.x1 : m2.x1;
    float newy2 = m1.y2 > m2.y2 ? m1.y2 : m2.y2;
    float newy1 = m1.y1 < m2.y1 ? m1.y1 : m2.y1;
	float new_area = (newx2 - newx1) * (newy2 - newy1);
	float old_area = getArea((void*)&m1);
	return new_area - old_area;
}

void IX_IndexHandle::adjustMBR(struct MBR &m1, struct MBR m2){
    m1.x1 = m1.x1 <= m2.x1 ? m1.x1 : m2.x1;
    m1.y1 = m1.y1 <= m2.y1 ? m1.y1 : m2.y1;
    m1.x2 = m1.x2 >= m2.x2 ? m1.x2 : m2.x2;
    m1.y2 = m1.y2 >= m2.y2 ? m1.y2 : m2.y2;
}

RC IX_IndexHandle::SplitNode(struct IX_NodeHeader *h1, struct IX_NodeHeader *newHeader, struct IX_NodeEntry newentry, PageNum &newPageNum){
    void* pData = (void*)&newentry.mbr;
    struct IX_NodeEntry *entries = (struct IX_NodeEntry*)((char*)h1 + header.entryOffset_N);
    int seed1, seed2;
    std::vector<int> g1;
    std::vector<int> g2;
    struct MBR m1;
    struct MBR m2;
    std::vector<bool> table(h1->num_entries + 1, false);
    int total = table.size();
    PickSeeds(entries, h1->num_entries, pData, seed1, seed2);
    g1.push_back(seed1);
    g2.push_back(seed2);
    if(seed1 == h1->num_entries){
        m1 = *(struct MBR*)pData;
        m2 = entries[seed2].mbr;
    }
    else if(seed2 == h1->num_entries){
        m2 = *(struct MBR*)pData;
        m1 = entries[seed1].mbr;
    }
    else{
        m1 = entries[seed1].mbr;
        m2 = entries[seed2].mbr;
    }
    table[seed1] = true;
    table[seed2] = true;
    int count = 2;
    while(count < total){
        if(g1.size() + total - count == header.m){
            for(int j = 0; j < total; j++){
                if(!table[j]){
                    g1.push_back(j);
                    count++;
                }

            }
            break;
        }
        if(g2.size() + total - count == header.m){
            for(int j = 0; j < total; j++){
                if(!table[j]){
                    g2.push_back(j);
                    count++;
                }
            }
            break;
        }
        int nextresult = PickNext(entries, table, pData, m1, m2);
        float exp1, exp2;
        struct MBR nextmbr;
        if(nextresult == total - 1){
            nextmbr = *(struct MBR*)pData;
        }
        else{
            nextmbr = entries[nextresult].mbr;
        }
        exp1 = getExpansion((void*)&m1, (void*)&nextmbr);
        exp2 = getExpansion((void*)&m2, (void*)&nextmbr);
        if(exp1 == exp2){
            float area1 = getArea((void*)&m1);
            float area2 = getArea((void*)&m2);
            if(area1 == area2){
                if(g1.size() <= g2.size()){
                    g1.push_back(nextresult);
                    adjustMBR(m1, nextmbr);
                }
                else{
                    g2.push_back(nextresult);
                    adjustMBR(m2, nextmbr);
                }
            }
            else if(area1 < area2){
                g1.push_back(nextresult);
                adjustMBR(m1, nextmbr);
            }
            else{
                g2.push_back(nextresult);
                adjustMBR(m2, nextmbr);
            }
        }
        else if(exp1 < exp2){
            g1.push_back(nextresult);
            adjustMBR(m1, nextmbr);
        }
        else{
            g2.push_back(nextresult);
            adjustMBR(m2, nextmbr);
        }
        count++;
    }
    //done splitting to two groups, prepare return
    //create node, deploy two sets of entries to two nodes
    PageNum newPage;
    //struct IX_NodeHeader *newHeader;
    PF_PageHandle newPH;
    bool isLeaf = false;
    if(h1->isLeaf) isLeaf = true;
    //create this new node, set its index to -1 for now
    //update in adjust tree
    RC rc = 0;
    if((rc = CreateNode(newPH, newPage, (char*&)newHeader, isLeaf, h1->parentPage, -1))){
        if((rc = pfh.MarkDirty(newPage)) || (rc = pfh.UnpinPage(newPage)))
            return (rc);

        return rc;
    }
    newPageNum = newPage;
    struct IX_NodeEntry *oldentries = (struct IX_NodeEntry*)malloc(h1->num_entries*sizeof(struct IX_NodeEntry));
    for(int k = 0; k < h1->num_entries; k++){
        oldentries[k] = entries[k];
    }
    struct IX_NodeEntry *newEntries = (struct IX_NodeEntry *)((char*)newHeader + header.entryOffset_N);
    newHeader->isEmpty = false;
    newHeader->num_entries = g2.size();
    newHeader->firstSlot = 0;
    newHeader->freeSlot = g2.size();
    newHeader->parentPage = h1->parentPage;
    h1->num_entries = g1.size();
    h1->firstSlot = 0;
    h1->freeSlot = g1.size();
    int k;
    for(k = 0; k < header.M - 1; k++){
        entries[k].status = -1;
        entries[k].page = -1;
        entries[k].nextSlot = k + 1;
        newEntries[k].status = -1;
        newEntries[k].page = -1;
        newEntries[k].nextSlot = k + 1;
    }
    entries[k].nextSlot = -1;
    newEntries[k].nextSlot = -1;
    for(int p = 0; p < g1.size(); p++){
        int previndex = g1[p];
        if(previndex != total - 1){
            entries[p].status = oldentries[previndex].status;
            entries[p].page = oldentries[previndex].page;
            //entries[p].page = oldentries[previndex].page;
            entries[p].slot = oldentries[previndex].slot;
            entries[p].mbr = oldentries[previndex].mbr;
        }
        else{
            entries[p].status = 0;
            //entries[p].page = newentry.page;
            entries[p].page = newentry.page;
            entries[p].slot = newentry.slot;
            entries[p].mbr = newentry.mbr;
        }
    }
    for(int p = 0; p < g2.size(); p++){
        int previndex = g2[p];
        if(previndex != total - 1){
            newEntries[p].status = oldentries[previndex].status;
            newEntries[p].page = oldentries[previndex].page;
            //newEntries[p].page = oldentries[previndex].page;
            newEntries[p].slot = oldentries[previndex].slot;
            newEntries[p].mbr = oldentries[previndex].mbr;
        }
        else{
            newEntries[p].status = 0;
            newEntries[p].page = newentry.page;
            //newEntries[p].childpage = newentry.childpage;
            newEntries[p].slot = newentry.slot;
            newEntries[p].mbr = newentry.mbr;
        }
    }
    if((rc = pfh.MarkDirty(newPage)) || (rc = pfh.UnpinPage(newPage)))
        return (rc);
    return rc;
}

int IX_IndexHandle::PickNext(struct IX_NodeEntry *entries, std::vector<bool> &table, void* pData, struct MBR m1, struct MBR m2){
    int i = 0;
    int result;
    float maxd = 0;
    if(!table.back()){
        float d1 = getExpansion((void*)&m1, pData);
        float d2 = getExpansion((void*)&m2, pData);
        float diff = d1 > d2 ? d1 - d2 : d2 - d1;
        if(diff > maxd){
            result = table.size() - 1;
            maxd = diff;
        }
    }
    while(i < table.size() - 1){
        if(!table[i]){
            float d1 = getExpansion((void*)&m1, (void*)&entries[i].mbr);
            float d2 = getExpansion((void*)&m2, (void*)&entries[i].mbr);
            float diff = d1 > d2 ? d1 - d2 : d2 - d1;
            if(diff > maxd){
                result = i;
                maxd = diff;
            }
        } 
    }
    return result;  
}


RC IX_IndexHandle::PickSeeds(struct IX_NodeEntry *entries, int NumEntries, void *pData, int &index1, int &index2){
    int i1, i2;
    //struct MBR temp = *(struct MBR*)pData;
    i1 = 1;
    //i2 = 1;
    RC rc = 0;
    float maxWaste = getExpansion((void*)&entries[0].mbr,pData) - getArea(pData);
    index1 = 0;
    index2 = NumEntries; //indicate pData
    while(i1 < NumEntries){
        float tempWaste = getExpansion((void*)&entries[i1].mbr,pData) - getArea(pData);
        if(tempWaste > maxWaste){
            index1 = i1;
            maxWaste = tempWaste;
        }
        i1++;
    }
    i1 = 0;
    while(i1 < NumEntries - 1){
        i2 = i1 + 1;
        while(i2 < NumEntries){
            float tempWaste = getExpansion((void*)&entries[i1].mbr, (void*)&entries[i2].mbr) - getArea((void*)&entries[i2].mbr);
            if(tempWaste > maxWaste){
                index1 = i1;
                index2 = i2;
                maxWaste = tempWaste;
            }
        }
        i1++;
    }
    return rc;
}

void IX_IndexHandle::getMBR(struct IX_NodeEntry *entries, int numEntries, struct MBR &resultmbr){
    struct MBR temp = entries[0].mbr;
    for(int i = 1; i < numEntries; i++){
        adjustMBR(temp,entries[i].mbr);
    }
    resultmbr = temp;
}

RC IX_IndexHandle::AdjustTree(struct IX_NodeHeader *chosenLeaf,struct MBR mbr){
    RC rc = 0;
    struct IX_NodeHeader *currentNH = chosenLeaf;
    struct MBR currentMBR = mbr;
    while(currentNH->parentPage != -1){
        PageNum parentPageNum = currentNH->parentPage;
        PF_PageHandle parentPH;
        struct IX_NodeHeader *parentHeader;
        if((rc = pfh.GetThisPage(parentPageNum,parentPH)) || (rc = parentPH.GetData((char *&)parentHeader)))
            return rc;
        struct IX_NodeEntry *entries = (struct IX_NodeEntry*)((char*)parentHeader + header.entryOffset_N);
        adjustMBR(entries[currentNH->myindex].mbr,currentMBR);
        currentMBR = entries[currentNH->myindex].mbr;
        currentNH = parentHeader;
    }
    return rc;
}

RC IX_IndexHandle::AdjustTree(PageNum pn1, PageNum pn2){
    struct IX_NodeHeader *L;
    struct IX_NodeHeader *LL;
    PF_PageHandle ph1;
    PF_PageHandle ph2;
    RC rc = 0;
    if((rc = pfh.GetThisPage(pn1,ph1)) || (rc = ph1.GetData((char *&)L)))
        return rc;
    if((rc = pfh.GetThisPage(pn2,ph2)) || (rc = ph2.GetData((char *&)LL)))
        return rc;
    PageNum parentPageNum = L->parentPage;
    struct IX_NodeEntry *entries1 = (struct IX_NodeEntry*)((char*)L + header.entryOffset_N);
    struct IX_NodeEntry *entries2 = (struct IX_NodeEntry*)((char*)LL + header.entryOffset_N);
        
    struct MBR mbr1;
    struct MBR mbr2;
    getMBR(entries1, L->num_entries, mbr1);
    getMBR(entries2, LL->num_entries, mbr2);
    if(parentPageNum == -1){
        PF_PageHandle newParentPH;
        PageNum newParentPN;
        struct IX_NodeHeader *newRoot;
        if((rc = CreateNode(newParentPH, newParentPN,(char*&)newRoot,false,-1,-1)))
            return rc;
        L->parentPage = newParentPN;
        LL->parentPage = newParentPN;
        newRoot->num_entries = 2;
        newRoot->firstSlot = 0;
        newRoot->freeSlot = 2;
        rootPH = newParentPH;
        struct IX_NodeEntry *entries = (struct IX_NodeEntry*)((char*)newRoot + header.entryOffset_N);
        entries[0].status = 0;
        entries[1].status = 0;
        entries[0].page = pn1;
        entries[1].page = pn2;
        
        entries[0].mbr = mbr1;
        entries[1].mbr = mbr2;
        if((rc = pfh.MarkDirty(newParentPN)) || (rc = pfh.UnpinPage(newParentPN)))
            return (rc);

        return rc;
    }
    PF_PageHandle parentPH;
    struct IX_NodeHeader *parentHeader;
    if((rc = pfh.GetThisPage(parentPageNum,parentPH)) || (rc = parentPH.GetData((char *&)parentHeader)))
        return rc;
    if(parentHeader->num_entries != header.M){
        struct IX_NodeEntry *pentries = (struct IX_NodeEntry*)((char*)parentHeader + header.entryOffset_N);
        LL->myindex = parentHeader->freeSlot;
        parentHeader->num_entries++;
        pentries[L->myindex].page = pn1;
        pentries[L->myindex].mbr = mbr1;
        pentries[LL->myindex].status = 0;
        pentries[LL->myindex].page = pn2;
        pentries[LL->myindex].mbr = mbr2;
        parentHeader->freeSlot = pentries[LL->myindex].nextSlot;
        struct MBR current_mbr;
        getMBR(pentries, parentHeader->num_entries, current_mbr);
        AdjustTree(parentHeader,current_mbr);
        //return rc;

    }
    else{
        struct IX_NodeEntry *pentries = (struct IX_NodeEntry*)((char*)parentHeader + header.entryOffset_N);
        pentries[L->myindex].page = pn1;
        pentries[L->myindex].mbr = mbr1;

        struct IX_NodeHeader *newHeader;
        struct IX_NodeEntry newentry;
        newentry.status = 0;
        newentry.page = pn2;
        newentry.mbr = mbr2;
        PageNum newPageNum;
        SplitNode(parentHeader,newHeader,newentry,newPageNum);
        AdjustTree(parentPageNum, newPageNum);
        //return rc;
    }
    if((rc = pfh.MarkDirty(parentPageNum)) || (rc = pfh.UnpinPage(parentPageNum)))
        return (rc);
    return rc;
} 

bool IX_IndexHandle::IsEqualMBR(struct MBR m1, struct MBR m2){
    return (m1.x1 == m2.x1 && m1.x2 == m2.x2 && m1.y1 == m2.y1 && m1.y2 == m2.y2);
}

bool IX_IndexHandle::Overlap(struct MBR rt1, struct MBR rt2){
	if (rt1.x1 > rt2.x2 || rt2.x1 > rt1.x2) return 0;
	if (rt1.y2 < rt2.y1 || rt2.y2 < rt1.y1) return 0;
    return 1;
}
