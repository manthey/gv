/* This file contains c-like windows memory management routines.
 *                                                             9/18/00-DWM */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "mem.h"

typedef struct MALLOCTBL {
  HANDLE hmem;
  void *mem;
  long count; } MALLOCTBL;

static HANDLE HMalloc;
static long MallocMax=0, MallocNum=0;
static MALLOCTBL *MallocTbl=0;

void free2(void *mem)
/* Free memory allocated through Windows calls.
 * Enter: void *mem: pointer to memory.                         5/4/96-DWM */
{
  long i;
  HANDLE hmem;

  if (!mem)  return;
  for (i=0; i<MallocNum; i++)
	if (mem==MallocTbl[i].mem) {
	  hmem = MallocTbl[i].hmem;
	  GlobalUnlock(hmem);
	  GlobalFree(hmem);
	  memmove(MallocTbl+i, MallocTbl+i+1, (MallocNum-i-1)*sizeof(MALLOCTBL));
	  MallocNum--;
	  return; }
}

void free2_all(void)
/* Free all pointers allocated using malloc2 or locked with lock2.
 *                                                             2/11/97-DWM */
{
  long i;

  for (i=MallocNum-1; i>=0; i--)
	free2(MallocTbl[i].mem);
}

HANDLE handle2(void *mem)
/* Return the handle associated with a memory pointer.  This neither locks
 *  nor unlocks the memory area.  If the memory area is not locked with the
 *  malloc2, realloc2, or lock2 calls, then the pointer is handle from
 *  windows.
 * Enter: void *mem: pointer to memory.
 * Exit:  HANDLE hmem: handle to the memory.                   2/12/97-DWM */
{
  long i;

  if (!mem)  return(0);
  for (i=0; i<MallocNum; i++)
	if (mem==MallocTbl[i].mem)
	  return(MallocTbl[i].hmem);
  return(GlobalHandle(mem));
}

void *lock2(HANDLE hmem)
/* Lock down a windows handle so that it can be accessed via pointers and
 *  manipulated with the free2 and realloc2 commands.  This can only fail if
 *  there are too many pointers allocated.  If a handle that is already
 *  locked is passed, the lock count is increased.
 * Enter: HANDLE hmem: handle to windows memory.
 * Exit:  void *mem: pointer to memory.                        2/11/97-DWM */
{
  char *mem;
  HANDLE hmem2;
  long i;

  if (!hmem)  return(0);
  for (i=0; i<MallocNum; i++)
	if (hmem==MallocTbl[i].hmem) {
	  MallocTbl[i].count++;
	  GlobalLock(hmem);
	  return(MallocTbl[i].mem); }
  if (MallocNum>=MallocMax) {
	if (!MallocMax) {
	  HMalloc = GlobalAlloc(GMEM_MOVEABLE, 1000*sizeof(MALLOCTBL));
	  if (!HMalloc)  return(0);
	  MallocTbl = GlobalLock(HMalloc);
	  MallocMax = 1000; }
	else {
	  GlobalUnlock(HMalloc);
	  hmem2 = GlobalReAlloc(HMalloc, (MallocMax+1000)*sizeof(MALLOCTBL),
							GMEM_MOVEABLE);
	  if (!hmem2) {
		MallocTbl = GlobalLock(HMalloc);  return(0); }
	  HMalloc = hmem2;  MallocTbl = GlobalLock(HMalloc);
	  MallocMax += 1000; } }
  mem = GlobalLock(hmem);
  MallocTbl[MallocNum].hmem  = hmem;
  MallocTbl[MallocNum].mem   = mem;
  MallocTbl[MallocNum].count = 1;
  MallocNum++;
  return(mem);
}

void *malloc2(size_t size)
/* Allocate memory through Windows calls.
 * Enter: size_t size: number of bytes to allocate.
 * Exit:  void *mem: pointer to memory.                         5/4/96-DWM */
{
  HANDLE hmem;
  char *mem;

  if (!size)  return(0);
  if (MallocNum>=MallocMax) {
	if (!MallocMax) {
	  HMalloc = GlobalAlloc(GMEM_MOVEABLE, 1000*sizeof(MALLOCTBL));
	  if (!HMalloc)  return(0);
	  MallocTbl = GlobalLock(HMalloc);
	  MallocMax = 1000; }
	else {
	  GlobalUnlock(HMalloc);
	  hmem = GlobalReAlloc(HMalloc, (MallocMax+1000)*sizeof(MALLOCTBL),
						   GMEM_MOVEABLE);
	  if (!hmem) {
		MallocTbl = GlobalLock(HMalloc);  return(0); }
	  HMalloc = hmem;  MallocTbl = GlobalLock(HMalloc);
	  MallocMax += 1000; } }
  hmem = GlobalAlloc(GMEM_MOVEABLE, size);
  if (!hmem)  return(0);
  mem = GlobalLock(hmem);
  MallocTbl[MallocNum].hmem  = hmem;
  MallocTbl[MallocNum].mem   = mem;
  MallocTbl[MallocNum].count = 1;
  MallocNum++;
  return(mem);
}

void *pointer2(HANDLE hmem)
/* Return the pointer associated with a locked memory handle.  This neither
 *  locks nor unlocks the memory area.
 * Enter: HANDLE hmem: handle to the memory.
 * Exit:  void *mem: pointer to memory.  Null if the handle isn't locked.
 *                                                             2/20/01-DWM */
{
  long i;

  if (!hmem)  return(0);
  for (i=0; i<MallocNum; i++)
	if (hmem==MallocTbl[i].hmem)
	  return(MallocTbl[i].mem);
  return(0);
}

void *realloc2(void *current, size_t size)
/* Realloc a memory block using Windows calls.
 * Enter: void *current: pointer to current data.
 *        size_t size: number of bytes to allocate for the new block.
 * Exit:  void *mem: pointer to memory.                         5/4/96-DWM */
{
  long i;
  HANDLE hmem, hmem2;

  if (!size)  size = 1;
  if (!current)  return(malloc2(size));
  /* Reallocate to even size blocks to help avoid memory fragmentation. */
  if (size>2*1024*1024)  size = (((size-1)/(1024*1024))+1)*1024*1024;
  else { for (i=1; i<size; i*=2);  size = i; }
  for (i=0; i<MallocNum; i++)
	if (current==MallocTbl[i].mem) {
	  hmem = (HANDLE)MallocTbl[i].hmem;
	  GlobalUnlock(hmem);
	  hmem2 = GlobalReAlloc(hmem, size, GMEM_MOVEABLE);
	  if (!hmem2) {
		GlobalLock(hmem);  return(0); }
	  current = GlobalLock(hmem2);
	  MallocTbl[i].hmem = hmem2;
	  MallocTbl[i].mem = current;
	  return(current); }
  return(0);
}

long size2(void *mem)
/* Return the size of the specified memory block.  If the pointer does not
 *  point to the base address of a memory block, a value of zero is returned.
 * Enter: void *mem: pointer to a locked memory block.
 * Exit:  long size: size of the memory block in bytes.        2/27/01-DWM */
{
  HANDLE hmem;

  if (!mem)  return(0);
  if (!(hmem=GlobalHandle(mem)))  return(0);
  return(GlobalSize(hmem));
}

HANDLE unlock2(void *mem)
/* Unlock a memory pointer which was allocated using malloc2, returning the
 *  Windows handle associated with it.  This piece of memory is no longer
 *  allocated using free2 and malloc2, but instead requires the Windows
 *  routines.  If the memory was locked multiple times, the lock count is
 *  decremented and the memory remains locked.
 * Enter: void *mem: pointer to memory.
 * Exit:  HANDLE hmem: handle to the memory.                   2/12/97-DWM */
{
  long i;
  HANDLE hmem;

  if (!mem)  return(0);
  for (i=0; i<MallocNum; i++)
	if (mem==MallocTbl[i].mem) {
	  hmem = MallocTbl[i].hmem;
	  GlobalUnlock(hmem);
	  MallocTbl[i].count--;
	  if (!MallocTbl[i].count) {
		memmove(MallocTbl+i, MallocTbl+i+1,
				(MallocNum-i-1)*sizeof(MALLOCTBL));
		MallocNum--; }
	  return(hmem); }
  return(0);
}
