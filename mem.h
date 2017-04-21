#ifndef MEMHEADER                              /* Prevent double inclusion */
#define MEMHEADER 1

#include <stdlib.h>
#include <windows.h>

void   free2    (void *mem);
void   free2_all(void);
HANDLE handle2  (void *mem);
void  *lock2    (HANDLE hmem);
void  *malloc2  (size_t size);
void  *pointer2 (HANDLE hmem);
void  *realloc2 (void *current, size_t size);
long   size2    (void *mem);
HANDLE unlock2  (void *mem);

#endif                                  /* End of prevent double inclusion */
