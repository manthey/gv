#include <stdio.h>
#include <stdlib.h>
#include "mem.h"
#include "process.h"

#ifndef STATIC
  #define STATIC
#endif

long ProcessError;

STATIC void add_handle(HANDLE pipe, HANDLE *closeList)
{
  int i;

  for (i = 0; closeList[i]; i += 1);
  closeList[i] = pipe;
}

STATIC void close_handle(HANDLE *pipe, HANDLE *closeList)
{
  int i;

  for (i = 0; closeList[i]; i += 1) {
    if (closeList[i] == *pipe) {
      CloseHandle(*pipe);
      *pipe = 0;
      closeList[i] = 0;
      break;
    }
  }
  for (i = i + 1; closeList[i]; i += 1) {
    closeList[i - 1] = closeList[i];
    closeList[i] = 0;
  }
}

STATIC void close_handles(HANDLE *closeList)
{
  int i;

  for (i = 0; closeList[i]; i += 1) {
    CloseHandle(closeList[i]);
    closeList[i] = 0;
  }
}

STATIC int create_pipe(HANDLE *read, HANDLE *write, HANDLE *closeList)
{
  int i;

  if (!CreatePipe(read, write, NULL, 0)) {
    close_handles(closeList);
    return 0;
  }
  for (i = 0; closeList[i]; i += 1);
  closeList[i] = *read;
  closeList[i + 1] = *write;
  return 1;
}

STATIC int inherit_pipe(HANDLE *pipe, HANDLE *closeList, int useNul)
{
  HANDLE dup;
  int i, res = 0;

  if (*pipe)
    res = DuplicateHandle(GetCurrentProcess(), *pipe, GetCurrentProcess(), &dup, 0, TRUE, DUPLICATE_SAME_ACCESS);
  if (!res && useNul) {
    if (*pipe)
      close_handle(pipe, closeList);
    *pipe = CreateFile("NUL", GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (*pipe == INVALID_HANDLE_VALUE) {
      *pipe = 0;
      close_handles(closeList);
      return 0;
    }
    add_handle(pipe, closeList);
    res = DuplicateHandle(GetCurrentProcess(), *pipe, GetCurrentProcess(), &dup, 0, TRUE, DUPLICATE_SAME_ACCESS);
  }
  if (!res) {
    close_handles(closeList);
    return 0;
  }
  for (i = 0; closeList[i]; i += 1) {
    if (closeList[i] == *pipe) {
      CloseHandle(closeList[i]);
      closeList[i] = dup;
    }
  }
  *pipe = dup;
  return 1;
}

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
  HANDLE outread;
  HANDLE outwrite;
  HANDLE inread;
  HANDLE inwrite;
  HANDLE errwrite;
  HANDLE writeThread;
  HANDLE closeList[10];
  DWORD writeThreadId;
  long chunk=1024*1024, len=0, maxlen;
  DWORD reslen;
  uchar *out, *new;
  PIPEWRITEPARAMS writeParams;
  char *cmd;

  ProcessError = 0;
  memset(closeList, 0, sizeof(HANDLE) * 10);
  if (!create_pipe(&inread, &inwrite, closeList))    return 0;
  if (!inherit_pipe(&inread, closeList, 0))          return 0;
  if (!create_pipe(&outread, &outwrite, closeList))  return 0;
  if (!inherit_pipe(&outwrite, closeList, 0))        return 0;
  errwrite = GetStdHandle(STD_ERROR_HANDLE);
  if (errwrite == INVALID_HANDLE_VALUE || !errwrite) {
    errwrite = 0;
  } else {
    add_handle(errwrite, closeList);
  }
  if (!inherit_pipe(&errwrite, closeList, 1))        return 0;

  memset(&sui, 0, sizeof(STARTUPINFOA));
  sui.cb = sizeof(STARTUPINFOA);
  sui.hStdInput = inread;
  sui.hStdOutput = outwrite;
  sui.hStdError = errwrite;
  sui.dwFlags |= STARTF_USESTDHANDLES;
  sui.wShowWindow = SW_HIDE;
  memset(&pi, 0, sizeof(PROCESS_INFORMATION));

  cmd = malloc2(strlen(command) + 1);
  if (!cmd) {
    close_handles(closeList);
    ProcessError = 0x506;  return 0; }
  strcpy(cmd, command);

  if (!CreateProcessA(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &sui, &pi)) {
    free2(cmd);
    close_handles(closeList);
    return 0;
  }
  close_handle(&inread, closeList);
  close_handle(&outwrite, closeList);
  close_handle(&errwrite, closeList);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  writeParams.inwrite = inwrite;
  writeParams.dptr = dptr;
  writeParams.dread = dread ? dread : fread;
  writeParams.data = data;
  writeParams.datalen = datalen;
  writeThread = CreateThread(NULL, 0, pipe_write_thread, (LPVOID)&writeParams, 0, &writeThreadId);
  if (writeThread == NULL) {
    close_handles(closeList);
    free2(cmd);
    ProcessError = 0x906;  return 0;
  }
  if (!(out=malloc2((maxlen=chunk)))) {
    CloseHandle(outread);
    WaitForSingleObject(writeThread, INFINITE);
    free2(cmd);
    ProcessError = 0x506;  return 0; }
  while (ReadFile(outread, out+len, maxlen - len, &reslen, NULL)) {
    len += reslen;
    if (len > maxoutlen) {
      CloseHandle(outread);
      WaitForSingleObject(writeThread, INFINITE);
      free2(out);
      free2(cmd);
      ProcessError = 0x906;  return 0; }
    if (reslen <= 0) {
      break;
    }
    if (len + chunk / 2 >= maxlen) {
      maxlen = len + chunk;
      if (!(new=realloc2(out, maxlen))) {
        CloseHandle(outread);
        WaitForSingleObject(writeThread, INFINITE);
        free2(out);
        free2(cmd);
        ProcessError = 0x506;  return 0; }
      out = new;
    }
  }
  CloseHandle(outread);
  WaitForSingleObject(pi.hProcess, INFINITE);
  WaitForSingleObject(writeThread, INFINITE);
  free2(cmd);
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
  uchar buf[64 * 1024];

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
