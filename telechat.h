/*****************************************************************************
 *
 * NAME
 *    telechat.h
 *
 * CONTENTS
 *    This file contains structure and header information for
 *    the chat system 
 *
 *    Copyright 1996 Hyperreal.  All rights reserved.  For more information
 *    consult the file LICENSE.
 *
 *****************************************************************************/

#define _XOPEN_SOURCE           /* To get crypt() prototype in GNU libc */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/resource.h>
#include <sys/signal.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <math.h>
#include <sys/types.h>
#include <limits.h>
#include <locale.h>

#include "config.h"

/* Lengths of strings used in the program */
#define MAXINPUT  511		/* Maximum length of any input */

#define MAXID     21		/* Maximum length of a user ID */
#define MAXPW     13		/* Maximum length of a password */

#define MAXHANDLE 33		/* Maximum length of a handle */
#define MAXMSG    511		/* Maximum length of a message */
#define MAXFMT    64		/* Maximum length of a format string */
#define MAXHOST   63		/* Maximum length of a hostname */
#define MAXEMAIL  256

/* The size of the screen */
#define MAXWIDTH 132
#define MINWIDTH 20

/* Character-oriented queue for output buffers.
   When it overflows, it "eats" the old data line by line.
   The overflow field keeps a number of characters left to be
   printed from overflow_msg message in case of overflow. */
typedef struct {
  char *qbase, *qread, *qwrite;
  int   qsize;
  int   overflow;
} QUEUE;

/* Structure to keep a list of username completions available */
typedef struct {
  int charidx;                  /* Index of first char of current variant */
  int current;                  /* Number of current variant */
  int total;                    /* Total variants */
  char names[1];                /* A sequence of null-terminated tails */
} COMPLS;

/* Structure for user accounts. */
typedef struct {
  char id[MAXID];
  char handle[MAXHANDLE];
  int  chan;
  int  width;
  int  level;
  char nlchar;
  unsigned int pa_notify: 1,
               nostat:    1,
               newlines:  1,
               beeping:   1,
               usecolor:  1,
               msgcolor:  3;
  char activefmt[MAXFMT];
  char msgfmt[MAXFMT];
  char email_address[MAXEMAIL];
  int  email_verified;
  int  email_public;
  char created_from[MAXEMAIL];    /* what machine was the person
				     on when the acct was created? */
  time_t last_logout;
  time_t last_typed;               
  
} ACCOUNT;

/* Definition of SLOT data structure associated with each connection. */
typedef struct slot_struct
{
  int     fd;                   /* file descriptor */
  int     nslot;                /* unique slot number */
  void  (*readfunc)();
  int     tsmode;
  ACCOUNT acct;
  time_t  login_time;
  time_t  last_typed;           /* the last time this person typed something */
  time_t  chan_time;            /* time the user joined his current channel */
  fd_set  squelch, reverse;
  int     inmax;
  char    wrap_base[MAXWIDTH + 1];
  char   *wrap_ptr;
  char   *last_roll;
  QUEUE   outq;
  QUEUE   stopq;
  char    in[MAXINPUT + 1];
  char   *inp;
  COMPLS *compls;               /* A list of available id completions */
  char    offmsg_to[MAXID];     /* They are sending offline msg to */
  char    pmail[MAXID];		/* They are sending pmail to */
  char    pmail_rcvd[MAXID];    /* Last one they got pmail from */
  char    pmail_sent[MAXID];    /* Last person slot sent pmail to */
  void  (*dispid)(struct slot_struct *sp, char *id);
  char   *temp;
  char    hostname[MAXHOST + 1];
  struct {
    unsigned int on:      1,	/* Set if user is logged in */
                 wrap:    1,	/* Set if word wrap is on */
                 stopped: 1,	/* Set if incoming input is stopped */
                 gotcr:   1,	/* Set if CR was received */
                 echo:    1,	/* Set if input echos back to user */
                 wstop:   1,	/* Set if writing to stop queue */
                 listed:  1,	/* Set if channels were listed before
                                   getting on a named channel */
                 extid:   1,    /* Set if proc_id() should look up for
                                   offline users too */
                 die:     1,    /* Set if this slot should be destroyed */
                 nosave:  1,	/* Set if we should not save account
                                   data when connection is closed */
                 compl:   2;    /* Tab completion mode: COMPL_DEFAULT,
                                   COMPL_STRICT, COMPL_NONE */
  } flags;
} SLOT;

/* prototypes */
#include "prototypes.h"

/* database access */
#include "db.h"

/* Number of user-usable channels */
#define NUMCHANNELS 100
#define MAINLIST NUMCHANNELS

/* Size of output and stopped output queues */
#define MAXSTOPQ 8192
#define MAXOUTQ 4096

/* The size of a message buffer */
#define MAXBUF 1024

/* Version information */

#define VERSION    "Telechat-ng 0.9.5"
#define COPYRIGHT  "Copyright (c) 1996 Hyperreal.\n" \
                   "Copyright (c) 2000 Const Kaplinsky."

extern char *salt;

extern char *LinkDate; /* created by Makefile */

/* Telnet control sequence processing */
#define TS_NONE	0
#define TS_IAC	1
#define TS_WILL	2
#define TS_WONT	3
#define TS_DO	4
#define TS_DONT	5

/* externs */

/* run-time configuration, config.c */
extern char base_dir[];
extern char *base_dir_prefix;

extern int verify_f;
extern int nodelete_f;
extern int nohostname_f;
extern int newusers_f;
extern int slots_f;
extern int roomtyping_f;
extern int chpasswd_f;
extern int icomplete_f;

extern int idle_timeout;

extern int max_users;
extern int max_slots;
extern int max_files;

extern int max_msgfile;

extern char *bad_logins[];
extern int num_bad_logins;

/* file names */
extern FILE *user_db;
extern char *acct_new_fname;
extern char *log_fname;
extern FILE *log_fp;
extern char *prelogin_fname;
extern char *postlogin_fname;
extern char *pid_fname;

/* main.c: */
extern int listen_sock;
extern char msg_buf[];
extern SLOT **slotbase;
extern SLOT *cur_slot;
extern int slots_used;
extern int maxfd_used;
extern SLOT **closeslots;
extern int closeslots_num;
extern time_t startup_time;
extern int login_count;
extern int cur_loggedin;
extern fd_set used;
extern int logout_idle_flag;
extern int inet_port;

/* channels.c */

typedef struct slot_list {
  SLOT *slot;
  struct slot_list *next;
} SLOT_LIST;

typedef struct {
  char *name;
  int count;
  time_t last_named;
  SLOT_LIST *members;
  SLOT_LIST *last_mem;          /*** ??? ***/
} CHANNEL;

extern CHANNEL channels[];

/* constants */

#define NORMAL          0
#define PA_MSG          0
#define USR_MSG         1
#define CHAN_NAME       2
#define CHAN_UNNAME     3

#define NOT_LOGGED_IN " not logged in.\n"

#define VAL_OK          0
#define VAL_RESERVED   -1
#define VAL_INVALID    -2

/* To use as the last setproc() argument */
#define COMPL_DEFAULT   0
#define COMPL_STRICT    1
#define COMPL_NONE      2
#define COMPL_MASK      3
#define READ_NOECHO     4

/* To use as the last setproc_id() argument */
#define PROC_ID_ONLINE  0
#define PROC_ID_ALL     1

