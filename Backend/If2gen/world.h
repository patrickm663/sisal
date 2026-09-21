#ifndef WORLD_H
#define WORLD_H

/**************************************************************************/
/* FILE   **************         world.h<2>        ************************/
/**************************************************************************/
/* Author: Dave Cann                                                      */
/* Update: Patrick Miller -- Ansi support (Dec 2000)                      */
/* Copyright (C) University of California Regents                         */
/**************************************************************************/
/*
 * $Log$
 * Revision 1.4  2002/11/21 04:05:02  patmiller
 * Continued updates.  A number of 15 year old bugs have been
 * fixed up:
 *
 * 1) Merging union values where the tags are different, but the value
 *    is the same (e.g. all tags are NULL type)
 *
 * 2) Literals were capped at size 127 bytes! (now 10240)
 *
 * Revision 1.3  2001/01/02 09:16:45  patmiller
 * Now ANSI compliant, but still a pthread problem
 *
 * Revision 1.2  2001/01/01 05:46:22  patmiller
 * Adding prototypes and header info -- all will be broken
 *
 * Revision 1.1.1.1  2000/12/31 17:57:34  patmiller
 * Well, here is the first set of big changes in the distribution
 * in 5 years!  Right now, I did a lot of work on configuration/
 * setup (now all autoconf), breaking out the machine dependent
 * #ifdef's (with a central acconfig.h driven config file), changed
 * the installation directories to be more gnu style /usr/local
 * (putting data in the /share/sisal14 dir for instance), and
 * reduced the footprint in the top level /usr/local/xxx hierarchy.
 *
 * I also wrote a new compiler tool (sisalc) to replace osc.  I
 * found that the old logic was too convoluted.  This does NOT
 * replace the full functionality, but then again, it doesn't have
 * 300 options on it either.
 *
 * Big change is making the code more portably correct.  It now
 * compiles under gcc -ansi -Wall mostly.  Some functions are
 * not prototyped yet.
 *
 * Next up: Full prototypes (little) checking out the old FLI (medium)
 * and a new Frontend for simpler extension and a new FLI (with clean
 * C, C++, F77, and Python! support).
 *
 * Pat
 *
 *
 * Revision 1.15  1994/06/16  21:30:43  mivory
 * info format and option changes M. Y. I.
 *
 * Revision 1.14  1994/04/15  15:51:07  denton
 * Added config.h to centralize machine specific header files.
 * Fixed gcc warings.
 *
 * Revision 1.13  1994/02/15  23:40:56  miller
 * Changes to allow new IF1/2 types (complex, typesets, etc...)
 *
 * Revision 1.12  1993/03/23  22:46:16  miller
 * date problem
 *
 * Revision 1.11  1994/03/11  23:09:32  miller
 * Moved IFX.h into Backend/Library and added support for Minimal
 * installation (removing source as compiled).
 *
 * Revision 1.10  1993/02/27  00:29:49  miller
 * Added MinSliceThrottle (to turn off %MS pragmas)
 *
 * Revision 1.9  1993/02/24  18:48:36  miller
 * Added new PrintHiRange and PrintLowRange to support new loop styles.
 *
 * Revision 1.8  1993/01/15  22:25:57  miller
 * Moved the definition for info up into IFX.h
 *
 * Revision 1.7  1993/01/14  22:27:25  miller
 * Now allow code comments if the -% option is used.  Had to make changes
 * to reflect the new ThinCopy pragma.  Fixed some other pragma stuff
 * in ifxstuff.c
 *
 * Revision 1.6  1993/01/07  00:38:50  miller
 * Make changes for LINT and combined files.
 *
 */
/**************************************************************************/

#include "sisalInfo.h"
#include "../Library/IFX.h"


extern int    info;             /* DUMP INFORMATION? */
extern FILE *infoptr;           /* INFORMATION output file */
extern FILE *infoptr1;          /* INFORMATION output file */
extern FILE *infoptr2;          /* INFORMATION output file */
extern FILE *infoptr3;          /* INFORMATION output file */
extern FILE *infoptr4;          /* INFORMATION output file */
extern int    aimp;             /* OPTIMIZE ARRAY DEREFERENCE OPERATIONS? */
extern int    if2opt;           /* OPTIMIZE GatherAT NODES? */

extern int    nmid;             /* NAME STAMP */
extern int    tmpid;            /* TEMPORARY STAMP */
extern int    bounds;           /* GENERATE BOUNDS CHECKS? */

extern int    line;             /* LINE NUMBER OF PREV. LINE OF IF1 FILE */

extern int   gshared;

extern int   sequential;        /* CURRENTLY GENERATING SEQUENTIAL CODE? */

extern int   invtfa;            /* COUNT OF INVARIANT TASK FRAME ARGS */
extern int   vec;               /* FURTHER VECTORIZE CODE? */

extern int   rmsrcnt;           /* COUNT OF ELIMINATED SR PRAGMAS */
extern int   rmpmcnt;           /* COUNT OF ELIMINATED PM PRAGMAS */
extern int   rmcmcnt;           /* COUNT OF ELIMINATED CM PRAGMAS */
extern int   rmcnoop;           /* COUNT OF REMOVED CONDITIONAL COPY NoOpS */
extern int   rmnoop;            /* COUNT OF REMOVED COPY NoOpS */
extern int   rmsmark;           /* COUNT OF REMOVED smarks */
extern int   rmvmark;           /* COUNT OF REMOVED vmarks */

extern int   cRay;              /* COMPILING FOR THE CRAY? */
extern int   alliantfx;         /* COMPILING FOR THE ALLIANT FX SERIES? */
extern int   movereads;         /* MOVE ARRAY READ OPERATIONS? */
extern int   xmpchains;         /* FORM CHAINS FOR THE CRAY X-MP? */
extern int   newchains;  

extern int   rag;               /* IDENTIFY RAGGED MEM-ALLOCS? */
extern int   bip;               /* BIP OPTIMIZATION? */
extern int   bipmv;
extern int   oruntime;/* USE ORIGINAL SISAL MICROTASKING SOFTWARE */

extern int   bindtosisal;       /* BIND INTERFACE CALLS TO SISAL? */

extern int   freeall;           /* FORCE RELEASE OF ALL STORAGE? */

extern int   Iupper;            /* INTERFACE NAME GENERATION COMMANDS */
extern int   IunderR;
extern int   IunderL;

extern int   fva;               /* FORCE ALLIANT VECTORIZATION PRAGMAS? */
extern int   fvc;               /* FORCE CRAY VECTORIZATION PRAGMAS? */
extern int   nltss;             /* COMPILE FOR NLTSS C-COMPILER? */

extern int   SISdebug;          /* REMOVE DEAD FUNCTION CALLS? */

extern int   intrinsics;        /* RECOGNIZE LOGICAL FUNCTIONS: and,or,xor,not? */

extern int   max_dims;          /* MAXIMUM NUMBER OF DESIRED POINTER SWAP DIMENSIONS */
extern int   share;             /* TRY AND SHARE POINTER SWAP STORAGE */

extern int   assoc;             /* DO ASSOCIATIVE TRANSFORMATIONS? */
extern int   stream_io;

extern int   standalone;        /* CALLED FROM THE OPERATING SYSTEM? */

extern int   gdata;             /* PREPARE GLOBAL DATA? */

extern int   sdbx;              /* GENERATE SDBX CODE? */

extern FILE *hyfd;              /* HYBRID FILE DESCRIPTOR */
extern char *hybrid;            /* HYBRID FILE NAME */

extern int nobrec;              /* DISABLE BASIC RECORD OPTIMIZATION? */
extern int CodeComments;        /* Show source code lines in code */
extern int MinSliceThrottle;    /* TRUE==> use minslice, F==> No throttle */
/* ------------------------------------------------------------ */
/* if2yank.c */
extern void     If2Yank1(void);
extern void     If2Yank0(void);
extern void     WriteYankInfo(void);

/* if2temp.c */
extern char     *MakeName(char*,char*,int);
extern PTEMP    GetTemp(char*,PINFO,int);
extern void     InitializeSymbolTable(void);
extern void     ChangeToAllocated(PEDGE,PNODE);
extern void     FreeTemp(PEDGE);
extern void     PropagateTemp(PNODE,int,int,PTEMP);
extern int      IsTempExported(PNODE,PTEMP);
extern int      IsTempImported(PNODE,PTEMP);
extern void     PrintLocals(void);
extern PNODE    FindCriticalPath(PNODE,PNODE);
extern void     AssignTemps(PNODE);
extern void     PrintFrameDeallocs(void);

/* if2names.c */
extern char     *GetCopyFunction(PINFO);
extern char     *GetReadFunction(int);
extern char     *GetWriteFunction(int);
extern char     *GetIncRefCountName(PINFO);
extern char     *GetSetRefCountName(PINFO);

/* if2preamble.c */
extern void     PrintFilePrologue(void);
extern void     MarkRecursiveFunctions(void);
extern void     CheckParallelFunctions(void);
extern void     PrintFunctPrologue(PNODE);
extern void     PrintFunctEpilogue(PNODE);
extern void     PrintFileEpilogue(void);

/* if2smash.c */
extern void     GenSmashTypes(void);

/* if2print.c */
extern void     PrintIndentation(int);
extern void     PrintTemp(PEDGE);
extern void     PrintFldRef(char*,char*,PEDGE,char*,int);
extern void     PrintAssgn(int,PEDGE,PEDGE);
extern void     PrintFldAssgn(int,char*,char*,PEDGE,char*,int,PEDGE);
extern void     PrintMacro(int,char*,PNODE,char*);
extern void     PrintSetRefCount(int,PEDGE,int,int);
extern void     PrintFreeCall(int,PEDGE);
extern void     PrintConsumerModifiers(int,PNODE);
extern void     PrintProducerLastModifiers(int,PNODE);
extern void     PrintProducerModifiers(int,PNODE);
extern int      GenIsIntrinsic(PNODE);
extern void     PrintGraph(int,PNODE);

/* if2record.c */
extern void     PrintUGetTag(int,PNODE);
extern void     PrintUElement(int,PNODE);
extern void     PrintUBuild(int,PNODE);
extern void     PrintRBuild(int,PNODE);
extern void     PrintRElements(int,PNODE);
extern void     PrintRReplace(int,PNODE);
extern void     PrintRecordNoOp(int,PNODE);
extern void     PrintBRAStore(int,PNODE,PNODE);
extern void     PrintBRBuild(int,PNODE);
extern void     PrintBROptAElement(int,PNODE);
extern void     PrintBRElements(int,PNODE);
extern void     PrintBRReplace(int,PNODE);
extern void     PrintUTagTest(int,PNODE);

/* if2array.c */
extern char     *GetSisalInfo(PNODE,char*);
extern char     *GetSisalInfoOnEdge(PEDGE,char*);
extern void     PrintBoundsCheck(int,PNODE,PEDGE,PEDGE);
extern void     PrintRagged(int,PNODE);
extern void     PrintPSMemAllocDVI(int,PNODE);
extern void     PrintPSScatter(int,PNODE);
extern void     PrintPSManager(int,PNODE,char*);
extern void     PrintPSFree(int,PNODE,char*,PNODE);
extern void     PrintPSAlloc(int,PNODE,char*);
extern void     PrintMemAlloc(int,PNODE);
extern void     PrintGABase(int,PNODE);
extern void     PrintOptAElement(int,PNODE);
extern void     PrintArrayMacro(int,char*,char*,PNODE);
extern void     PrintAReplace(int,PNODE);
extern void     PrintABuild(int,PNODE);
extern void     PrintABuildAT(int,PNODE);
extern void     PrintAAddHLAT(int,PNODE);
extern void     PrintAAddH(int,PNODE);
extern void     PrintACatenateAT(int,PNODE);
extern void     PrintArrayNoOp(int,PNODE);

/* if2select.c */
extern void     PrintSelect(int,PNODE);
extern void     PrintTagCase(int,PNODE);

/* if2loop.c */
extern void     PrintAStore(int,PNODE,PNODE);
extern void     PrintYankedRed(int,PNODE,char*);
extern int      AreAllUnitFanout(PNODE);
extern void     PrintSumOfTerms(int,PEDGE);
extern PEDGE    GetSliceParam(PEDGE,PNODE);
extern void     PrintRanges(PNODE);
extern void     PrintRangeLow(PNODE);
extern void     PrintRangeHigh(PNODE);
extern void     PrintSliceTaskInit(int,PNODE);
extern void     PrintForall(int,PNODE);
extern void     PrintLoop(int,PNODE);
extern void     PrintFirstSum(int,PNODE);
extern void     PrintTri(int,PNODE);
extern void     PrintVMinMax(int,PNODE,char*);
extern int      LCMSize(PNODE);
extern void     PrintReturnRapUp(int,PNODE);

/* if2aimp.c */
extern void     NormalizeVectorLoop(PNODE);
extern void     AssignNewPortNums(PNODE,int);
extern void     If2AImp(void);
extern void     WriteIf2AImpInfo(void);

/* if2opt.c */
extern void     PrepareGraph(PNODE);
extern void     If2Opt(void);
extern void     WriteIf2OptInfo(void);
extern void     WriteIf2AImpInfo2(void);
extern void     WriteIf2OptInfo2(void);

/* if2vector.c */
extern void     WriteVectorInfo(void);
extern void     If2Vectorize(int);
extern void     PrintNOVECTOR(void);
extern void     PrintVECTOR(void);
extern void     PrintASSOC(void);
extern void     PrintSAFE(char*);

/* if2prebuild.c */
extern void     GenNormalizeNode(PNODE);
extern void     PointerSwap(PNODE,PNODE);
extern void     WritePrebuildInfo(void);
extern void     OptimizeBIPs(PNODE);
extern void     If2Prebuild0(void);
extern void     If2Prebuild1(void);
extern void     If2Prebuild2(void);

/* if2fibre.c */
extern void     PrintReadFibreInputs(PNODE);
extern void     PrintTypeWriters(PNODE);
extern void     PrintWriteFibreOutputs(PNODE);
extern void     PrintWriteFibreInputs(PNODE);
extern void     PrintPeek(int,PNODE);

/* if2free.c */
extern char     *GetFreeName(PINFO);
extern void     PrintFreeUtilities(void);
extern void     PrintInputDeallocs(char*,int,PNODE);
extern void     PrintOutputDeallocs(int,PNODE);

/* if2interface.c */
extern void     WriteInterfaceInfo(void);
extern int      GetLanguage(PNODE);
extern char     *BindInterfaceName(char*,int,int);
extern void     PrintInterfaceCall(int,PNODE,PNODE);
extern int      GenIsReadOnly(PNODE,int);
extern void     PrintInterfaceUtilities(void);
extern void     PrintInterface(PNODE);

/* if2sdbx.c */
extern void     PrintSdbxFunctionList(void);
extern void     BuildAndPrintSdbxScope(PNODE);
extern void     UpdateSdbxScopeNames(PNODE);
extern void     SaveSdbxState(PNODE);

/* if2ureduce.c */
extern void     PrintUReduceRapUp(int,PNODE);
extern void     PrintUReduceUpd(int,PNODE);
extern void     PrintUReduceInit(int,PNODE);

#endif
