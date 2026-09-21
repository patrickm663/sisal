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
 * Sequent
\************************************************************************/

extern char *shmalloc(int);

void AcquireSharedMemory(int NumBytes)
{
  SharedSize = NumBytes + 100000;

  SharedBase = SharedMemory = shmalloc( SharedSize-40 );

  if ( SharedMemory == (char *) NULL )
    SisalError( "AcquireSharedMemory", "shmalloc FAILED" );

  SharedMemory = ALIGN(char*,SharedMemory);
}

void ReleaseSharedMemory(void)
{
  shfree( SharedBase );
}

#ifdef GANGD
void StartWorkers(void)
{
  int pID;

  begin_parallel( NumWorkers );

  GETPROCID(pID);

#if defined(DIST_DSA)
        if(pID != 0)
                InitDsa(DsaSize/(2*(NumWorkers - 1)), XftThreshold);
#endif

  EnterWorker( pID );

  if ( pID != 0 ) {
    LeaveWorker();
    end_parallel();
    }
}

void StopWorkers(void)
{
  *SisalShutDown = TRUE;
  LeaveWorker();
  end_parallel();
}

void AbortParallel(void) 
{ 
  abort_parallel(); 
}
#else
int p_procnum = 0;

void StartWorkers(void)
{
  int NumProcs = NumWorkers;

  while( --NumProcs > 0 )
    if ( fork() == 0 )
      break;

#if defined(DIST_DSA)
        if(NumProcs != 0)
                InitDsa(DsaSize/(2*(NumWorkers - 1)), XftThreshold);
#endif
  EnterWorker( p_procnum = NumProcs );

  if ( NumProcs != 0 ) {
    LeaveWorker();
    exit( 0 );
    }
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

#endif
