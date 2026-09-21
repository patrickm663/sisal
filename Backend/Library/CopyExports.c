/**************************************************************************/
/* FILE   **************       CopyExports.c       ************************/
/**************************************************************************/
/* Author: Dave Cann                                                      */
/* Update: Patrick Miller -- Ansi support (Dec 2000)                      */
/* Copyright (C) University of California Regents                         */
/**************************************************************************/
/**************************************************************************/

#include "world.h"


/**************************************************************************/
/* GLOBAL **************        CopyExports        ************************/
/**************************************************************************/
/* PURPOSE: COPY THE EXPORT LIST OF NODE n1 AND ATTACH IT TO NODE n2. A   */
/*          COPIED EXPORT IS NOT LINKED TO ITS COPY'S  DESTINATION NODE.  */
/**************************************************************************/

void CopyExports(PNODE n1, PNODE n2)
{
    PEDGE e;

    for ( e = n1->exp; e != NULL; e = e->esucc )
        LinkExport( n2, CopyEdge( e, n2, NULL_NODE ) );
}


/*
 * $Log:
 */
