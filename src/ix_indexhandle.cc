//
// File:        ix_indexhandle.cc
// Description: IX_IndexHandle handles manipulations within the index
// Author:      <Your Name Here>
//

#include <unistd.h>
#include <sys/types.h>
#include "ix.h"
#include "pf.h"
#include "mbr.h"
#include "comparators.h"
#include <cstdio>


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
	struct IX_NodeHeader choosenleaf;
	if((rc = ChooseLeaf(rootheader, pData, choosenleaf);
		return rc;
	//check if there are duplicate mbr in the leaf node
	struct IX_NodeEntry *entries = (struct IX_NodeEntry*)((char*)choosenleaf + header.entryOffset_N);
	int index = choosenleaf->firstSlot;
	while(index != -1){
		char *value = &entries[index].mbr;
		float tempExpansion = getExpansion((void*)value, pData);
		if(tempExpansion == 0){
			break;
		}
		index = &entries[index].nextSlot;
	}
	//if there are duplicates
	if(index != -1){
		struct IX_NodeEntry dupEntry = entries[index];
		//if it already contain duplicate value
		PageNum bucketPage;
		//already a bucket
		if(dupEntry.status == 1){
			bucketPage = dupEntry.page;
			if((rc = InsertToBucket(bucketPage, rid)))
				return rc;
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
		}
	}
	else{
	//no duplicate, insert to node directly
		if(choosenleaf->num_entries == header.M){
		//unfortunately, node is full
			PageNum newPage;
			char* newData;
			PF_PageHandle newPH;
			PageNum parentp = choosenleaf->parentPage;
			int newindex = entries[index].nextSlot;
			if((rc = CreateNode(newPH, newPage, newData, true, parentp)))
		
}

RC IX_IndexHandle::DeleteEntry(void *pData, const RID &rid)
{
  // Implement this
}

RC IX_IndexHandle::ForcePages()
{
  // Implement this
  RC rc = 0;
  pfh.ForcePages();
  return rc;
}

//private helpers
RC IX_IndexHandle::CreateNode(PF_PageHandle &ph, PageNum &page, char *nData, bool isLeaf, PageNum parent, int myindex){
    RC rc = 0;
    if((rc = pfh.AllocatePage(ph)) || (rc = ph.GetPageNum(page))){
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

RC IX_IndexHandle::ChooseLeaf(struct IX_NodeHeader *rHeader, void *pData, struct IX_NodeHeader &result){
	RC rc = 0;
	struct IX_NodeHeader *current_h;
	current_h = rHeader;
	while(!current_h->isLeaf){
		struct IX_NodeEntry *entries = (struct IX_NodeEntry*)((char*)current_h + header.entryOffset_N);
		int index = current_h->firstSlot;
		int bestindex = index;
		float minExpansion;
		char *value = &entries[index].mbr;
		minExpansion = getExpansion((void*)value, pData);
		index++;
		while(index != -1){
			char *value = &entries[index].mbr;
			int tempExpansion = getExpansion((void*)value, pData);
			if(tempExpansion <= minExpansion){
				if((tempExpansion == minExpansion && getArea((void*)value) < getArea((void*)&entries[bestindex])) || tempExpansion < minExpansion){
					bestindex = index;
					minExpansion = tempExpansion;
				}
			}
			index++;
		}
		PageNum nextPageNum = entries[bestindex].page;
		PF_PageHandle nextPH;
		if((rc = pfh.GetThisPage(nextPageNum, nextPH)) || (rc = nextPH.GetData((char *&)current_h)))
			return rc;
		
	}
	result = *current_h;
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
			bukcetHeader->num_entries++;
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
	float newx1 = min(m1.x1, m2.x1);
	float newy2 = min(m1.y1, m2.y1);
	float newx2 = max(m1.x2, m2.x2);
	float newy2 = max(m1.y2, m2.y2);
	float new_area = (newx2 - newx1) * (newy2 - newy1);
	float old_area = getArea(m1);
	return new_area - old_area;
}	
