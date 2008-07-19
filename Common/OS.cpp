// GetDP - Copyright (C) 1997-2008 P. Dular, C. Geuzaine
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <getdp@geuz.org>.

// This file contains a bunch of functions that depend on OS-dependent
// features and/or system calls

// these are available on all OSes
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>

#if !defined(WIN32) || defined(__CYGWIN__)
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

#if defined(WIN32)
#include <windows.h>
#include <process.h>
#include <io.h>
#endif

#if defined(__APPLE__)
#define RUSAGE_SELF      0
#define RUSAGE_CHILDREN -1
#endif

#include "Message.h"

double GetTimeInSeconds()
{
#if !defined(WIN32) || defined(__CYGWIN__)
  struct timeval tp;
  gettimeofday(&tp, (struct timezone *)0);
  double t = (double)tp.tv_sec + 1.e-6 * (double)tp.tv_usec;
  return t;
#else
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  double t =  1.e-7 * 4294967296. * (double)ft.dwHighDateTime +
              1.e-7 * (double)ft.dwLowDateTime;
  return t;
#endif
}

void SleepInSeconds(double s)
{
#if !defined(WIN32) || defined(__CYGWIN__)
  usleep((long)(1.e6 * s));
#else
  Sleep((long)(1.e3 * s));
#endif
}

void GetResources(double *s, long *mem)
{
#if !defined(WIN32) || defined(__CYGWIN__)
  static struct rusage r;
  getrusage(RUSAGE_SELF, &r);
  *s = (double)r.ru_utime.tv_sec + 1.e-6 * (double)r.ru_utime.tv_usec;
  *mem = (long)r.ru_maxrss;
#else
  FILETIME creation, exit, kernel, user;
  if(GetProcessTimes(GetCurrentProcess(), &creation, &exit, &kernel, &user)){
    *s = 1.e-7 * 4294967296. * (double)user.dwHighDateTime +
         1.e-7 * (double)user.dwLowDateTime;
  }
  *mem = 0;
#endif
}

void CheckResources(){
#if !defined (WIN32) || defined(__CYGWIN__)
  static struct rlimit r;

  getrlimit(RLIMIT_STACK, &r);

  // Try to get at least 16 MB of stack. Running with too small a stack
  // can cause crashes in the recursive calls (cf. Cal_Quantity)
  if(r.rlim_cur < 16 * 1024 * 1024){
    Msg::Info("Increasing process stack size (%d kB < 16 MB)", r.rlim_cur / 1024);
    r.rlim_cur = r.rlim_max;
    setrlimit(RLIMIT_STACK, &r);
  }
#endif
}

double Cpu()
{
  long mem = 0;
  double s = 0.;
  GetResources(&s, &mem);
  return s;
}

int GetProcessId()
{
#if !defined(WIN32) || defined(__CYGWIN__)
  return getpid();
#else
  return _getpid();
#endif
}

int UnlinkFile(const char *filename)
{
#if !defined(WIN32) || defined(__CYGWIN__)
  return unlink(filename);
#else
  return _unlink(filename);
#endif
}

int StatFile(const char *filename)
{
#if !defined(WIN32) || defined(__CYGWIN__)
  struct stat buf;
  return stat(filename, &buf);
#else
  struct _stat buf;
  return _stat(filename, &buf);
#endif
}

int KillProcess(int pid)
{
#if !defined(WIN32) || defined(__CYGWIN__)
  if(kill(pid, 9))
    return 0;
#else
  HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
  if(!TerminateProcess(hProc, 0)){
    CloseHandle(hProc);
    return 0;
  }
#endif
  return 1;
}

int SystemCall(const char *command)
{
#if defined(WIN32)
  STARTUPINFO suInfo;
  PROCESS_INFORMATION prInfo;
  memset(&suInfo, 0, sizeof(suInfo));
  suInfo.cb = sizeof(suInfo);
  Msg::Info("Calling '%s'", command);
  CreateProcess(NULL, (char*)command, NULL, NULL, FALSE,
                NORMAL_PRIORITY_CLASS, NULL, NULL, &suInfo, &prInfo);
  return 0;
#else
  if(!system(NULL)) {
    Msg::Error("Could not find /bin/sh: aborting system call");
    return 1;
  }
  Msg::Info("Calling '%s'", command);
  return system(command);
#endif
}
