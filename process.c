#include <stdio.h>
#include <stdlib.h>
#include "mem.h"
#include "process.h"

#ifndef STATIC
  #define STATIC
#endif

long ProcessError;

STATIC uchar *pipe_command(
    char *command, FILE *dptr, size_t (*dread)(void *, size_t, size_t, FILE *),
    uchar *data, long datalen, long maxoutlen, long *outlen)
/* Pipe a file or buffer to a command and return the results as a malloc2ed
 *  object.
 * Enter: char *command: the command to execute.
 *        FILE *dptr: if present, a file-like handle to pass to the dread
 *                    function to read data.  If null, the data and datalen
 *                    parameters must be set.
 *        size_t (*dread)(void *, size_t, size_t, FILE *): a function akin to
 *            fread that can be used to read from dptr.  If null, fread is
 *            used.  Ignored if dptr is  null.
 *        uchar *data: the data to send to the command.  Ignored if dptr is
 *                     set.
 *        long datalen: the length of the data to send, if data is used.
 *        long maxoutlen: the maximum amount of data to receive.
 *        long *outlen: a location to store the amount of data returned.
 * Exit:  uchar *out: the data returned from the command.         4/4/17-DWM */
{
  STARTUPINFOA sui;
  PROCESS_INFORMATION pi;
  SECURITY_ATTRIBUTES sa;
  HANDLE outread;
  HANDLE outwrite;
  HANDLE inread;
  HANDLE inwrite;
  HANDLE writeThread;
  DWORD writeThreadId;
  long chunk=1024*1024, len=0, maxlen;
  DWORD reslen;
  uchar *out, *new;
  PIPEWRITEPARAMS writeParams;

  ProcessError = 0;
  memset(&sa, 0, sizeof(SECURITY_ATTRIBUTES));
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.bInheritHandle = TRUE;
  if (!CreatePipe(&outread, &outwrite, &sa, 0)) {
    return 0;
  }
  if (!SetHandleInformation(outread, HANDLE_FLAG_INHERIT, 0)) {
    CloseHandle(outread);
    CloseHandle(outwrite);
    return 0;
  }
  if (!CreatePipe(&inread, &inwrite, &sa, 0)) {
    CloseHandle(outread);
    CloseHandle(outwrite);
    return 0;
  }
  if (!SetHandleInformation(inwrite, HANDLE_FLAG_INHERIT, 0)) {
    CloseHandle(outread);
    CloseHandle(outwrite);
    CloseHandle(inread);
    CloseHandle(inwrite);
    return 0;
  }
  memset(&sui, 0, sizeof(STARTUPINFOA));
  sui.cb = sizeof(STARTUPINFOA);
  sui.hStdError = outwrite;
  sui.hStdOutput = outwrite;
  sui.hStdInput = inread;
  sui.dwFlags |= STARTF_USESTDHANDLES;
  memset(&pi, 0, sizeof(PROCESS_INFORMATION));
  if (!CreateProcessA(NULL, command, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &sui, &pi)) {
    CloseHandle(outread);
    CloseHandle(outwrite);
    CloseHandle(inread);
    CloseHandle(inwrite);
    return(0);
  }
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  CloseHandle(outwrite);
  CloseHandle(inread);
  writeParams.inwrite = inwrite;
  writeParams.dptr = dptr;
  writeParams.dread = dread ? dread : fread;
  writeParams.data = data;
  writeParams.datalen = datalen;
  writeThread = CreateThread(NULL, 0, pipe_write_thread, (LPVOID)&writeParams, 0, &writeThreadId);
  if (writeThread == NULL) {
    CloseHandle(outread);
    CloseHandle(inwrite);
  }
  if (!(out=malloc2((maxlen=chunk)))) {
    CloseHandle(outread);
    WaitForSingleObject(writeThread, INFINITE);
    ProcessError = 0x506;  return(0); }
  while (ReadFile(outread, out+len, maxlen - len, &reslen, NULL)) {
    len += reslen;
    if (len > maxoutlen) {
      CloseHandle(outread);
      WaitForSingleObject(writeThread, INFINITE);
      free2(out);
      ProcessError = 0x906;  return(0); }
    if (reslen <= 0) {
      break;
    }
    if (len + chunk / 2 >= maxlen) {
      maxlen = len + chunk;
      if (!(new=realloc2(out, maxlen))) {
        CloseHandle(outread);
        WaitForSingleObject(writeThread, INFINITE);
        free2(out);
        ProcessError = 0x506;  return(0); }
      out = new;
    }
  }
  CloseHandle(outread);
  WaitForSingleObject(writeThread, INFINITE);
  outlen[0] = len;
  return out;
}

STATIC uchar *pipe_file_to_command(
    char *command, FILE *dptr, size_t (*dread)(void *, size_t, size_t, FILE *),
    long maxoutlen, long *outlen)
/* Pipe a file or buffer to a command and return the results as a malloc2ed
 *  object.
 * Enter: char *command: the command to execute.
 *        FILE *dptr: if present, a file-like handle to pass to the dread
 *                    function to read data.  If null, the data and datalen
 *                    parameters must be set.
 *        size_t (*dread)(void *, size_t, size_t, FILE *): a function akin to
 *            fread that can be used to read from dptr.  If null, fread is
 *            used.  Ignored if dptr is  null.
 *        long maxoutlen: the maximum amount of data to receive.
 *        long *outlen: a location to store the amount of data returned.
 * Exit:  uchar *out: the data returned from the command.         4/4/17-DWM */
{
  return pipe_command(command, dptr, dread, NULL, 0, maxoutlen, outlen);
}

STATIC uchar *pipe_memory_to_command(
    char *command, uchar *data, long datalen, long maxoutlen, long *outlen)
/* Pipe a buffer to a command and return the results as a malloc2ed object.
 * Enter: char *command: the command to execute.
 *        uchar *data: the data to send to the command.  Ignored if dptr is
 *                     set.
 *        long datalen: the length of the data to send, if data is used.
 *        long maxoutlen: the maximum amount of data to receive.
 *        long *outlen: a location to store the amount of data returned.
 * Exit:  uchar *out: the data returned from the command.         4/4/17-DWM */
{
  return pipe_command(command, NULL, NULL, data, datalen, maxoutlen, outlen);
}

DWORD WINAPI pipe_write_thread(LPVOID lpWriteParams)
/* Send an input file or buffer to a pipe.
 * Enter: LPVOID lpWriteParams: a pointer to a structure with the write
 *                              parameters.
 * Exit:  DWORD result: 0 for failure, 1 for success.             4/4/17-DWM */
{
  PIPEWRITEPARAMS *writeParams = (PIPEWRITEPARAMS *)lpWriteParams;
  long reslen, pos = 0, chunk = 64 * 1024, inlen;
  uchar buf[65536];

  if (writeParams->dptr) {
    while (1) {
      inlen = writeParams->dread(buf, 1, chunk, writeParams->dptr);
      if (!inlen) {
        break;
      }
      pos = 0;
      while (pos < inlen) {
        if (!WriteFile(writeParams->inwrite, buf + pos, inlen - pos, &reslen, NULL)) {
          CloseHandle(writeParams->inwrite);
          return 0;
        }
        pos += reslen;
      }
    }
  } else {
    while (pos < writeParams->datalen) {
      if (!WriteFile(writeParams->inwrite, writeParams->data + pos, writeParams->datalen - pos > chunk ? chunk : writeParams->datalen - pos, &reslen, NULL)) {
        CloseHandle(writeParams->inwrite);
        return 0;
      }
      pos += reslen;
    }
  }
  CloseHandle(writeParams->inwrite);
  return 1;
}
