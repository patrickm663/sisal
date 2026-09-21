/**************************************************************************/
/* FILE   **************           srt0.c          ************************/
/**************************************************************************/
/* Author: Dave Cann                                                      */
/* Update: Patrick Miller -- Ansi support (Dec 2000)                      */
/* Copyright (C) University of California Regents                         */
/**************************************************************************/
/*
 * $Log:
 */
/**************************************************************************/

/************************************************************************\
 * p-srt0.c - SISAL runtime system main
\************************************************************************/

#include "sisalrt.h"

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

static char RCSVERSION[] = "$State$";
static char RCS_REVISION[SIZEOF(RCSVERSION)] = "?.?";

/************************************************************************\
 * MAIN
\************************************************************************/

int main( int argc, char **argv )
{
  int           i;

#ifdef _WIN32
  /* Every SISAL program's whole output goes through FibreOutFd, and every
     FIBRE-format argument comes in through FibreInFd -- both default to
     stdin/stdout below, which Windows opens in text mode by default,
     translating LF to CRLF on write (and, same as _READ/_PIPE/_STDIN,
     mangling anything containing CR or 0x1A on read). Same fix as those:
     ask for binary explicitly, since nothing here ever wants translation. */
  _setmode( _fileno( stdin ), _O_BINARY );
  _setmode( _fileno( stdout ), _O_BINARY );
#endif

  FibreInFd = stdin;
  FibreOutFd = stdout;
  PerfFd = stderr;

  ParseCommandLine( argc, argv );
  InitSisalRunTime();

  /* Form the version number from the RCS checkout revision number */
  if ( RCSVERSION[6] == ':' ) {
    strcpy(RCS_REVISION,RCSVERSION+8);
    for(i=0; RCS_REVISION[i]; i++) {
      if ( RCS_REVISION[i] == '$' ) {
        RCS_REVISION[i] = '\0';
        break;
      }
    }
  }

  if (!NoFibreOutput) FPRINTF( stderr, "%s %s\n", SISAL_BANNER, RCS_REVISION );

  SisalMainArgs = ReadFibreInputs();

  StartWorkers();
  SisalMain( SisalMainArgs );
  StopWorkers();

  if ( !NoFibreOutput ) {
    WriteFibreOutputs( SisalMainArgs );
    fputc( '\n', FibreOutFd );
  }

  if ( GatherPerfInfo )
    DumpRunTimeInfo();

  ShutDownDsa();

  return 0;
}
