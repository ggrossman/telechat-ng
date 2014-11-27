/*****************************************************************************
 *
 * NAME
 *   init.c
 *
 * CONTENTS
 *    This file contains the intialization routines for the chat server.
 *
 *    Copyright 1996 Hyperreal.  All rights reserved.  For more information
 *    consult the file LICENSE.
 * 
 *****************************************************************************/

#include "telechat.h"

/* the files */
char *prelogin_fname;
char *postlogin_fname;
char *log_fname;
char *pid_fname;

FILE *log_fp;

/*****************************************************************************
 *
 * NAME
 *     initslots()
 *
 * OVERVIEW
 *     initslots clears out the slot table.  It should be called only
 *     once, at the beginning of the program, in order to create the
 *     slot table and blank it out. 
 *
 ****************************************************************************/

void initslots (void)
{
  int i;

  slotbase = (SLOT **) malloc ((max_slots + 1) * sizeof(SLOT *));
  closeslots = (SLOT **) malloc (max_slots * sizeof(SLOT *));

  if (slotbase == NULL || closeslots == NULL)
    panic ("telechat-ng: malloc");

  for (i = 0; i <= max_slots; i++)
    slotbase[i] = NULL;

  slots_used = 0;
  closeslots_num = 0;
}

/*****************************************************************************
 *
 * NAME
 *    initsock()
 *
 * OVERVIEW
 *    This procedure installs the socket the server uses.
 *
 *****************************************************************************/

void initsock (void)
{
  struct sockaddr_in sockaddr;
  int one = 1;
  int sockbufsize = 6144;
  
  /* the 0 below stands for IP protocol */
  if ((listen_sock = socket (PF_INET, SOCK_STREAM, 0)) < 0)
    panic ("telechat-ng: socket");
  else 
    printf ("socket = %d\n", listen_sock);

  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = INADDR_ANY;
  sockaddr.sin_port = htons (inet_port);

  if (setsockopt (listen_sock, SOL_SOCKET, SO_REUSEADDR, 
                  (char *) &one, sizeof(int)) < 0)
    panic ("telechat-ng: setsockopt SO_REUSEADDR");

  if (setsockopt (listen_sock, SOL_SOCKET, SO_SNDBUF, 
                  (char *) &sockbufsize, sizeof(int)) < 0)
    panic ("telechat-ng: setsockopt SO_SNDBUF");

  if (setsockopt (listen_sock, SOL_SOCKET, SO_RCVBUF, 
                  (char *) &sockbufsize, sizeof(int)) < 0)
    panic ("telechat-ng: setsockopt SO_RCVBUF");

  if (bind (listen_sock, (struct sockaddr *) &sockaddr,
            sizeof (struct sockaddr)) < 0)
    panic ("telechat-ng: bind");

  if (fcntl (listen_sock, F_SETFL, O_NDELAY) < 0)
    panic ("telechat-ng: fcntl");

  /* listen on the socket, allow up to eight connections to
     backlog while we deal with a single connection */
  if (listen (listen_sock, 8) < 0)
    panic ("telechat-ng: listen");

  maxfd_used = listen_sock;
}

/*****************************************************************************
 *
 * NAME
 *    initvars()
 *
 * OVERVIEW
 *    This procedure initializes global variables
 *
 *****************************************************************************/

void initvars (void)
{
  int path_len;

  printf ("Initializing variables...\n");
  
  /* first, initialize the file names */
  db_set_filenames (base_dir_prefix, ACCT_PASSWORDS, ACCT_FILENAME);
  printf ("Passwords in %s\n", acct_passwords);
  printf ("Account database in %s\n", acct_fname);

  path_len = strlen (base_dir_prefix);

  log_fname =
    (char *) malloc (path_len + strlen (LOG_FILENAME) + 8);
  prelogin_fname =
    (char *) malloc (path_len + strlen (PRELOGIN_FILENAME) + 2);
  postlogin_fname =
    (char *) malloc (path_len + strlen (POSTLOGIN_FILENAME) + 2);
  pid_fname =
    (char *) malloc (path_len + 11);

  sprintf (log_fname, "%s/%s.%d", base_dir_prefix, LOG_FILENAME, inet_port);
  sprintf (prelogin_fname, "%s/%s", base_dir_prefix, PRELOGIN_FILENAME);
  sprintf (postlogin_fname, "%s/%s", base_dir_prefix, POSTLOGIN_FILENAME);
  sprintf (pid_fname, "%s/.pid.%d", base_dir_prefix, inet_port);

  printf ("Logging information in %s\n", log_fname);
  printf ("prelogin information in %s\n", prelogin_fname);
  printf ("postlogin information in %s\n", postlogin_fname);
  printf ("Done initializing variables...\n");
}

void initchannels (void)
{
  int i;

  for (i = 0; i <= NUMCHANNELS; i++) {
    channels[i].name = NULL;
    channels[i].count = 0;
    channels[i].last_named = 0;
    channels[i].members = NULL;
    channels[i].last_mem = NULL;
  }

  channels[1].name = (char *) malloc (strlen(CHANNEL_01_NAME) + 1);
  strcpy(channels[1].name, CHANNEL_01_NAME);

#ifdef CHANNEL_02_NAME
  channels[2].name = (char *) malloc (strlen(CHANNEL_02_NAME) + 1);
  strcpy(channels[2].name, CHANNEL_02_NAME);
#endif
#ifdef CHANNEL_03_NAME
  channels[3].name = (char *) malloc (strlen(CHANNEL_03_NAME) + 1);
  strcpy(channels[3].name, CHANNEL_03_NAME);
#endif
}

