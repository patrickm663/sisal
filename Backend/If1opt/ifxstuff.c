/**************************************************************************/
/* FILE   **************       ifxstuff.c<2>       ************************/
/**************************************************************************/
/* Author: Dave Cann                                                      */
/* Update: Patrick Miller -- Ansi support (Dec 2000)                      */
/* Copyright (C) University of California Regents                         */
/**************************************************************************/
/*
 * $Log:
 */
/**************************************************************************/

#include "world.h"

int ExpandedEqual       = FALSE; /* Use original def'n */
int FixPortsToo         = TRUE; /* Fix up Loop ports */
int AllowVMarks         = TRUE; /* Allow %mk=V input */
int StreamsOK           = TRUE; /* Allow stream types */
int InitialNodeLevel    = -1;   /* n->level not used */
int streams             = FALSE; /* No streams found yet */
int recursive           = FALSE; /* No %mk=B functions found yet */
int dbl                 = FALSE; /* Don't convert double types */
int flt                 = FALSE; /* Don't convert float  types */
int DMarkProblem        = FALSE; /* Don't mess with with %mk=d nodes */
int echange             = 0;
int nchange             = 0;
int CheckForBadEdges    = FALSE; /* Turn edge checking on/off */


double mcosts[12]       = { 0.0,0.0,0.0,0.0,0.0,0.0, /* Don't care about */
                            0.0,0.0,0.0,0.0,0.0,0.0 }; /* costs yet */

void PlaceInEntryTable(char *c)
{}
void PlaceInFortranTable(char *c)
{}
void PlaceInCTable(char *c)
{}

/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */
#define DoAssignPragmas(x) \
{       \
  (x)->name  = pragmas.name;    \
  (x)->line  = pragmas.line;    \
  (x)->mark  = pragmas.mark;    \
  (x)->umark  = pragmas.umark;  \
  (x)->print = pragmas.print;   \
\
  (x)->file  = ( pragmas.file )?(pragmas.file):sfile;   \
  (x)->funct = ( pragmas.funct )?(pragmas.funct):(      \
                     (cfunct == NULL)? "" : cfunct->G_NAME);    \
}
void TypeAssignPragmas(PINFO p)
{
  DoAssignPragmas(p);
}
void EdgeAssignPragmas(PEDGE p)
{
  DoAssignPragmas(p);
}
void NodeAssignPragmas(PNODE p)
{
  DoAssignPragmas(p);
}

/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */
#define DoInitPragmas(x) StandardPragmas(x)

void PragInitPragmas(PRAGS *p)
{
  DoInitPragmas(p);
}
void NodeInitPragmas(PNODE p)
{
  DoInitPragmas(p);
}
void TypeInitPragmas(PINFO p)
{
  DoInitPragmas(p);
}
void EdgeInitPragmas(PEDGE p)
{
  DoInitPragmas(p);
}
