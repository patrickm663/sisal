#ifndef FIBRE_H
#define FIBRE_H

/**************************************************************************/
/* FILE   **************          fibre.h          ************************/
/**************************************************************************/
/* Author: Dave Cann                                                      */
/* Update: Pat Miller -- Ansi support (Dec 2000)                          */
/* Copyright (C) University of California Regents                         */
/**************************************************************************/
/*
 * $Log:
 */
/**************************************************************************/

extern FILE *FibreInFd;
extern FILE *FibreOutFd;
extern FILE *PerfFd;

extern void    FibreError(char*);
extern int     FibreParse(int);
extern POINTER SisalMainArgs; 
extern POINTER ReadFibreInputs(void);;
extern void    WriteFibreOutputs(POINTER);;

extern int JsonOutput;

/* No-op in JSON mode: FIBRE's indentation is meaningless there, and JSON
   output is written compact (see the Write*Vector functions in
   vectorIO.c, which insert commas instead). */
#define PrintIndent \
{                                                  \
  int Counter;                            \
  if ( !JsonOutput ) \
    for ( Counter = Indent; Counter > 0; Counter-- ) \
      fprintf( FibreOutFd, " " );                    \
}

/* Write*Vector() in vectorIO.c wraps its whole body in one of these: every
 * WriteInt/WriteFlt/.../fprintf call it makes in between goes to an
 * in-memory stream instead of one fprintf() per array element hitting the
 * real (possibly piped, possibly contended with other threads) stream, and
 * FIBRE_BUF_END flushes the lot with a single fwrite. Nests correctly for
 * arrays of arrays/records, since it only ever redirects through whatever
 * FibreOutFd currently is. Falls back to writing straight through, exactly
 * as before, when open_memstream isn't available (MinGW) or fails (OOM).
 */
#ifdef HAVE_OPEN_MEMSTREAM
#define FIBRE_BUF_BEGIN() \
  FILE *_fibreRealFd = FibreOutFd; \
  char *_fibreBufData = NULL; \
  size_t _fibreBufSize = 0; \
  FILE *_fibreMemFd = open_memstream(&_fibreBufData, &_fibreBufSize); \
  if ( _fibreMemFd ) FibreOutFd = _fibreMemFd

#define FIBRE_BUF_END() \
  if ( _fibreMemFd ) { \
    fclose( _fibreMemFd ); \
    FibreOutFd = _fibreRealFd; \
    fwrite( _fibreBufData, 1, _fibreBufSize, FibreOutFd ); \
    free( _fibreBufData ); \
  }
#else
#define FIBRE_BUF_BEGIN() ((void)0)
#define FIBRE_BUF_END()   ((void)0)
#endif

extern char *iformat,*fformat,*dformat,*nformat,*cformat,*cformat2,*bformat;
extern int  fformat_is_default, dformat_is_default;
extern void JsonPutChar(char,FILE*);
extern void FormatFixedDecimal(double,int,char*,size_t);

/* iformat/fformat/dformat/etc are FIBRE's, tunable with -iformat and
   friends and always trailed by a separating space; JSON needs its own
   clean, comma-separated tokens instead, so each Write* picks a format
   on JsonOutput rather than reusing those.
   WriteInt still uses %d/iformat either way -- integers already print in
   full precision with no notation ambiguity. WriteFlt/WriteDbl are
   different: unless the user has picked a custom -fformat/-dformat,
   both FIBRE and JSON output use FormatFixedDecimal, which always
   renders in fixed decimal (never scientific) notation with just enough
   fractional digits for the value's own magnitude -- a fixed %f
   precision would either waste digits on large values or truncate small
   ones to all zeros. JSON always uses it, since a user's custom FIBRE
   format string isn't guaranteed to produce valid JSON anyway. */
#define WriteInt(x)  {PrintIndent; \
  if ( JsonOutput ) fprintf( FibreOutFd, "%d", x ); \
  else fprintf( FibreOutFd, iformat, x );   }
#define WriteFlt(x)  {PrintIndent; \
  if ( JsonOutput || fformat_is_default ) { \
    char _fbuf[512]; \
    FormatFixedDecimal( (double)(x), 9, _fbuf, sizeof(_fbuf) ); \
    fputs( _fbuf, FibreOutFd ); \
  } else fprintf( FibreOutFd, fformat, x ); }
#define WriteDbl(x)  {PrintIndent; \
  if ( JsonOutput || dformat_is_default ) { \
    char _dbuf[512]; \
    FormatFixedDecimal( (x), 17, _dbuf, sizeof(_dbuf) ); \
    fputs( _dbuf, FibreOutFd ); \
  } else fprintf( FibreOutFd, dformat, x );}
#define WriteNil(x)  {PrintIndent; \
  if ( JsonOutput ) fputs( "null", FibreOutFd ); \
  else fprintf( FibreOutFd, nformat, x );  }

#define WriteChar(x) \
{                                              \
  PrintIndent;                                 \
  if ( JsonOutput ) {                          \
    fputc( '"', FibreOutFd );                  \
    JsonPutChar( (x), FibreOutFd );            \
    fputc( '"', FibreOutFd );                  \
  }                                            \
  else if ( ((x) < ' ') || ((x) > '~') )            \
    fprintf( FibreOutFd, cformat2, (x) & 0xff);\
  else if ( (x) == '\\' )                      \
    fprintf( FibreOutFd, "'\\\\'\n" );         \
  else if ( (x) == '\'' )                      \
    fprintf( FibreOutFd, "'\\''\n" );          \
  else                                         \
    fprintf( FibreOutFd, cformat, (x) & 0xff );\
}

#define WriteBool(x) \
  {PrintIndent; \
   if ( JsonOutput ) fputs( (x) ? "true" : "false", FibreOutFd ); \
   else fprintf( FibreOutFd, bformat, (x)? 'T' : 'F' );}

#define ReadInt(x)  {FibreParse( INT_ );    x = FibreInt; }
#define ReadFlt(x)  {FibreParse( FLOAT_ );  x = FibreFlt; }
#define ReadDbl(x)  {FibreParse( DOUBLE_ ); x = FibreDbl; }
#define ReadNil(x)  {FibreParse( NIL_ );    x = FibreNil; }
#define ReadBool(x) {FibreParse( BOOL_ );   x = FibreBool;}
#define ReadChar(x) {FibreParse( CHAR_ );   x = FibreChar;}

#define IF_BOOL   12
#define IF_CHAR   13
#define IF_DOUBLE 14
#define IF_INT    15
#define IF_NULL   16
#define IF_REAL   17

#define IF_NON    18

#define IF_RCPX   19
#define IF_DCPX   20

#define IF_ARRAY   0
#define IF_RECORD  5
#define IF_UNION   9
#define IF_STREAM  6

extern char    FibreChar;
extern char    FibreBool;
extern char    FibreNil;
extern int     FibreInt;
extern float   FibreFlt;
extern double  FibreDbl;

extern int     FibreStrings;

#define BASE_         300
#define STRING_TERM_  300
#define STRING_START_ 301
#define STRING_CHAR_  302
#define INT_          303
#define CHAR_         304
#define DOUBLE_       305
#define NIL_          306
#define BOOL_         307
#define EOF_          308
#define RECORDE_      309
#define RECORDB_      310
#define STREAME_      311
#define STREAMB_      312
#define UNIONE_       313
#define UNIONB_       314
#define COLON_        315
#define ARRAYB_       316
#define ARRAYE_       317
#define SEMI_COLON_   318
#define FLOAT_        319
#define COMMA_        320
#define ANY_          321

extern char    LookAhead;
extern int     LookAheadToken;
extern int     Indent;

#define GET_LOOKAHEAD  LookAheadToken = FibreParse( ANY_ ); \
                       if ( LookAheadToken != ARRAYE_ &&    \
                            LookAheadToken != STREAME_ )    \
                         LookAhead = TRUE

extern char ArgumentString[];

#endif
