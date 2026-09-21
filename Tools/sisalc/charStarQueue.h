#ifndef CHARSTARQUEUE_H
#define CHARSTARQUEUE_H

/**************************************************************************/
/* FILE   **************      charStarQueue.h      ************************/
/**************************************************************************/
/* Author: Patrick Miller December 31 2000                                */
/* Copyright (C) 2000 Patrick Miller                                      */
/**************************************************************************/
/*
 * $Log:
*/
/**************************************************************************/


/* ----------------------------------------------- */
/* We need a lot of FIFO queue structures          */
/* ----------------------------------------------- */
typedef struct charStarQueue {
   char* charStar;
   struct charStarQueue* next;
} charStarQueue;

extern void enqueue(charStarQueue** queue, char* arg);
extern void explodeEnqueue(charStarQueue** queue, char* arg);
extern char* dequeue(charStarQueue** queue);
extern int queueSize(charStarQueue* queue);

#endif
