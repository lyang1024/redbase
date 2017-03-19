//
// File:        ix_indexmanager.cc
// Description: IX_IndexHandle handles indexes
// Author:      Yifei Huang (yifei@stanford.edu)
//

#include <unistd.h>
#include <sys/types.h>
#include "ix.h"
#include "pf.h"
#include "mbr.h"
#include <climits>
#include <string>
#include <sstream>
#include <math.h>
#include <cstdio>
#include "comparators.h"
#include "ix_internal.h"


IX_Manager::IX_Manager(PF_Manager &pfm) : pfm(pfm)
{
}

IX_Manager::~IX_Manager()
{

}

/*
 * Creates a new index given the filename, the index number, attribute type and length.
 */
RC IX_Manager::CreateIndex(const char *fileName, int indexNo,
                   AttrType attrType, int attrLength)
{
	if(fileName == NULL || indexNo < 0)
		return (IX_BADFILENAME);
	//may check if attr length is correct
	RC rc = 0;
	std::string indexname = std::string(fileName) + "." + std::to_string(indexNo);

	if((rc = pfm.CreateFile(indexname.c_str())))
		return rc;
	
	PF_FileHandle fh;
	PF_PageHandle ph_header;
	PF_PageHandle ph_root;
	if((rc = pfm.OpenFile(indexname.c_str(),fh)))
		return rc;
	PageNum headerpage;
	PageNum rootpage;
	if((rc = fh.AllocatePage(ph_header)) || (rc = ph_header.GetPageNum(headerpage)) || (rc = fh.AllocatePage(ph_root)) || ((rc = ph_root.GetPageNum(rootpage)))){
		return rc;
	}
	struct IX_IndexHeader *header;
	struct IX_NodeHeader *rootheader;
	struct IX_NodeEntry *entries;
	if((rc = ph_header.GetData((char*&) header)) || (rc = ph_root.GetData((char *&) rootheader))){
    RC flag;
	    if((flag = fh.MarkDirty(headerpage)) || (flag = fh.UnpinPage(headerpage)) || (flag = fh.MarkDirty(rootpage)) || (flag = fh.UnpinPage(rootpage)) || (flag = pfm.CloseFile(fh)))
		    return flag;
	    return rc;
	}

	int max_entries = floor((PF_PAGE_SIZE - sizeof(struct IX_NodeHeader))/(sizeof(struct IX_NodeEntry) + attrLength));
	int max_buckets = floor((PF_PAGE_SIZE - sizeof(struct IX_BucketHeader))/(sizeof(struct IX_BucketEntry)));
	header->attr_type = attrType;
	header->attr_length = attrLength;
	header->M = max_entries;
	header->m = max_entries/4; //set m to M/4 for now
	header->Mb = max_buckets;

	header->entryOffset_N = sizeof(struct IX_NodeHeader);
	header->entryOffset_B = sizeof(struct IX_BucketHeader);
	//header->numEntryOffset_N = header->entryOffset_N + max_entries*sizeof(
	header->rootPage = rootpage;

	rootheader->isLeaf = true;
	rootheader->isEmpty = true;
	rootheader->num_entries = 0;
	//rootheader->page = -1; //no child page for now
	rootheader->firstSlot = -1;
	rootheader->freeSlot = 0;
	entries = (struct IX_NodeEntry *)((char *)rootheader + header->entryOffset_N);
	int i;
	for(i = 0; i < header->M - 1; i++){
		entries[i].status = -1;
		entries[i].page = -1;
		entries[i].nextSlot = i + 1;
	}
	entries[i].nextSlot = -1;

	RC flag;
	if((flag = fh.MarkDirty(headerpage)) || (flag = fh.UnpinPage(headerpage)) || (flag = fh.MarkDirty(rootpage)) || (flag = fh.UnpinPage(rootpage)) || (flag = pfm.CloseFile(fh)))
		return flag;
	return rc;
}

/*
 * This function destroys a valid index given the file name and index number.
 */
RC IX_Manager::DestroyIndex(const char *fileName, int indexNo)
{
	RC rc = 0;
	std::string indexname = std::string(fileName) + "." + std::to_string(indexNo);
    if((rc = pfm.DestroyFile(indexname.c_str())))
		return rc;
	return rc;
}

/*
 * This function, given a valid fileName and index Number, opens up the
 * index associated with it, and returns it via the indexHandle variable
 */
RC IX_Manager::OpenIndex(const char *fileName, int indexNo,
                 IX_IndexHandle &indexHandle)
{
    if(fileName == NULL || indexNo < 0){
		return (IX_BADFILENAME);
	}
    /*
	if(indexHandle.isOpenHandle == true){
		return (IX_INVALIDINDEXHANDLE);
	}
    */
	RC rc = 0;
	PF_FileHandle fh;
	std::string indexname;
	if((rc = pfm.OpenFile(indexname.c_str(),fh)))
		return rc;
	PF_PageHandle ph;
	PageNum firstpage;
	char *pData;
	//if unsuccessful, unpin and exit
	if((rc = fh.GetFirstPage(ph)) || (rc = ph.GetPageNum(firstpage)) || (ph.GetData(pData))){
		fh.UnpinPage(firstpage);
		pfm.CloseFile(fh);
		return rc;
	}
	struct IX_IndexHeader *iheader = (struct IX_IndexHeader*) pData;
	memcpy(&indexHandle.header, iheader, sizeof(struct IX_IndexHeader));

	if((rc = fh.GetThisPage(iheader->rootPage, indexHandle.rootPH)))
		return rc;
	
	if(indexHandle.header.attr_type == INT){
		indexHandle.comparator = compare_int;
	}
	else if(indexHandle.header.attr_type == FLOAT){
		indexHandle.comparator = compare_float;
	}
	/*
	else if(indexHandle.header.attr_type == MBR){
		indexHandle.comparator = get_overlap;
	}
	*/
	else{
		indexHandle.comparator = compare_string;
	}
	indexHandle.pfh = fh;
	
	RC flag;
	if((flag = fh.UnpinPage(firstpage)))
		return flag;
	if((rc != 0)){
		pfm.CloseFile(fh);
	}
	return rc;

}

/*
 * Given a valid index handle, closes the file associated with it
 */
RC IX_Manager::CloseIndex(IX_IndexHandle &indexHandle)
{
	RC rc = 0;
	PF_PageHandle ph;
	PageNum page;
	char *pData;

	PageNum root = indexHandle.header.rootPage;
	if((rc = indexHandle.pfh.MarkDirty(root)) || (rc = indexHandle.pfh.UnpinPage(root)))
		return rc;
	
	if(indexHandle.dirtyHeader){
		if((rc = indexHandle.pfh.GetFirstPage(ph)) || ph.GetPageNum(page))
			return rc;
		if((rc = ph.GetData(pData))){
			RC flag;
			if((flag = indexHandle.pfh.UnpinPage(page)))
				return flag;
			return flag;
		}
		memcpy(pData, &indexHandle.header, sizeof(struct IX_IndexHeader));
		if((rc = indexHandle.pfh.MarkDirty(page)) || (rc = indexHandle.pfh.UnpinPage(page)))
			return rc;
	}

	if((rc = pfm.CloseFile(indexHandle.pfh)))
		return rc;
	return rc;
}
