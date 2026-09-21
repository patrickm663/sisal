#ifndef PARALLELISM_H
#define PARALLELISM_H

/**************************************************************************/
/* FILE   **************       parallelism.h       ************************/
/**************************************************************************/
/* Author: Dave Cann                                                      */
/* Update: Patrick Miller -- Ansi support (Dec 2000)                      */
/* Copyright (C) University of California Regents                         */
/**************************************************************************/
/**************************************************************************/

/************************************************************************\
 * Alliant
\************************************************************************/

void ReleaseSharedMemory(void)
{
  free( SharedBase );
}

void AcquireSharedMemory(int NumBytes)
{
  SharedSize = NumBytes + 100000;

  SharedBase = SharedMemory = malloc( SharedSize-40 );

  if ( SharedMemory == (char *) NULL )
    SisalError( "AcquireSharedMemory", "malloc FAILED" );

  SharedMemory = ALIGN(char*,SharedMemory);
}

void StartWorkers(void)
{
  EnterWorker( 0 );
}

void StopWorkers(void)
{
  *SisalShutDown = TRUE;
  LeaveWorker();
}

void AbortParallel(void) 
{ 
  (void)kill( 0, SIGKILL ); 
}

/*
 * $Log:
 */

#endif
