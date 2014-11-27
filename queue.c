/*****************************************************************************
 *
 * NAME
 *    queue.c
 *
 * CONTENTS
 *    This file contains a number of queue routines for the chat server.
 *
 *    Copyright 1996 Hyperreal.  All rights reserved.  For more information
 *    consult the file LICENSE.
 * 
 *****************************************************************************/

#include "telechat.h"

/* The following message should be in format suitable
   to be transmitted via telnet protocol directly. */
static char *overflow_msg = "\r\n [ Part of the output was lost. ]\r\n";

static char *qnext (QUEUE *q, char *s);
static char *qprev (QUEUE *q, char *s);
static char *qnextline (QUEUE *q, char *s);

/*****************************************************************************
 * 
 * NAME
 *    qnext
 * 
 * OVERVIEW
 *    This procedure return the incremented value of queue pointer "s" 
 *
 ****************************************************************************/

static char *qnext (QUEUE *q, char *s)
{
  return (s == q->qbase + q->qsize - 1) ? q->qbase : s + 1;
}

/*****************************************************************************
 * 
 * NAME
 *    qprev
 * 
 * OVERVIEW
 *    This procedure return the decremented value of queue pointer "s" 
 *
 ****************************************************************************/

static char *qprev (QUEUE *q, char *s)
{
  return (s == q->qbase) ? q->qbase + q->qsize - 1 : s - 1;
}

/*****************************************************************************
 * 
 * NAME
 *    qnextline
 * 
 * OVERVIEW
 *    This procedure returns the line-incremented value of queue pointer 
 *
 ****************************************************************************/

static char *qnextline (QUEUE *q, char *s)
{
  int i;
  char ch;
  
  for (i = 0; i < q->qsize; i++) {
    ch = *s;
    s = qnext (q, s);
    if (ch == '\n')
      break;
  }
  return s;
}

void startqueue (SLOT *sp)
{
  sp->flags.stopped = 1;
}

void clearqueue (SLOT *sp)
{
  sp->flags.stopped = 0;
}

/*****************************************************************************
 * 
 * NAME
 *    qinsert
 * 
 * OVERVIEW
 *    This procedure inserts a character into a q
 *
 ****************************************************************************/

void qinsert (QUEUE *q, char ch)
{
  *q->qwrite = ch;
  q->qwrite = qnext (q, q->qwrite);

  if (q->qwrite == q->qread) {
    if (!q->overflow)
      q->overflow = strlen (overflow_msg);
    q->qread = qnextline (q, q->qread);
    if (q->qwrite == q->qread)
      q->qread = qprev (q, q->qread);
  }
}

/*****************************************************************************
 * 
 * NAME
 *    qempty
 * 
 * OVERVIEW
 *    This procedure nonzero if a queue is empty, or zero otherwise. 
 *
 ****************************************************************************/

int qempty (QUEUE *q)
{
  return (q->qread == q->qwrite);
}

/*****************************************************************************
 * 
 * NAME
 *    qflush
 * 
 * OVERVIEW
 *    This procedure makes a queue empty
 *
 ****************************************************************************/

void qflush (QUEUE *q)
{
  q->qread = q->qwrite = q->qbase;
}

/*****************************************************************************
 * 
 * NAME
 *    qdispose
 * 
 * OVERVIEW
 *    This procedure gives back memory
 *
 ****************************************************************************/

void qdispose (QUEUE *q)
{
  if (q->qbase != NULL) {
    free (q->qbase);
    q->qbase = NULL;
  }
}

/*****************************************************************************
 * 
 * NAME
 *    qcreate
 * 
 * OVERVIEW
 *    This procedure creates a queue
 *
 ****************************************************************************/

int qcreate (QUEUE *q, size_t sz)
{
  q->qbase = (char *) malloc (sz);
  if (q->qbase == NULL)
    return 0;

  q->qsize = sz;
  q->qread = q->qwrite = q->qbase;
  return 1;
}

/*****************************************************************************
 * 
 * NAME
 *    writequeue
 * 
 * OVERVIEW
 *    This procedure transmits data in queue q to slot
 *
 ****************************************************************************/

int writequeue (SLOT *sp, QUEUE *q)
{
  struct iovec iov[2];
  int ret = 0;

  /* First write a message if queue overflow had occured */
  if (q->overflow) {
    ret = write (sp->fd,
                 &overflow_msg[strlen(overflow_msg) - q->overflow],
                 q->overflow);
    if (ret < 0)
      return -1;
    q->overflow -= ret;
    if (q->overflow)
      return 0;
  }

  if (qempty (q))
    return 1;

  if (q->qread < q->qwrite) {
    ret = write (sp->fd, q->qread, q->qwrite - q->qread);
    if (ret > -1) {
      q->qread += ret;
    } else {
      /* was trying to fake out bad writes here, but
         for now it's better to just return -1 and then
         disconnect the session with the bad write 
       sp->outq.qread = sp->outq.qwrite;
       sp->stopq.qread = sp->stopq.qwrite;
      */
      return -1;
    }
  } else {
    iov[0].iov_base = q->qread;
    iov[0].iov_len = q->qsize - (q->qread - q->qbase);
    iov[1].iov_base = q->qbase;
    iov[1].iov_len = q->qwrite - q->qbase;
    ret = writev (sp->fd, iov, 2);
    if (ret > -1) {
      q->qread += ret;
    } else {
      /* was trying to fake out bad writes here, but
         for now it's better to just return -1 and then
         disconnect the session with the bad writev
       sp->outq.qread = sp->outq.qwrite;
       sp->stopq.qread = sp->stopq.qwrite;
      */
      return -1;
    }
    if (q->qread >= q->qbase + q->qsize)
      q->qread -= q->qsize;
  }
  return 1;
}

/*****************************************************************************
 * 
 * NAME
 *    qlength
 * 
 * OVERVIEW
 *    qlength returns the current length of a queue 
 *
 ****************************************************************************/

int qlength (QUEUE *q)
{
  if (q->qread > q->qwrite)
    return (q->qwrite + q->qsize - q->qread);
  else
    return (q->qwrite - q->qread);
}
