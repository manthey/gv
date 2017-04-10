#ifndef PROCESSHEADER                            /* Prevent double inclusion */
#define PROCESSHEADER 1

#include <windows.h>

#define uchar unsigned char

typedef struct PIPEWRITEPARAMS {
  HANDLE inwrite;
  FILE *dptr;
  size_t (*dread)(void *, size_t, size_t, FILE *);
  uchar *data;
  long datalen; } PIPEWRITEPARAMS;

extern long ProcessError;

uchar *pipe_command     (char *command, FILE *dptr,
                         size_t (*dread)(void *, size_t, size_t, FILE *),
                         uchar *data, long datalen, long maxoutlen,
                         long *outlen);
uchar *pipe_file_to_command(char *command, FILE *dptr,
                         size_t (*dread)(void *, size_t, size_t, FILE *),
                         long maxoutlen, long *outlen);
uchar *pipe_memory_to_command(char *command, uchar *data, long datalen,
                         long maxoutlen, long *outlen);
DWORD WINAPI pipe_write_thread(LPVOID lpWriteParams);

#endif                                  /* End of prevent double inclusion */
