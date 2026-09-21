#ifndef LOCKS_H
#define LOCKS_H

/**************************************************************************/
/* FILE   **************          locks.h          ************************/
/**************************************************************************/
/* Author: Dave Cann                                                      */
/* Update: Patrick Miller -- Ansi support (Dec 2000)                      */
/* Copyright (C) University of California Regents                         */
/**************************************************************************/
/*
 * $Log:
 */
/**************************************************************************/
/* locks.h - SISAL runtime system machine-specific lock definitions       */
/**************************************************************************/

#include "lock-implementation.h"

extern void MyInitLock(void);
extern void MyLock(void);
extern void MyUnlock(void);
extern void MyBarrier(BARRIER_TYPE *);

#endif
