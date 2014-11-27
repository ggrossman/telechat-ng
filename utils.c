/*****************************************************************************
 *
 * NAME
 *    utils.c
 *
 * CONTENTS
 *    This file contains general utility functions for the chat server.
 *
 *    Copyright 1996 Hyperreal.  All rights reserved.  For more information
 *    consult the file LICENSE.
 * 
 *****************************************************************************/
/***                Remove this file before 0.9.6                          ***/

#include "telechat.h"

/*****************************************************************************
 * 
 * NAME
 *    xatoi()
 *
 * OVERVIEW
 *     converts an ASCII string to a positive integer, returns -1 on error 
 *
 ****************************************************************************/

int xatoi (char *s) /* ok, but won't work correctly with very large values */
{
  int i;
  int n = 0;

  for (i = 0; isdigit(s[i]) && n >= 0; i++)
    n = n * 10 + s[i] - '0';

  return (i && s[i] == '\0' && n >= 0) ? n : -1;
}

/******************************************************************************
 * 
 * NAME
 *    panic()
 *   
 * OVERVIEW
 *    panic is called to report a fatal error and exit while the program
 *    is installing.
 *
 ***************************************************************************/

void panic (char *msg)
{
  perror (msg);
  exit (1);
}

/*****************************************************************************
 * 
 * NAME
 *    closestd()
 *
 * OVERVIEW
 *     Closes the standard input, output and error files, since a daemon
 *     obviously has no use for them. 
 *
 ****************************************************************************/

void closestd (void)
{
  /* PROBLEM -- when I picked up the code this function did 
	(void) close (stdin);
	(void) close (stdout);
	(void) close (stderr);
     which technically does not work . . . stdin is a file POINTER,
     not a file descriptor, so the files weren't REALLY getting closed.
     So, I changed the code to use fclose to really close the files, and
     that seems to have introduced some odd bugs into the server.  One of them
     is that sometimes when someone logs off, their logout message gets
     printed multiple times.

     So, to get rid of that bug, I'm changing this function back to 
     use close on stdin/stdout/stderr until I figure out what it really 
     is supposed to do.
  */

/* I have to comment out these things for a while (const)
  (void) close (stdin);
  (void) close (stdout);
  (void) close (stderr);
*/
}

