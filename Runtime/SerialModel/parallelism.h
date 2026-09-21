#ifndef PARALLELISM_H
#define PARALLELISM_H

/**************************************************************************/
/* FILE   **************       parallelism.h       ************************/
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
 * Sequential UNIX
\************************************************************************/

int p_procnum = 0;

void ReleaseSharedMemory(void)
{
  free( SharedBase );
}

void AcquireSharedMemory(int NumBytes)
{
  SharedSize = NumBytes + 100000;

  SharedBase = SharedMemory = (char *)malloc( SharedSize-40 );

  if ( SharedMemory == (char *) NULL )
    SisalError( "AcquireSharedMemory", "malloc FAILED" );
}

void StartWorkers(void) 
{
#if defined(DIST_DSA)
        if(p_procnum != 0)
                InitDsa(DsaSize/(2*(NumWorkers - 1)), XftThreshold);
#endif

  EnterWorker( p_procnum );
}

void StopWorkers(void)
{
  *SisalShutDown = TRUE;
  LeaveWorker();
}

void AbortParallel(void) 
{ 
  exit( 1 ); 
}

#endif
