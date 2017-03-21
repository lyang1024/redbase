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
#include <iostream>
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
	//std::cout<<"file allocated -1"<<"\n";
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
	//std::cout<<"file allocated 0"<<"\n";
	struct IX_IndexHeader *header;
	struct IX_NodeHeader *rootheader;
	struct IX_NodeEntry *entries;
	if((rc = ph_header.GetData((char*&) header)) || (rc = ph_root.GetData((char *&) rootheader))){
    RC flag = 0;
	    if((flag = fh.MarkDirty(headerpage)) || (flag = fh.UnpinPage(headerpage)) || (flag = fh.MarkDirty(rootpage)) || (flag = fh.UnpinPage(rootpage)) || (flag = pfm.CloseFile(fh)))
		    return flag;
	    return rc;
	}
	//std::cout<<"file allocated"<<"\n";
	//int max_entries = floor((PF_PAGE_SIZE - sizeof(struct IX_NodeHeader))/(sizeof(struct IX_NodeEntry)));
	//int max_buckets = floor((PF_PAGE_SIZE - sizeof(struct IX_BucketHeader))/(sizeof(struct IX_BucketEntry)));
	int max_entries = 2;
	int max_buckets = 2;
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
    rootheader->parentPage = -1;
	rootheader->myindex = -1;
	entries = (struct IX_NodeEntry *)((char *)rootheader + header->entryOffset_N);
	int i;
	for(i = 0; i < header->M - 1; i++){
		entries[i].status = -1;
		entries[i].page = -1;
		entries[i].nextSlot = i + 1;
		entries[i].slot = -1;
	}
	entries[i].nextSlot = -1;
	//std::cout<<"all set"<<"\n";
	RC flag = 0;
	if((flag = fh.MarkDirty(headerpage)) || (flag = fh.UnpinPage(headerpage)) || (flag = fh.MarkDirty(rootpage)) || (flag = fh.UnpinPage(rootpage)) || (flag = pfm.CloseFile(fh)))
		//std::cout<<"here ?"<<"\n";
		return flag;
	std::cout<<"index created"<<"\n";
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
	std::string indexname =  std::string(fileName) + "." + std::to_string(indexNo);
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

	rc = SetUpIH(indexHandle, fh, iheader);
	RC rc2 = 0;
	if((rc2 = fh.UnpinPage(firstpage)))
		return (rc2);

	if(rc != 0){
		pfm.CloseFile(fh);
	}
	return (rc);

}

/*
 * Given a valid index handle, closes the file associated with it
 */

RC IX_Manager::CleanUpIH(IX_IndexHandle &indexHandle){
	if(indexHandle.isOpenHandle == false)
		return (IX_INVALIDINDEXHANDLE);
	indexHandle.isOpenHandle = false;
	return (0);
}


RC IX_Manager::DestroyIndex(const char *fileName, int indexNo){
    RC rc;
    if(fileName == NULL || indexNo < 0)
        return (IX_BADFILENAME);
    std::string indexname = std::string(fileName) + "." + std::to_string(indexNo);

    if((rc = pfm.DestroyFile(indexname.c_str())))
        return (rc);
    return (0);
}

/*
 * This function sets up the private variables of an IX_IndexHandle to get it
 * ready to refer to an open file
 */
RC IX_Manager::SetUpIH(IX_IndexHandle &ih, PF_FileHandle &fh, struct IX_IndexHeader *header){
    RC rc = 0;
    memcpy(&ih.header, header, sizeof(struct IX_IndexHeader));

    // check that this is a valid index file
    /*
    if(! IsValidIndex(ih.header.attr_type, ih.header.attr_length))
        return (IX_INVALIDINDEXFILE);

    if(! ih.isValidIndexHeader()){ // check that the header is valid
        return (rc);
    }
     */

    if((rc = fh.GetThisPage(header->rootPage, ih.rootPH))){ // retrieve the root page
        return (rc);
    }


	if(ih.header.attr_type == INT){
		ih.comparator = compare_int;
	}
	else if(ih.header.attr_type == FLOAT){
		ih.comparator = compare_float;
	}

	else if(ih.header.attr_type == MBR){
		ih.comparator = compare_overlap;
	}

	else{
		ih.comparator = compare_string;
	}
    ih.header_modified = false;
    ih.pfh = fh;
    ih.isOpenHandle = true;
    return (rc);
}




/*
 * Given a valid index handle, closes the file associated with it
 */
RC IX_Manager::CloseIndex(IX_IndexHandle &indexHandle){
    RC rc = 0;
    PF_PageHandle ph;
    PageNum page;
    char *pData;

    if(indexHandle.isOpenHandle == false){ // checks that it's a valid index handle
        return (IX_INVALIDINDEXHANDLE);
    }

    // rewrite the root page and unpin it
    PageNum root = indexHandle.header.rootPage;
    if((rc = indexHandle.pfh.MarkDirty(root)) || (rc = indexHandle.pfh.UnpinPage(root)))
        return (rc);

    // Check that the header is modified. If so, write that too.
    //if(indexHandle.header_modified == true){
	if(true){
        if((rc = indexHandle.pfh.GetFirstPage(ph)) || ph.GetPageNum(page))
            return (rc);
        if((rc = ph.GetData(pData))){
            RC rc2;
            if((rc2 = indexHandle.pfh.UnpinPage(page)))
                return (rc2);
            return (rc);
        }
        memcpy(pData, &indexHandle.header, sizeof(struct IX_IndexHeader));
        if((rc = indexHandle.pfh.MarkDirty(page)) || (rc = indexHandle.pfh.UnpinPage(page)))
            return (rc);
    }

    // Close the file
    if((rc = pfm.CloseFile(indexHandle.pfh)));
        //return (rc);

    if((rc = CleanUpIH(indexHandle)));
        //return (rc);

	rc = 0;
    return (rc);
}