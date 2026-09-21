#ifndef OPTION_H
#define OPTION_H

/**************************************************************************/
/* FILE   **************          option.h         ************************/
/**************************************************************************/
/* Author: Patrick Miller December 31 2000                                */
/* Copyright (C) 2000 Patrick Miller                                      */
/**************************************************************************/
/*
 * $Log:
 */
/**************************************************************************/

#include "charStarQueue.h"

/* ----------------------------------------------- */
/* Option descriptions                             */
/* ----------------------------------------------- */
typedef struct sisalc_option {
   char* name;
   char* alternate;
   int (*matcher)(int,char*,char***,struct sisalc_option*);
   char* oneLineDoc;
   char* manDoc;
   int* param0;
   int* param1;
   char** param2;
   charStarQueue** queue;
} option_t;

extern int defaultTrue(int,char*,char***, option_t*);
extern int defaultFalse(int,char*,char***, option_t*);
extern int defaultInitialized(int,char*,char***, option_t*);
extern int suffixedFile(int,char*,char***, option_t*);
extern int fetchStringEqual(int,char*,char***, option_t*);
extern int fetchStringNext(int,char*,char***, option_t*);
extern int appendQueue(int,char*,char***, option_t*);
extern int prefixedOption(int,char*,char***, option_t*);
extern int catchAll(int,char*,char***, option_t*);

extern int optionHelp(int,char*,char***, option_t*);
extern int optionHTML(int,char*,char***, option_t*);
extern int optionMAN(int,char*,char***, option_t*);

extern int overviewOption(int action,char*,char*** argP, option_t* option);
extern void setOptionDefaults(option_t* options);

extern void exitIfNotFound(char* bad);
extern void optionScan(char* program, int argc, char** argv, option_t* options, void (*handler)(char*));

#endif
