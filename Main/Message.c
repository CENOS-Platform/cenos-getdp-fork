#define RCSID "$Id: Message.c,v 1.53 2002-11-10 19:29:38 geuzaine Exp $"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#ifdef __APPLE__
#include <sys/time.h>
#endif /* __APPLE__ */
#ifndef MSDOS
#include <sys/resource.h>
#endif /* MSDOS */
#ifdef __APPLE__
#define   RUSAGE_SELF      0
#define   RUSAGE_CHILDREN -1
#endif /* __APPLE__ */

#include "GetDP.h"
#include "CurrentData.h"
#include "Version.h"
#include "LinAlg.h"
#include "GmshClient.h"

extern int     InteractiveLevel, InteractiveInterrupt ;

void FinalizeAndExit(void);

/* ------------------------------------------------------------------------ */
/*  I n f o                                                                 */
/* ------------------------------------------------------------------------ */

/* The Sparskit is under GNU license. If we mention it, we have to 
   make GetDP free sofware... */

char acronym[]   = "GetDP %g, a General environment for the treatment of Discrete Problems\n";
char copyright[] = "Copyright (c) 1997-2002 Patrick Dular and Christophe Geuzaine\n";

char version[]   = "Version      : %g\n";
char os[]        = "Build OS     : %s\n";
char date[]      = "Build date   : %s\n";
char host[]      = "Build host   : %s\n";
char packager[]  = "Packager     : %s\n";
char url[]       = "Web site     : http://www.geuz.org/getdp/\n";
char email[]     = "Mailing list : getdp@geuz.org\n";

#ifdef _SPARSKIT
#ifdef _ILU_FLOAT
char solver[]    = "Solver       : Default (real arithmetic, single precision preconditioning)\n";
#else
char solver[]    = "Solver       : Default (real arithmetic)\n";
#endif
#else
#ifdef PETSC_USE_COMPLEX
char solver[]    = "Solver       : PETSc (complex arithmetic)\n";
#else
char solver[]    = "Solver       : PETSc (real arithmetic)\n";
#endif
#endif

char help[] = 
#if _SPARSKIT
  "Usage: %s [file] [options]\n"
#else
  "Usage: mpirun [MPI options] %s [file] [options] [PETSc options]\n"
#endif
  "Processing options:\n"
  "  -pre 'Resolution'         pre-processing\n"
  "  -cal                      processing\n"
  "  -pos 'PostOperation(s)'   post-processing\n"
  "  -ipos 'PostProcessing(s)' interactive post-processing\n"
  "  -msh file                 read mesh (in msh format) from file\n"
  "  -restart                  resume processing from where it stopped\n"
  "  -solve 'Resolution'       same as -pre 'Resolution' -cal\n"
  "  -split                    save processing results in separate files\n"
  "  -res file(s)              load processing results from file(s)\n"
  "  -name string              use string as generic file name\n"
  "  -adapt file               read adaptation constraints from file\n"
  "  -order num                restrict maximum interpolation order\n"
#if _SPARSKIT
  "Linear solver options:\n"
  "  -solver file              specify parameter file (default: solver.par)\n"
  "  -'Parameter' num          override value of solver parameter 'Parameter'\n"
#endif
  "Output options:\n"
  "  -bin                      use binary format for output files\n"
  "  -log                      save processing history in log file\n"
  "  -socket string            communicate through socket string\n"
  "Other options:\n"
  "  -check                    interactive check of problem structure\n"
  "  -v num                    set verbosity level (default: 4)\n"
  "  -p num                    set progress indicator update (default: 10)\n"
  "  -version                  show version number\n"
  "  -info                     show detailed version information\n"
  "  -help                     show this message\n"
  ;

void Info (int level, char *arg0){
  
  switch(level){
  case 0 :
    fprintf(stderr, acronym, GETDP_VERSION);
    fprintf(stderr, copyright);
    fprintf(stderr, help, arg0);
    break;
  case 1:
    fprintf(stderr, "%g\n", GETDP_VERSION);
    break;
  case 2:
    fprintf(stderr, version, GETDP_VERSION);
    fprintf(stderr, os, GETDP_OS);
    fprintf(stderr, date, GETDP_DATE);
    fprintf(stderr, host, GETDP_HOST);
    fprintf(stderr, packager, GETDP_PACKAGER);
    fprintf(stderr, solver);
    fprintf(stderr, url);
    fprintf(stderr, email);
    break;
  }

  FinalizeAndExit();
}

/* ------------------------------------------------------------------------ */
/*  S i g n a l                                                             */
/* ------------------------------------------------------------------------ */

void Signal (int sig_num){

  /* It is VERY wrong to call stdio functions in a signal handler. But
     who cares? ;-) */

  if(sig_num == SIGINT){
    if(!InteractiveLevel){
      if(TreatmentStatus == _CAL && Flag_VERBOSE > 1){
	Msg(BIGINFO, "Switching to low verbosity mode"); 
	Flag_VERBOSE = 1;
      }
      else Msg(ERROR, "Interrupt (generated from terminal special character)"); 
    }
    else
      InteractiveInterrupt = 1;
    return;
  }

  switch (sig_num){
  case SIGSEGV : 
    Msg(BIGERROR, "Segmentation violation (invalid memory reference)");
    break;
  case SIGFPE : 
    Msg(BIGERROR, "Floating point exception (division by zero?)");
    break;
  default :
    Msg(ERROR, "Unknown signal");
    break;
  }
}


/* ------------------------------------------------------------------------ */
/*  M s g                                                                   */
/* ------------------------------------------------------------------------ */

void Get_GetDPContext(char *FileName, char *FileVersion, char *FileDate, 
		      char *FileAuthor, int *Line, char *FunctionName){
  char Dum[32] ;

  sscanf(GetDP_CurrentSourceFile[GetDP_CurrentStackIndex-1],
	 "$Id: %s %s %s %s %s", FileName, FileVersion, FileDate, Dum, FileAuthor);
  FileName[strlen(FileName)-2] = '\0' ;
  strcpy(FunctionName, GetDP_CurrentFunction[GetDP_CurrentStackIndex-1]);
  *Line = GetDP_CurrentSourceLine[GetDP_CurrentStackIndex-1];
}


void PrintMsg(FILE *stream, int level, int Verbosity, 
	      va_list args, char *fmt, int *abort) {

  int  verb, nl, gmshlevel;
  char *str, prefix[1000], sockmsg[1000];

#ifdef USE_DEBUG
  char FileName[256], FileVersion[32], FunctionName[256], FileAuthor[32], FileDate[32];
  int  Line ;
#endif
  
  switch(level){
  case CHECK     : verb = 0; nl = 0; str = NULL; break;
  case ERROR     : /* fall-through */
  case BIGERROR  : verb = 0; nl = 1; str = ERROR_STR; *abort = 1; break;
  case WARNING   : verb = 0; nl = 1; str = WARNING_STR; break;
  case OPERATION : verb = 2; nl = 1; str = OPERATION_STR; break;
  case LOADING   : verb = 2; nl = 1; str = LOADING_STR; break;
  case INFO      : verb = 3; nl = 1; str = INFO_STR; break;
  case INFO1     : verb = 3; nl = 0; str = INFO_STR; break;
  case INFO2     : verb = 3; nl = 0; str = NULL; break;
  case INFO3     : verb = 3; nl = 1; str = NULL; break;
  case BIGINFO   : verb = 1; nl = 1; str = BIGINFO_STR; break;
  case DEBUG     : verb =99; nl = 0; str = NULL; break;
  case DEBUG2    : verb =98; nl = 0; str = NULL; break;
  case SPARSKIT  : verb = 3; nl = 0; str = SPARSKIT_STR; break;
  case PETSC     : verb = 3; nl = 1; str = PETSC_STR; break;
  case ITER      : verb = 4; nl = 0; str = NULL; break;
  case DIRECT    : /* fall-through */
  default        : verb = 1; nl = 1; str = NULL; break;
  }

  if(Verbosity >= verb || stream == LogStream){

    if(Flag_SOCKET > 0 && stream != LogStream){
      if(str) strcpy(prefix, str); else strcpy(prefix, "");
      vsprintf(sockmsg, fmt, args);
      if(!nl) sockmsg[strlen(sockmsg)-1] = '\0' ;
      strcat(prefix, sockmsg);
      switch(level){
      case ERROR    : /* fall-through */
      case BIGERROR : gmshlevel = GMSH_CLIENT_ERROR; break;
      case WARNING  : gmshlevel = GMSH_CLIENT_WARNING; break;
      default       : gmshlevel = GMSH_CLIENT_INFO; break;
      }
      Gmsh_SendString(Flag_SOCKET, gmshlevel, prefix);
    }
    else{
      if(str) fprintf(stream, str); 
      vfprintf(stream, fmt, args); 
      if(nl) fprintf(stream, "\n");
      if(level == BIGERROR){
#ifdef USE_DEBUG
	Get_GetDPContext(FileName, FileVersion, FileDate, FileAuthor, 
			 &Line, FunctionName);
	fprintf(stream, WHITE_STR); 
	fprintf(stream, "File '%s' (V%s by %s on %s)\n", 
		FileName, FileVersion, FileAuthor, FileDate);
	fprintf(stream, WHITE_STR); 
	fprintf(stream, "Function '%s' (L%d)\n", FunctionName, Line);
#endif
	fprintf(stream, WHITE_STR "------------------------------------------------------\n");
	fprintf(stream, WHITE_STR "You have discovered a bug in GetDP! You may report it\n");
	fprintf(stream, WHITE_STR "by e-mail (together with any helpful data permitting to\n");
	fprintf(stream, WHITE_STR "reproduce it) to <getdp@geuz.org>\n");
      }
    }

  }

}

void GetResources(long *s, long *us, long *mem){
#ifndef MSDOS
  static struct rusage r;

  getrusage(RUSAGE_SELF,&r);
  *s   = (long)r.ru_utime.tv_sec ;
  *us  = (long)r.ru_utime.tv_usec ;
  *mem = (long)r.ru_maxrss ;
#else
  *s = *us = *mem = 0 ;
#endif
}

void PrintResources(FILE *stream, char *fmt, long s, long us, long mem){

#ifndef MSDOS
  char sockmsg[1000];
  if(Flag_SOCKET > 0){
    sprintf(sockmsg, RESOURCES_STR "%scpu %ld.%ld s / mem %ld kb\n", fmt, s, us, mem);
    Gmsh_SendString(Flag_SOCKET, GMSH_CLIENT_INFO, sockmsg);
  }
  else{
    fprintf(stream, RESOURCES_STR) ;
    fprintf(stream, "%scpu %ld.%ld s / mem %ld kb\n", fmt, s, us, mem);
  }
#endif
}


void Msg(int level, char *fmt, ...){
  va_list  args;
  int      abort = 0;
  long     s, us, mem ;

  if(Current.RankCpu && level != PETSC) return ;

  if(level == RESOURCES){
    if(Flag_LOG || Flag_VERBOSE > 3) GetResources(&s, &us, &mem) ;
    if(Flag_VERBOSE > 3) PrintResources(stderr, fmt, s, us, mem) ;
    if(Flag_LOG) PrintResources(LogStream, fmt, s, us, mem) ;
  }
  else if(level == SUMMARY){
    if(Flag_LOG || Flag_VERBOSE > 0) GetResources(&s, &us, &mem) ;
    if(Flag_VERBOSE > 0) PrintResources(stderr, fmt, s, us, mem) ;
    if(Flag_LOG) PrintResources(LogStream, fmt, s, us, mem) ;
  }
  else{
    va_start (args, fmt);
    PrintMsg(stderr,level, Flag_VERBOSE, args, fmt, &abort) ;
    if(Flag_LOG) {
      PrintMsg(LogStream, level, Flag_VERBOSE, args, fmt, &abort) ;
      fflush(LogStream);
    }
    va_end (args);
    if(abort){
      Debug();
      FinalizeAndExit();
    }
  }
}


/* ------------------------------------------------------------------------ */
/*  P r o g r e s s                                                         */
/* ------------------------------------------------------------------------ */

void Progress(int current, int final, char *label){
  char sockmsg[100];
  static int ProgressIndex ;

  if(Current.RankCpu || !Flag_VERBOSE || !Flag_PROGRESS) return ;

  if(!current){
    ProgressIndex = 0;
    if(100/(double)final > Flag_PROGRESS) Flag_PROGRESS = 100/final+1 ;
  }
  if(100*current/(double)final >= ProgressIndex){
    if(Flag_SOCKET > 0){
      sprintf(sockmsg, "(%s%d %%)", label, ProgressIndex) ;
      Gmsh_SendString(Flag_SOCKET, GMSH_CLIENT_PROGRESS, sockmsg);
    }
    else
      fprintf(stderr, "(%s%d %%)     \r", label, ProgressIndex) ;
    ProgressIndex += Flag_PROGRESS ;
  }
  if(current >= final-1){
    if(Flag_SOCKET < 0) fprintf(stderr, "                        \r") ;
  }

}


/* ------------------------------------------------------------------------ */
/*  D e b u g                                                               */
/* ------------------------------------------------------------------------ */

void Debug(void){

}
