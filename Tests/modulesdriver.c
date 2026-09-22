/**************************************************************************/
/* FILE   **************      modulesdriver.c       ************************/
/**************************************************************************/
/* Copyright (C) University of California Regents                        */
/**************************************************************************/
/*
 * Links moduleA.sis and moduleB.sis -- two independently compiled SISAL
 * "programs", each built with -forC -- into one binary and calls into
 * both, round-tripping a real argument across the FLI each time (unlike
 * fromC.sis's foo(), which takes none). This is also the regression test
 * for a real link-time bug: each -forC compile emits its own file-scope
 * "int ProvideModuleDataBaseOnAllCompiles;" (see PrintFilePrologue in
 * if2preamble.c), and under GCC's -fno-common default two non-static
 * tentative definitions of the same name in different translation units
 * are a hard "multiple definition" link error, not the silent merge older
 * compilers gave them.
 */

#include "sisalInfo.h"

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

/* Both -forC entry points take one POINTER-to-int input, one
   POINTER-to-int output -- see PrintWriteOp's generated call in
   if2preamble.c; a zero-argument function like fromC.sis's foo() only
   gets the output pointer. */
extern void cube(int*, int*);
extern void doubleit(int*, int*);

extern void InitSisalRunTime(void);
extern void StartWorkers(void);
extern void StopWorkers(void);

int main(void) {
   int in1 = 3, out1;
   int in2 = 21, out2;
#ifdef _WIN32
   _setmode( _fileno( stdout ), _O_BINARY );
#endif
   InitSisalRunTime();
   StartWorkers();
   cube(&in1, &out1);
   doubleit(&in2, &out2);
   StopWorkers();
   puts("THIS LINE IS SKIPPED IN COMPARISON");
   printf("cube(3)=%d doubleit(21)=%d\n", out1, out2);
   return 0;
}
