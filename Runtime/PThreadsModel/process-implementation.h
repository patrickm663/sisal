#ifndef PROCESS_IMPLEMENTATION_H
#define PROCESS_IMPLEMENTATION_H

/**************************************************************************/
/* FILE   **************  process-implementation.h ************************/
/**************************************************************************/
/* Author: Dave Raymond                                                   */
/* Update: Patrick Miller -- Ansi support (Dec 2000)                      */
/* Copyright (C) University of California Regents                         */
/**************************************************************************/
/*
 * $Log:
 */
/**************************************************************************/

/* PThreadsModel */

extern int sisalGetID(void);
#define GetProcId  sisalGetID()
#define GETPROCID(x) x = GetProcId
extern BARRIER_TYPE *MyInitBarrier(int);
extern void sisalYield(void);
#define YIELD_IF_REQUIRED() sisalYield()
#endif
