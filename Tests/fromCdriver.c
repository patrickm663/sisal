/**************************************************************************/
/* FILE   **************       fromCdriver.c       ************************/
/**************************************************************************/
/* Author: Patrick Miller January  3 2001                                 */
/* Copyright (C) 2001 Patrick Miller                                      */
/**************************************************************************/
/*
 * $Log:
 */
/**************************************************************************/

#include "sisalInfo.h"

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

extern void foo(int*);
extern int NumWorkers;

extern void InitSisalRunTime(void);
extern void StartWorkers(void);
extern void StopWorkers(void);

int main(int argc, char** argv) {
   int x;
#ifdef _WIN32
   /* Same reasoning as srt0.c and inputio.c: this test's own puts()/printf()
      go through stdout's default text mode, which would CRLF-translate them. */
   _setmode( _fileno( stdout ), _O_BINARY );
#endif
   if ( argc > 1 && argv[1][0] == '-' && argv[1][1] == 'w' ) {
      NumWorkers = atoi(argv[1]+2);
   }
   InitSisalRunTime();
   StartWorkers();
   foo(&x);
   StopWorkers();
   puts("THIS LINE IS SKIPPED IN COMPARISON");
   printf("X should be three, and is %d\n",x);
   return 0;
}
