/**************************************************************************/
/* FILE   **************         vectorIO.c        ************************/
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

int     FibreStrings = TRUE;
int     sisal_file_io = 0;

#define GenericReadArray(scalartype,reader,term)\
{ \
  POINTER       val0; \
  scalartype val6; \
  int lob; \
 \
  FibreParse( INT_ ); \
  lob = FibreInt; \
  GET_LOOKAHEAD; \
  if ( LookAheadToken == COMMA_ ) { \
    FibreParse( COMMA_ ); \
    FibreParse( INT_ ); \
    OptABld( val0, lob, 1, FibreInt, scalartype ); \
    ((ARRAYP)val0)->Phys->Size = 0; \
  } else { \
    ABld( val0, lob, 1 ); \
  } \
  FibreParse( COLON_ ); \
 \
  GET_LOOKAHEAD; \
  while ( LookAheadToken != term ) { \
    if ( LookAheadToken == SEMI_COLON_ ) \
      FibreError( "REPETITION FACILITY NOT IMPLEMENTED" ); \
    reader( val6 ); \
    AGather( val0, val6, scalartype ); \
    GET_LOOKAHEAD; \
  } \
  return val0; \
}

/* ------------------------------------------------------------ */
/* B O O L E A N                                                */
/* ------------------------------------------------------------ */
POINTER ReadBoolVector(void)
{
  switch ( FibreParse( ANY_ ) ) {
  case ARRAYB_:
  GenericReadArray(char,ReadBool,ARRAYE_);

  case STREAMB_:
  GenericReadArray(char,ReadBool,STREAME_);

  case STRING_START_:
    FibreError( "STRING DELIMITER WAS NOT EXPECTED" );
    break;

  default:
    FibreError( "ARRAY DELIMITER EXPECTED" );
  }
  return (POINTER)(NULL);
}
void WriteBoolVector(POINTER val)
{
  POINTER Base2;
  int     HiBound;
  int     Lo2;
  ARRAYP arr = (ARRAYP) val;
  int saveIndent;

  FIBRE_BUF_BEGIN();

  PrintIndent;
  Lo2 = arr->LoBound;
  fprintf( FibreOutFd, "[ %d,%d:", Lo2, Lo2+(arr->Size)-1 );
  saveIndent = Indent;
#ifdef VERBOSE
  fprintf( FibreOutFd, " # DRC=%d PRC=%d\n", arr->RefCount, arr->Phys->RefCount );
  Indent++;
#else
  Indent = 0;
  fprintf( FibreOutFd, " " );
#endif

  Indent++;
  Base2 = arr->Base;
  HiBound = Lo2 + arr->Size - 1;
  for ( ; Lo2 <= HiBound; Lo2++ ) {
    WriteBool( (((char*)Base2)[Lo2]) );
  }

  Indent = saveIndent;
#ifdef VERBOSE
  PrintIndent;
#endif
  fprintf( FibreOutFd, "]\n" );

  FIBRE_BUF_END();
}


/* ------------------------------------------------------------ */
/* C H A R A C T E R                                            */
/* ------------------------------------------------------------ */
POINTER ReadCharVector(void)
{
  POINTER       val0;

  if ( sisal_file_io ) {
    int c;
    ABld(val0,1,1);
    while( (c = fgetc(stdin)) != EOF ) {
      AGather( val0, c, char );
    }
    return val0;
  }

  switch ( FibreParse( ANY_ ) ) {
  case ARRAYB_:
    GenericReadArray(char,ReadChar,ARRAYE_);

  case STREAMB_:
    GenericReadArray(char,ReadChar,STREAME_);

  case STRING_START_:
    ABld( val0, 1, 1 );
    while ( FibreParse( ANY_ ) != STRING_TERM_ ) {
      AGather( val0, FibreChar, char );
    }
    return val0;

  default:
    FibreError( "ARRAY DELIMITER EXPECTED" );
  }
  return (POINTER)(NULL);
}

void WriteCharVector(POINTER val)
{
  POINTER Base2;
  int     HiBound;
  int     Lo2;
  ARRAYP  arr = (ARRAYP) val;
  int              c;

  Lo2 = arr->LoBound;
  Base2 = arr->Base;
  HiBound = Lo2 + arr->Size - 1;

  if ( sisal_file_io ) {
    for (       ; Lo2 <= HiBound; Lo2++ ) {
      c = *((char*)Base2+Lo2);
      fputc(c,FibreOutFd);
    }
    fflush(FibreOutFd);
    return;
  }

  FIBRE_BUF_BEGIN();

  PrintIndent;
  if ( FibreStrings && Lo2 == 1 ) {
    fputc( '"', FibreOutFd );
    for (       ; Lo2 <= HiBound; Lo2++ ) {
      c = *((char*)Base2+Lo2);
      if ( isascii(c) && isprint(c) ) {
        if ( c == '"' ) {
          fputs( "\\\"",FibreOutFd);
        } else if ( c == '\\' ) {
          fputs( "\\\\",FibreOutFd);
        } else { 
          fputc( c, FibreOutFd );
        }
      } else {
        switch ( c ) {
        case '\b': fputs( "\\b", FibreOutFd ); break;
        case '\n': fputs( "\\n", FibreOutFd ); break;
        case '\f': fputs( "\\f", FibreOutFd ); break;
        case '\r': fputs( "\\r", FibreOutFd ); break;
        case '\t': fputs( "\\t", FibreOutFd ); break;
        default:
          fprintf( FibreOutFd,"\\%03o", c & 0xff);
        }
      }
    }
    fputc( '"', FibreOutFd );
  } else {
    fprintf( FibreOutFd, "[ %d,%d:", Lo2, Lo2+(arr->Size)-1 );
#ifdef VERBOSE
    fprintf( FibreOutFd, " # DRC=%d PRC=%d\n", arr->RefCount, arr->Phys->RefCount );
#else
    fprintf( FibreOutFd, " " );
#endif

    Indent++;
    for (       ; Lo2 <= HiBound; Lo2++ ) {
      WriteChar( *((char*)Base2+Lo2) );
    }

    Indent--;
    PrintIndent;
    fputc( ']', FibreOutFd );
  }
  fputc( '\n', FibreOutFd );

  FIBRE_BUF_END();
}

/* ------------------------------------------------------------ */
/*  D O U B L E                                                 */
/* ------------------------------------------------------------ */
POINTER ReadDoubleVector(void)
{
  switch ( FibreParse( ANY_ ) ) {
  case ARRAYB_:
    GenericReadArray(double,ReadDbl,ARRAYE_);

  case STREAMB_:
    GenericReadArray(double,ReadDbl,STREAME_);

  case STRING_START_:
    FibreError( "STRING DELIMITER WAS NOT EXPECTED" );
    break;

  default:
    FibreError( "ARRAY DELIMITER EXPECTED" );
  }
  return (POINTER)(NULL);
}

void WriteDoubleVector(POINTER val)
{
  POINTER Base2;
  int     HiBound;
  int     Lo2;
  ARRAYP arr = (ARRAYP) val;
  int saveIndent;

  FIBRE_BUF_BEGIN();

  PrintIndent;
  Lo2 = arr->LoBound;
  fprintf( FibreOutFd, "[ %d,%d:", Lo2, Lo2+(arr->Size)-1 );
  saveIndent = Indent;
#ifdef VERBOSE
  fprintf( FibreOutFd, " # DRC=%d PRC=%d\n", arr->RefCount, arr->Phys->RefCount );
  Indent++;
#else
  Indent = 0;
  fprintf( FibreOutFd, " " );
#endif

  Indent++;
  Base2 = arr->Base;
  HiBound = Lo2 + arr->Size - 1;
  for ( ; Lo2 <= HiBound; Lo2++ ) {
    WriteDbl( (((double*)Base2)[Lo2]) );
  }

  Indent = saveIndent;
#ifdef VERBOSE
  PrintIndent;
#endif
  fprintf( FibreOutFd, "]\n" );

  FIBRE_BUF_END();
}

/* ------------------------------------------------------------ */
/* I N T E G E R                                                */
/* ------------------------------------------------------------ */
POINTER ReadIntegerVector(void)
{
  switch ( FibreParse( ANY_ ) ) {
  case ARRAYB_:
    GenericReadArray(int,ReadInt,ARRAYE_);

  case STREAMB_:
    GenericReadArray(int,ReadInt,STREAME_);

  case STRING_START_:
    FibreError( "STRING DELIMITER WAS NOT EXPECTED" );
    break;

  default:
    FibreError( "ARRAY DELIMITER EXPECTED" );
  }
  return (POINTER)(NULL);
}

void WriteIntegerVector(POINTER val)
{
    POINTER Base2;
    int     HiBound;
    int     Lo2;
    ARRAYP arr = (ARRAYP) val;
    int saveIndent;

    FIBRE_BUF_BEGIN();

    PrintIndent;
    Lo2 = arr->LoBound;
    fprintf( FibreOutFd, "[ %d,%d:", Lo2, Lo2+(arr->Size)-1 );
    saveIndent = Indent;
#ifdef VERBOSE
    fprintf( FibreOutFd, " # DRC=%d PRC=%d\n", arr->RefCount, arr->Phys->RefCount );
    Indent++;
#else
    Indent = 0;
    fprintf( FibreOutFd, " " );
#endif

    Base2 = arr->Base;
    HiBound = Lo2 + arr->Size - 1;
    for ( ; Lo2 <= HiBound; Lo2++ ) {
      WriteInt( (((int*)Base2)[Lo2]) );
      }

    Indent = saveIndent;
#ifdef VERBOSE
    PrintIndent;
#endif
    fprintf( FibreOutFd, "]\n" );

    FIBRE_BUF_END();
}

/* ------------------------------------------------------------ */
/* N U L L                                                      */
/* ------------------------------------------------------------ */
POINTER ReadNullVector(void)
{
  switch ( FibreParse( ANY_ ) ) {
  case ARRAYB_:
    GenericReadArray(int,ReadNil,ARRAYE_);

  case STREAMB_:
    GenericReadArray(int,ReadNil,STREAME_);

  case STRING_START_:
    FibreError( "STRING DELIMITER WAS NOT EXPECTED" );
    break;

  default:
    FibreError( "ARRAY DELIMITER EXPECTED" );
  }
  return (POINTER)(NULL);
}

void WriteNullVector(POINTER val)
{
  POINTER Base2;
  int     HiBound;
  int     Lo2;
  ARRAYP arr = (ARRAYP) val;
  int saveIndent;

  FIBRE_BUF_BEGIN();

  PrintIndent;
  Lo2 = arr->LoBound;
  fprintf( FibreOutFd, "[ %d,%d:", Lo2, Lo2+(arr->Size)-1 );
  saveIndent = Indent;
#ifdef VERBOSE
  fprintf( FibreOutFd, " # DRC=%d PRC=%d\n", arr->RefCount, arr->Phys->RefCount );
  Indent++;
#else
  Indent = 0;
  fprintf( FibreOutFd, " " );
#endif

  Indent++;
  Base2 = arr->Base;
  HiBound = Lo2 + arr->Size - 1;
  for ( ; Lo2 <= HiBound; Lo2++ ) {
    WriteNil( (((char*)Base2)[Lo2]) );
  }

  Indent = saveIndent;
#ifdef VERBOSE
  PrintIndent;
#endif
  fprintf( FibreOutFd, "]\n" );

  FIBRE_BUF_END();
}

/* ------------------------------------------------------------ */
/* R E A L                                                      */
/* ------------------------------------------------------------ */
POINTER ReadRealVector(void)
{
  switch ( FibreParse( ANY_ ) ) {
  case ARRAYB_:
    GenericReadArray(float,ReadFlt,ARRAYE_);

  case STREAMB_:
    GenericReadArray(float,ReadFlt,STREAME_);

  case STRING_START_:
    FibreError( "STRING DELIMITER WAS NOT EXPECTED" );
    break;

  default:
    FibreError( "ARRAY DELIMITER EXPECTED" );
  }
  return (POINTER)(NULL);
}

void WriteRealVector(POINTER val)
{
  POINTER Base2;
  int     HiBound;
  int     Lo2;
  ARRAYP arr = (ARRAYP) val;
  int saveIndent;

  FIBRE_BUF_BEGIN();

  PrintIndent;
  Lo2 = arr->LoBound;
  fprintf( FibreOutFd, "[ %d,%d:", Lo2, Lo2+(arr->Size)-1 );
  saveIndent = Indent;
#ifdef VERBOSE
  fprintf( FibreOutFd, " # DRC=%d PRC=%d\n", arr->RefCount, arr->Phys->RefCount );
  Indent++;
#else
  Indent = 0;
  fprintf( FibreOutFd, " " );
#endif

  Indent++;
  Base2 = arr->Base;
  HiBound = Lo2 + arr->Size - 1;
  for ( ; Lo2 <= HiBound; Lo2++ ) {
    WriteFlt( (((float*)Base2)[Lo2]) );
  }

  Indent = saveIndent;
#ifdef VERBOSE
  PrintIndent;
#endif
  fprintf( FibreOutFd, "]\n" );

  FIBRE_BUF_END();
}
