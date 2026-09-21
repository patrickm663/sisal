/**************************************************************************/
/* FILE   **************         p-error.c         ************************/
/**************************************************************************/
/* Author: Dave Cann                                                      */
/* Update: Patrick Miller -- Ansi support (Dec 2000)                      */
/* Copyright (C) University of California Regents                         */
/**************************************************************************/
/*
 * $Log:
 */
/**************************************************************************/

#include "sisalrt.h"


static LOCK_TYPE *ErrorLock = (LOCK_TYPE*) NULL;


void InitErrorSystem(void)
{
  ErrorLock = (LOCK_TYPE*) SharedMalloc( SIZEOF(LOCK_TYPE) );
  MY_SINIT_LOCK(ErrorLock);
}


int SisalError(char *Message1, char *Message2)
{
  if ( ErrorLock != (LOCK_TYPE*) NULL )
    MY_SLOCK( ErrorLock );

  FPRINTF( stderr, "\nERROR: (%s) %s\n", Message1, Message2 );

  if ( UsingSdbx )
    SdbxMonitor( SDBX_ERR );

  AbortParallel();

  /* SisalError is declared noreturn (PROTO_NORET), but AbortParallel is not
     guaranteed to end the process on every model -- several of them raise a
     signal with kill(), which returns.  Make good on the declaration rather
     than falling off the end of a noreturn function. */
  exit( 1 );
}
