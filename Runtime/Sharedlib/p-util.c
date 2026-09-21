/**************************************************************************/
/* FILE   **************          p-util.c         ************************/
/**************************************************************************/
/* Author: Dave Cann                                                       */
/* Update: Patrick Miller -- Ansi support (Dec 2000)                      */
/* Copyright (C) University of California Regents                         */
/**************************************************************************/
/*
 * $Log:
 */
/**************************************************************************/

#include "sisalrt.h"

/************************************************************************\
 * p-util.c - SISAL runtime system array memory utilities
\************************************************************************/

PHYSP DoPhysExpand(POINTER Array, int CompSize)
{
  PHYSP  Phys;
  char  *Dest;
  char  *Source;
  PHYSP  APhys;
  APhys = ((ARRAYP)Array)->Phys;

  Phys = (PHYSP) Alloc( SIZE_OF(PHYS) +
         ((APhys->Size + 1 + (APhys->ExpHistory * ArrayExpansion)) * CompSize));

  MY_INIT_LOCK( &Phys->Mutex );

  Phys->Base       = (POINTER) ALIGNED_INC(PHYS,Phys);
  Phys->RefCount   = 1;
  Phys->Size       = APhys->Size;
  Phys->ExpHistory = APhys->ExpHistory + 1;
  Phys->Free       = (APhys->ExpHistory * ArrayExpansion) + 1;

  Source  = (char*) APhys->Base;
  Dest    = (char*) Phys->Base;

  if ( Phys->Size != 0 ) /* CRAY BUG WORK-AROUND!!! */
    memmove( Dest, Source, Phys->Size * CompSize );

  DeAlloc( (POINTER) APhys );
  ((ARRAYP)Array)->Base = Phys->Base - (((ARRAYP)Array)->LoBound * CompSize);

  return( ((ARRAYP)Array)->Phys = Phys );
}


POINTER ArrayDuplicate(POINTER OldArray, int CompSize)
{
  ARRAYP Array;
  PHYSP  Phys;

  Array = (ARRAYP) Alloc( SIZEOF(ARRAY) );

  Array->LoBound  = ((ARRAYP)OldArray)->LoBound;
  Array->Size     = ((ARRAYP)OldArray)->Size;
  Array->RefCount = 1;

  MY_INIT_LOCK( &Array->Mutex );

  Phys = (PHYSP) Alloc( SIZE_OF(PHYS) + (Array->Size * CompSize) );

  MY_INIT_LOCK( &Phys->Mutex );

  Phys->Base       = (POINTER) ALIGNED_INC(PHYS,Phys);
  Phys->RefCount   = 1;
  Phys->Size       = Array->Size;
  Phys->ExpHistory = 1;
  Phys->Free       = 0;

  Array->Base = Phys->Base - (Array->LoBound * CompSize);
  Array->Phys = Phys;

  return( (POINTER) Array );
}


POINTER ArrayDuplicatePlus(POINTER OldArray, int CompSize)
{
  ARRAYP Array;
  PHYSP  Phys;

  Array = (ARRAYP) Alloc( SIZEOF(ARRAY) );

  Array->LoBound  = ((ARRAYP)OldArray)->LoBound;
  Array->Size     = ((ARRAYP)OldArray)->Size;
  Array->RefCount = 1;

  MY_INIT_LOCK( &Array->Mutex );

  Phys = (PHYSP) Alloc( SIZE_OF(PHYS) +
                        ((Array->Size + ArrayExpansion + 1) * CompSize) );

  MY_INIT_LOCK( &Phys->Mutex );

  Phys->Base       = (POINTER) ALIGNED_INC(PHYS,Phys);
  Phys->RefCount   = 1;
  Phys->Size       = Array->Size;
  Phys->ExpHistory = 1;
  Phys->Free       = ArrayExpansion + 1;

  Array->Base = Phys->Base - (Array->LoBound * CompSize);
  Array->Phys = Phys;

  return( (POINTER) Array );
}
