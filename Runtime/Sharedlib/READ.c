#include "sisalrt.h"

struct Args15;
struct Args16;

#undef _FATAL
void         _FATAL(void *);
int RecompileTheModuleDefining_FATAL = 0;
#undef _READ
void         _READ(void *);
int RecompileTheModuleDefining_READ = 0;
#undef _PIPE
void         _PIPE(void *);
int RecompileTheModuleDefining_PIPE = 0;
#undef _STDIN
void         _STDIN(void *);
int RecompileTheModuleDefining_STDIN = 0;
#undef _ARGV
void         _ARGV(void *);
int RecompileTheModuleDefining_ARGV = 0;
#undef _EXIT
void         _EXIT(struct Args15*);
int RecompileTheModuleDefining_EXIT = 0;
#undef _LSHIFT
void         _LSHIFT(struct Args16*);
int RecompileTheModuleDefining_LSHIFT = 0;
#undef _RSHIFT
void         _RSHIFT(struct Args16*);
int RecompileTheModuleDefining_RSHIFT = 0;
#undef _BITOR
void         _BITOR(struct Args16*);
int RecompileTheModuleDefining_BITOR = 0;
#undef _BITAND
void         _BITAND(struct Args16*);
int RecompileTheModuleDefining_BITAND = 0;
#undef _BITXOR
void         _BITXOR(struct Args16*);
int RecompileTheModuleDefining_BITXOR = 0;

struct Args12 {   
struct ActRec *FirstAR; int Count;   
POINTER In1;    POINTER Out1;   
  };

struct Args13 {   
struct ActRec *FirstAR; int Count;   
POINTER Out1;   
  };

struct Args14 {   
struct ActRec *FirstAR; int Count;   
  POINTER In1;    POINTER Out1;   int Out2;
  };

struct Args15 {   
struct ActRec *FirstAR; int Count;   
  int In1; POINTER In2;    int Out1;
  };

struct Args16 {   
struct ActRec *FirstAR; int Count;   
  int In1;  int In2; int Out1;
  };

extern char** sisal_save_argv;
void _ARGV( void* args )
{
  POINTER val;
  POINTER arg;
  char** p;
  char* q;

  ABld(val,1,1); /* Empty array */
  ((struct Args13*)args)->Out1 = val;

  for(p=sisal_save_argv;p && *p;++p) {
    ABld(arg,1,1);
    for(q=*p; q && *q; ++q) {
      AGather(arg,*q,char);
    }
    AGather(val,arg,POINTER);
  }
}

/************************************************************************\
 * Whole-stream input.
 *
 * These used to accumulate a stream one byte at a time with fgetc()/AGather().
 * That is doubly expensive: a locked, bounds-checked macro per byte, and an
 * array whose capacity grows *arithmetically* (DoPhysExpand adds
 * ExpHistory*ArrayExpansion cells per expansion), so filling N bytes costs
 * O(N^1.5) in memmove alone.  Instead we pull the stream into a scratch buffer
 * with block reads and geometric growth, then build the SISAL array once, at
 * exactly the right size.  For a regular file the size is known up front and
 * the whole read is a single fread() into a single allocation.
\************************************************************************/

#define SLURP_CHUNK (64 * 1024)

/* Upper bound on a file name or pipe command handed over from SISAL. */
#define PATH_BUFFER_SIZE 4096

/* Build a SISAL character array (lower bound 1) holding Len bytes of Data. */
static POINTER BuildCharArray( const char *Data, size_t Len )
{
  POINTER val;
  PHYSP   Phys;

  if ( Len > (size_t)INT_MAX ) {
    FPRINTF( stderr, "SISAL: input of %lu bytes exceeds the maximum array size\n",
             (unsigned long)Len );
    exit( 1 );
  }

  /* Capacity is exactly Len: OptABld sizes to (hi - lo + 1). */
  OptABld( val, 1, 1, (int)Len, char );

  Phys = ((ARRAYP)val)->Phys;
  if ( Len != 0 )
    memcpy( (char*)Phys->Base, Data, Len );

  Phys->Size            = (int)Len;
  Phys->Free            = 0;
  ((ARRAYP)val)->Size   = (int)Len;

  return val;
}

/* Read Fp to end of stream.  What names the source for diagnostics. */
static POINTER SlurpStream( FILE *Fp, const char *What )
{
  char   *buf = NULL;
  size_t  cap = 0;
  size_t  len = 0;
  POINTER val;

#ifdef HAVE_SYS_STAT_H
  {
    /* A regular file tells us its length, so size the buffer once and read
       the lot in a single call.  Anything else (pipe, tty, /proc) reports a
       size we cannot trust, so fall through to the growing path. */
    struct stat st;
    int fd = fileno( Fp );

    if ( fd >= 0 && fstat( fd, &st ) == 0 && S_ISREG( st.st_mode ) &&
         st.st_size > 0 && (size_t)st.st_size < (size_t)INT_MAX )
      cap = (size_t)st.st_size + 1;
  }
#endif

  if ( cap == 0 )
    cap = SLURP_CHUNK;

  if ( (buf = (char*) malloc( cap )) == NULL ) {
    FPRINTF( stderr, "SISAL: out of memory reading %s\n", What );
    exit( 1 );
  }

  for ( ;; ) {
    size_t room = cap - len;
    size_t got;

    if ( room == 0 ) {
      char  *grown;
      size_t want = cap + (cap / 2) + SLURP_CHUNK;   /* geometric growth */

      if ( want <= cap || want > (size_t)INT_MAX ) {
        FPRINTF( stderr, "SISAL: input from %s exceeds the maximum array size\n",
                 What );
        free( buf );
        exit( 1 );
      }
      if ( (grown = (char*) realloc( buf, want )) == NULL ) {
        FPRINTF( stderr, "SISAL: out of memory reading %s\n", What );
        free( buf );
        exit( 1 );
      }
      buf  = grown;
      cap  = want;
      room = cap - len;
    }

    got  = fread( buf + len, 1, room, Fp );
    len += got;

    if ( got < room ) {
      if ( ferror( Fp ) ) {
        FPRINTF( stderr, "SISAL: read error on %s\n", What );
        perror( What );
        free( buf );
        exit( 1 );
      }
      break;                                          /* clean end of stream */
    }
  }

  val = BuildCharArray( buf, len );
  free( buf );
  return val;
}

/* Copy a SISAL character array into a NUL-terminated C string.  SISAL arrays
   are counted, not NUL-terminated, so this is the only safe way to hand one
   to fopen()/popen(). */
static void ArrayToCString( ARRAYP Array, char *Buf, size_t BufSize,
                            const char *What )
{
  size_t size = (Array->Size > 0) ? (size_t)Array->Size : 0;

  if ( size >= BufSize ) {
    FPRINTF( stderr, "SISAL: %s is %lu bytes, longer than the %lu byte limit\n",
             What, (unsigned long)size, (unsigned long)(BufSize - 1) );
    exit( 1 );
  }

  if ( size != 0 )
    memcpy( Buf, (char*)(Array->Base + Array->LoBound), size );
  Buf[size] = '\0';
}

void _FATAL( void* args )
{
  ARRAYP message = (ARRAYP)(((struct Args12*)args)->In1);
  int    size    = (message->Size > 0) ? message->Size : 0;

  /* The message is a counted array with no terminator, so bound the print
     with a precision rather than letting %s run off the end of it. */
  FPRINTF( stderr, "FATAL: %.*s\n", size,
           (char*)(message->Base + message->LoBound) );
  exit( 1 );
}

void _READ( void* args )
{
  FILE *fp;
  char  name[PATH_BUFFER_SIZE];

  ArrayToCString( (ARRAYP)(((struct Args12*)args)->In1),
                  name, sizeof(name), "file name" );

  if ( (fp = fopen( name, "r" )) == NULL ) {
    FPRINTF( stderr, "SISAL: cannot open file: %s\n", name );
    perror( name );
    exit( 1 );
  }

  ((struct Args12*)args)->Out1 = SlurpStream( fp, name );

  if ( fclose( fp ) != 0 ) {
    FPRINTF( stderr, "SISAL: error closing file: %s\n", name );
    perror( name );
    exit( 1 );
  }
}

void _PIPE( void* args )
{
  FILE *fp;
  char  command[PATH_BUFFER_SIZE];
  int   status;

  ArrayToCString( (ARRAYP)(((struct Args14*)args)->In1),
                  command, sizeof(command), "pipe command" );

  if ( (fp = popen( command, "r" )) == NULL ) {
    FPRINTF( stderr, "SISAL: cannot run pipe: %s\n", command );
    perror( command );
    exit( 1 );
  }

  ((struct Args14*)args)->Out1 = SlurpStream( fp, command );

  /* pclose() returns -1 if it could not reap the child; that is distinct from
     the child exiting non-zero, and used to be discarded silently. */
  if ( (status = pclose( fp )) == -1 ) {
    FPRINTF( stderr, "SISAL: error closing pipe: %s\n", command );
    perror( command );
    exit( 1 );
  }

  ((struct Args14*)args)->Out2 = status;
}

void _STDIN( void* args )
{
  ((struct Args13*)args)->Out1 = SlurpStream( stdin, "standard input" );
}


void _EXIT(struct Args15* args) {
  ARRAYP message = (ARRAYP)(args->In2);
  int i;
  for(i=0;i<message->Size;++i) {
    fputc(((char*)(message->Base+message->LoBound))[i],stderr);
  }
  fputc('\n',stderr);
  exit(args->In1);
}

void _LSHIFT(struct Args16* args) {
  args->Out1 = (args->In1 << args->In2);
}

void _RSHIFT(struct Args16* args) {
  args->Out1 = (args->In1 >> args->In2);
}

void _BITOR(struct Args16* args) {
  args->Out1 = (args->In1 | args->In2);
}

void _BITAND(struct Args16* args) {
  args->Out1 = (args->In1 & args->In2);
}

void _BITXOR(struct Args16* args) {
  args->Out1 = (args->In1 ^ args->In2);
}

