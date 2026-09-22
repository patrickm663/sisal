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
#include <math.h>

int     FibreStrings = TRUE;
int     sisal_file_io = 0;
int     JsonOutput = TRUE;

/* Format val with `sig` significant digits, always in fixed decimal
   notation (never scientific/exponential). This is the default real/double
   formatting for both FIBRE and JSON output. Plain "%.*f" alone can't do
   this: a fixed fractional-digit count either wastes digits on
   large-magnitude values or truncates small-magnitude ones to all zeros,
   so the fractional digit count is derived from the value's own magnitude
   instead of being fixed in advance. */
void FormatFixedDecimal( double val, int sig, char *buf, size_t bufsize )
{
  int frac_digits;

  if ( val == 0.0 || !isfinite(val) ) {
    snprintf( buf, bufsize, "%.*f", sig > 1 ? sig - 1 : 0, val );
    return;
  }

  frac_digits = sig - 1 - (int)floor( log10( fabs( val ) ) );
  if ( frac_digits < 0 )   frac_digits = 0;
  if ( frac_digits > 340 ) frac_digits = 340;  /* smallest normal double ~1e-308 */

  snprintf( buf, bufsize, "%.*f", frac_digits, val );
}

/* Write one byte as JSON string content (no surrounding quotes). Used both
   for a char array printed as a JSON string and, via WriteChar in fibre.h,
   for a lone char or a char array *not* being printed as a string -- same
   escaping either way, so callers agree on which bytes need one. */
void JsonPutChar( char c, FILE *fp )
{
  switch ( c ) {
    case '"' : fputs( "\\\"", fp ); break;
    case '\\': fputs( "\\\\", fp ); break;
    case '\b': fputs( "\\b",  fp ); break;
    case '\f': fputs( "\\f",  fp ); break;
    case '\n': fputs( "\\n",  fp ); break;
    case '\r': fputs( "\\r",  fp ); break;
    case '\t': fputs( "\\t",  fp ); break;
    default:
      if ( isascii((unsigned char)c) && isprint((unsigned char)c) )
        fputc( c, fp );
      else
        fprintf( fp, "\\u%04x", (unsigned char)c & 0xff );
  }
}

/* Shared body for the five scalar-vector JSON branches below: a plain
   comma-separated JSON array, no indentation or bounds header (those are
   FIBRE-only). WriteCharVector uses WriteChar directly instead, since it
   also has the string-vs-array-of-chars case to handle. */
#define WriteJsonVector(elemtype, WriteFn) \
{ \
  Lo2 = arr->LoBound; \
  Base2 = arr->Base; \
  HiBound = Lo2 + arr->Size - 1; \
  fputc( '[', FibreOutFd ); \
  for ( ; Lo2 <= HiBound; Lo2++ ) { \
    if ( Lo2 > arr->LoBound ) fputc( ',', FibreOutFd ); \
    WriteFn( (((elemtype*)Base2)[Lo2]) ); \
  } \
  fputc( ']', FibreOutFd ); \
}

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

  if ( JsonOutput ) {
    WriteJsonVector(char, WriteBool);
  } else {
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
  }

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

  if ( JsonOutput ) {
    if ( FibreStrings && Lo2 == 1 ) {
      fputc( '"', FibreOutFd );
      for (       ; Lo2 <= HiBound; Lo2++ ) {
        JsonPutChar( *((char*)Base2+Lo2), FibreOutFd );
      }
      fputc( '"', FibreOutFd );
    } else {
      WriteJsonVector(char, WriteChar);
    }
  } else {
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
  }

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

  if ( JsonOutput ) {
    WriteJsonVector(double, WriteDbl);
  } else {
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
  }

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

    if ( JsonOutput ) {
      WriteJsonVector(int, WriteInt);
    } else {
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
    }

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

  if ( JsonOutput ) {
    WriteJsonVector(char, WriteNil);
  } else {
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
  }

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

  if ( JsonOutput ) {
    WriteJsonVector(float, WriteFlt);
  } else {
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
  }

  FIBRE_BUF_END();
}
