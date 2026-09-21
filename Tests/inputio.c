/**************************************************************************/
/* FILE   **************         inputio.c         ************************/
/**************************************************************************/
/* Exercises the runtime's whole-stream input builtins (READ, PIPE, STDIN) */
/* directly from C, the way compiled SISAL code calls them.                */
/*                                                                         */
/* These paths carry no coverage from the .sis tests, and they are the ones */
/* that handle untrusted, arbitrary-length, possibly binary input, so they  */
/* are worth pinning: empty streams, streams larger than the read chunk,    */
/* embedded NUL bytes, non-seekable streams, and child exit status.         */
/**************************************************************************/

#include "sisalrt.h"

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

/* Argument blocks, laid out exactly as the code generator emits them. */
struct Args12 { struct ActRec *FirstAR; int Count; POINTER In1; POINTER Out1; };
struct Args13 { struct ActRec *FirstAR; int Count; POINTER Out1; };
struct Args14 { struct ActRec *FirstAR; int Count; POINTER In1; POINTER Out1;
                int Out2; };

extern void _READ(void*);
extern void _STDIN(void*);
extern void _PIPE(void*);

extern void InitSisalRunTime(void);
extern void StartWorkers(void);
extern void StopWorkers(void);

#define BIG_BYTES (3 * 1024 * 1024 + 12345)   /* well past SLURP_CHUNK */

static int failures = 0;

/* Build a SISAL character array (lower bound 1) from a C string. */
static POINTER MakeSisalString( const char *Str )
{
  size_t  len = strlen( Str );
  POINTER val;
  PHYSP   Phys;

  OptABld( val, 1, 1, (int)len, char );
  Phys = ((ARRAYP)val)->Phys;
  if ( len != 0 )
    memcpy( (char*)Phys->Base, Str, len );
  Phys->Size          = (int)len;
  Phys->Free          = 0;
  ((ARRAYP)val)->Size = (int)len;
  return val;
}

static void Check( const char *What, POINTER Val,
                   const char *Expect, size_t ExpectLen )
{
  ARRAYP      arr  = (ARRAYP)Val;
  const char *base = (const char*)(arr->Base + arr->LoBound);

  if ( (size_t)arr->Size != ExpectLen ) {
    printf( "FAIL %s: got %d bytes, expected %lu\n",
            What, arr->Size, (unsigned long)ExpectLen );
    failures++;
    return;
  }
  if ( ExpectLen != 0 && memcmp( base, Expect, ExpectLen ) != 0 ) {
    printf( "FAIL %s: contents differ\n", What );
    failures++;
    return;
  }
  /* The physical block's bookkeeping must agree with the logical length,
     or every later AGather onto this array writes in the wrong place. */
  if ( arr->Phys->Size != arr->Size ) {
    printf( "FAIL %s: Phys->Size %d disagrees with Array->Size %d\n",
            What, arr->Phys->Size, arr->Size );
    failures++;
    return;
  }
  printf( "ok   %s (%lu bytes)\n", What, (unsigned long)ExpectLen );
}

static void WriteFile( const char *Name, const char *Data, size_t Len )
{
  FILE *fp = fopen( Name, "wb" );

  if ( fp == NULL || (Len != 0 && fwrite( Data, 1, Len, fp ) != Len) ||
       (fp != NULL && fclose( fp ) != 0) ) {
    printf( "FAIL could not write %s\n", Name );
    exit( 1 );
  }
}

int main( int argc, char **argv )
{
  struct Args12 a12;
  struct Args13 a13;
  struct Args14 a14;
  char         *big;
  size_t        i;
  size_t        stdinLen;

  (void)argc; (void)argv;          /* run-test also invokes us with -w3 */

#ifdef _WIN32
  /* This test's own printf()s go through stdout same as the runtime's
     FibreOutFd does, and hit the same default-text-mode CRLF translation;
     see srt0.c for the fuller explanation. */
  _setmode( _fileno( stdout ), _O_BINARY );
#endif

  /* run-test drops the first line of output before diffing. */
  printf( "SISAL runtime input tests\n" );

  InitSisalRunTime();
  StartWorkers();

  /* An empty stream must yield a zero-length array, not a bad pointer. */
  WriteFile( "inputio-empty.dat", "", 0 );
  memset( &a12, 0, sizeof a12 );
  a12.In1 = MakeSisalString( "inputio-empty.dat" );
  _READ( &a12 );
  Check( "READ empty file", a12.Out1, "", 0 );

  WriteFile( "inputio-small.dat", "hello, sisal\n", 13 );
  memset( &a12, 0, sizeof a12 );
  a12.In1 = MakeSisalString( "inputio-small.dat" );
  _READ( &a12 );
  Check( "READ small file", a12.Out1, "hello, sisal\n", 13 );

  /* Binary data with embedded NULs and bytes above 127, larger than one
     read chunk, so the buffer has to grow and every byte has to survive. */
  big = (char*) malloc( BIG_BYTES );
  if ( big == NULL ) { printf( "FAIL out of memory\n" ); return 1; }
  for ( i = 0; i < (size_t)BIG_BYTES; i++ )
    big[i] = (char)(i * 31 + (i >> 8));
  WriteFile( "inputio-big.dat", big, BIG_BYTES );

  memset( &a12, 0, sizeof a12 );
  a12.In1 = MakeSisalString( "inputio-big.dat" );
  _READ( &a12 );
  Check( "READ large binary file", a12.Out1, big, BIG_BYTES );

  /* A pipe cannot be stat'd for its length, so this drives the growing path. */
  memset( &a14, 0, sizeof a14 );
  a14.In1 = MakeSisalString( "cat inputio-big.dat" );
  _PIPE( &a14 );
  Check( "PIPE large binary stream", a14.Out1, big, BIG_BYTES );
  if ( a14.Out2 != 0 ) {
    printf( "FAIL PIPE exit status: got %d, expected 0\n", a14.Out2 );
    failures++;
  } else {
    printf( "ok   PIPE zero exit status\n" );
  }

  /* A failing child must surface as a non-zero status rather than vanish. */
  memset( &a14, 0, sizeof a14 );
  a14.In1 = MakeSisalString( "exit 3" );
  _PIPE( &a14 );
  if ( a14.Out2 == 0 ) {
    printf( "FAIL PIPE exit status: got 0 for a child that exited 3\n" );
    failures++;
  } else {
    printf( "ok   PIPE non-zero exit status\n" );
  }

  /* STDIN has no length to query either.  Round-trip it through READ so the
     two builtins are checked against each other without pinning the input. */
  memset( &a13, 0, sizeof a13 );
  _STDIN( &a13 );
  stdinLen = (size_t)((ARRAYP)a13.Out1)->Size;
  WriteFile( "inputio-stdin.dat",
             (const char*)(((ARRAYP)a13.Out1)->Base +
                           ((ARRAYP)a13.Out1)->LoBound), stdinLen );
  memset( &a12, 0, sizeof a12 );
  a12.In1 = MakeSisalString( "inputio-stdin.dat" );
  _READ( &a12 );
  Check( "STDIN agrees with READ", a12.Out1,
         (const char*)(((ARRAYP)a13.Out1)->Base +
                       ((ARRAYP)a13.Out1)->LoBound), stdinLen );

  free( big );
  StopWorkers();

  if ( failures != 0 ) {
    printf( "%d input test(s) failed\n", failures );
    return 1;
  }
  printf( "all input tests passed\n" );
  return 0;
}
