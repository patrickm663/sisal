/**************************************************************************/
/* FILE   **************         ifxstuff.c        ************************/
/**************************************************************************/
/* Author: Dave Cann                                                       */
/* Update: Patrick Miller -- Ansi support (Dec 2000)                      */
/* Copyright (C) University of California Regents                         */
/**************************************************************************/
/*
 * $Log:
 */
/**************************************************************************/

#include "world.h"

int FixPortsToo         = TRUE; /* Fix up Loop ports */
int AllowVMarks         = TRUE; /* Allow %mk=V input */
int StreamsOK           = TRUE; /* Allow stream types */
int InitialNodeLevel    = -1;   /* n->level not used */
int streams             = FALSE; /* No streams found yet */
int recursive           = FALSE; /* No %mk=B functions found yet */
int dbl                 = FALSE; /* Don't convert double types */
int flt                 = FALSE; /* Don't convert float  types */
int CheckForBadEdges    = FALSE;  /* Turn edge checking on/off */

/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */
#define DoAssignPragmas(x) \
{       \
  (x)->print = pragmas.print;   \
  (x)->mark  = pragmas.mark;    \
\
  (x)->name  = pragmas.name;    \
  (x)->line  = pragmas.line;    \
  (x)->file  = ( pragmas.file )?(pragmas.file):sfile;   \
  (x)->funct = ( pragmas.funct )?(pragmas.funct):sfunct;        \
}

void TypeAssignPragmas(PINFO i)
{
  DoAssignPragmas(i);
}
void NodeAssignPragmas(PNODE n)
{
  DoAssignPragmas(n);
  if (n->type==IFXGraph && IsReductionInterface(n->CoNsT)) {
      n->mark = 'd';
  }
}
void EdgeAssignPragmas(PEDGE e)
{
  DoAssignPragmas(e);
}

/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */
#define DoInitPragmas(x) \
{       \
  StandardPragmas(x); \
  (x)->print = TRUE; \
}

void PragInitPragmas(PRAGS *p)
{
  DoInitPragmas(p);
}
void TypeInitPragmas(PINFO i)
{
  DoInitPragmas(i);
}
void NodeInitPragmas(PNODE n)
{
  DoInitPragmas(n);
}
void EdgeInitPragmas(PEDGE e)
{
  DoInitPragmas(e);
}
