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
#include <cstdio>

IX_IndexScan::IX_IndexScan()
{
  // Implement this
}

IX_IndexScan::~IX_IndexScan()
{
  // Implement this
}

RC IX_IndexScan::OpenScan(const IX_IndexHandle &indexHandle,
                CompOp compOp,
                void *value,
                ClientHint  pinHint)
{
  // Implement this
}

RC IX_IndexScan::GetNextEntry(RID &rid)
{
  // Implemenet this
}

RC IX_IndexScan::CloseScan()
{
  // Implement this
}
