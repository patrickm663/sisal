/**************************************************************************/
/* FILE   **************        p-pointer.c        ************************/
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


void FreePointerSwapFramework(POINTER BPtr, POINTER FramePtr)
{
  PSBUFFERP Buffer;

  Buffer = (PSBUFFERP) BPtr;

  if ( Buffer->InfoTop <= 0 )
    return;

  if ( FramePtr != (POINTER) NULL )
    DeAlloc( FramePtr );
}


static void FreeStorage(int Level, int *Info, POINTER FramePtr)
{
  ARRAYP Array;
  int    Index;

  if ( Level == 1 ) {
    for ( Index = 0; Index < *Info; Index++ ) {
      Array = ((ARRAYP*)FramePtr)[Index];
      DeAlloc( (POINTER) (Array->Phys) );
      DeAlloc( (POINTER) Array );
      }

    return;
    }

  if ( *Info == 0 )
    return;

  for ( Index = 0; Index < *Info; Index++ )
    FreeStorage( Level-1, Info+1, ((POINTER*)FramePtr)[Index] );
}


void FreePointerSwapStorage(POINTER BPtr, POINTER FramePtr)
{
  PSBUFFERP Buffer;

  Buffer = (PSBUFFERP) BPtr;

  if ( Buffer->InfoTop <= 0 ) {
    DeAlloc( (POINTER) ((ARRAYP)FramePtr)->Phys );
    DeAlloc( (POINTER) FramePtr );
    return;
    }

  if ( Buffer->Info[2] != 0 ) {
    FreeStorage( Buffer->InfoTop, &(Buffer->Info[2]), FramePtr );
    DeAlloc( FramePtr );
    }
}


static void InitializeStorage(int Level,
                              int *Info,
                              int *ArrayInfo,
                              int CompSize,
                              POINTER *Base,
                              POINTER **Next)
{
  ARRAYP Array;
  PHYSP  Phys;
  int    Index;

  if ( Level == 1 ) {
    for ( Index = 0; Index < *Info; Index++ ) {
      Array = (ARRAYP) Alloc( SIZEOF(ARRAY) );

      Array->LoBound  = ArrayInfo[1];
      Array->Size     = ArrayInfo[0];
      Array->RefCount = 1;

      MY_INIT_LOCK( &Array->Mutex );

      Phys = (PHYSP) Alloc( SIZE_OF(PHYS) + (Array->Size * CompSize) );

      MY_INIT_LOCK( &Phys->Mutex );

      Phys->Base       = (POINTER) ALIGNED_INC(PHYS,Phys);
      Phys->RefCount   = 1;
      Phys->Size       = Array->Size;
      Phys->ExpHistory = 1;
      Phys->Free       = 0;
      Phys->Dope       = (POINTER) Array;

      Array->Base = Phys->Base - (Array->LoBound * CompSize);
      Array->Phys = Phys;

      Base[Index] = (POINTER) Array;
      }

    return;
    }

  if ( *Info == 0 )
    return;

  for ( Index = 0; Index < *Info; Index++ ) {
    Base[Index] = (POINTER) *Next;
    InitializeStorage( Level-1, Info+1, ArrayInfo, CompSize, *Next, Next );
    *Next += *(Info+1);
    }
}


POINTER AllocPointerSwapStorage(POINTER BPtr, int CompSize)
{
  PSBUFFERP  Buffer;
  ARRAYP     Array;
  PHYSP      Phys;
  int        FrameworkSize;
  POINTER   *Base;
  int       *Info;
           POINTER   *Next;

  Buffer = (PSBUFFERP) BPtr;

  if ( Buffer->InfoTop <= 0 ) {
    Array = (ARRAYP) Alloc( SIZEOF(ARRAY) );

    Array->LoBound  = Buffer->Info[1];
    Array->Size     = Buffer->Info[0];
    Array->RefCount = 1;

    MY_INIT_LOCK( &Array->Mutex );

    Phys = (PHYSP) Alloc( SIZE_OF(PHYS) + (Array->Size * CompSize) );

    MY_INIT_LOCK( &Phys->Mutex );

    Phys->Base       = (POINTER) ALIGNED_INC(PHYS,Phys);
    Phys->RefCount   = 1;
    Phys->Size       = Array->Size;
    Phys->ExpHistory = 1;
    Phys->Free       = 0;
    Phys->Dope       = (POINTER) Array;

    Array->Base = Phys->Base - (Array->LoBound * CompSize);
    Array->Phys = Phys;

    return( (POINTER) Array );
    }

  Info = Buffer->Info;

  FrameworkSize = Info[2]*(1+(Info[3]*(1+(Info[4]*(1+Info[5])))));

  if ( FrameworkSize == 0 )
    return( (POINTER) NULL );

  Base = (POINTER*) Alloc( FrameworkSize * SIZEOF(POINTER) );
  Next = Base + Info[2];

  InitializeStorage( Buffer->InfoTop, &(Info[2]), Info, CompSize, Base, &Next );

  return( (POINTER) Base );
}


/* SKI */
static void SkiInitializeStorage(int Nv,
                                 int Level,
                                 int *Info,
                                 int *ArrayInfo,
                                 int CompSize,
                                 POINTER *Base,
                                 POINTER **Next)
{
  ARRAYP Array;
  PHYSP  Phys;
  int    Index;

  if ( Level == 1 ) {
    for ( Index = 0; Index < *Info; Index++ ) {
      Array = (ARRAYP) Alloc( SIZEOF(ARRAY) );

      Array->LoBound  = ArrayInfo[1];
      Array->Size     = ArrayInfo[0];
      Array->RefCount = 1;

      MY_INIT_LOCK( &Array->Mutex );

      Phys = (PHYSP) Alloc( SIZE_OF(PHYS) + ((Nv+Array->Size) * CompSize) );

      MY_INIT_LOCK( &Phys->Mutex );

      Phys->Base       = (POINTER) ALIGNED_INC(PHYS,Phys);
      Phys->Base       = Phys->Base+(Nv*CompSize);
      Phys->RefCount   = 1;
      Phys->Size       = Array->Size;
      Phys->ExpHistory = 1;
      Phys->Free       = 0;
      Phys->Dope       = (POINTER) Array;

      Array->Base = Phys->Base - (Array->LoBound * CompSize);
      Array->Phys = Phys;

      Base[Index] = (POINTER) Array;
      }

    return;
    }

  if ( *Info == 0 )
    return;

  for ( Index = 0; Index < *Info; Index++ ) {
    Base[Index] = (POINTER) *Next;
    SkiInitializeStorage( Nv, Level-1, Info+1, ArrayInfo, 
                          CompSize, *Next, Next );
    *Next += *(Info+1);
    }
}


POINTER SkiAllocPointerSwapStorage(int Nv, POINTER BPtr, int CompSize)
{
  PSBUFFERP  Buffer;
  ARRAYP     Array;
  PHYSP      Phys;
  int        FrameworkSize;
  POINTER   *Base;
  int       *Info;
           POINTER   *Next;

  Buffer = (PSBUFFERP) BPtr;

  if ( Buffer->InfoTop <= 0 ) {
    Array = (ARRAYP) Alloc( SIZEOF(ARRAY) );

    Array->LoBound  = Buffer->Info[1];
    Array->Size     = Buffer->Info[0];
    Array->RefCount = 1;

    MY_INIT_LOCK( &Array->Mutex );

    Phys = (PHYSP) Alloc( SIZE_OF(PHYS) + ((Nv+Array->Size) * CompSize) );

    MY_INIT_LOCK( &Phys->Mutex );

    Phys->Base       = (POINTER) ALIGNED_INC(PHYS,Phys);
    Phys->Base       = Phys->Base+(Nv*CompSize);
    Phys->RefCount   = 1;
    Phys->Size       = Array->Size;
    Phys->ExpHistory = 1;
    Phys->Free       = 0;
    Phys->Dope       = (POINTER) Array;

    Array->Base = Phys->Base - (Array->LoBound * CompSize);
    Array->Phys = Phys;

    return( (POINTER) Array );
    }

  Info = Buffer->Info;

  FrameworkSize = Info[2]*(1+(Info[3]*(1+(Info[4]*(1+Info[5])))));

  if ( FrameworkSize == 0 )
    return( (POINTER) NULL );

  Base = (POINTER*) Alloc( FrameworkSize * SIZEOF(POINTER) );
  Next = Base + Info[2];

  SkiInitializeStorage( Nv, Buffer->InfoTop, &(Info[2]), Info, 
                        CompSize, Base, &Next );

  return( (POINTER) Base );
}
/* SKI */
