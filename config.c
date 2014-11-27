/*******************************************************************
 *
 * $Id: config.c,v 1.11 2000/06/08 15:38:52 const Exp $
 * Run-time configuration handling.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/

#include "telechat.h"

char base_dir[PATH_MAX+1];      /* Base directory for resource files */
char *base_dir_prefix;          /* Empty string if base_dir is "/",
                                   otherwise the same as base_dir */

/* These variables should be read from configuration file */

int verify_f     = 1;
int nodelete_f   = 0;
int nohostname_f = 1;
int newusers_f   = 1;
int slots_f      = 1;
int roomtyping_f = 1;
int chpasswd_f   = 1;           /* If 1, users may change their passwords */
int icomplete_f  = 1;           /* If 1, incremental completion mode is on */

int idle_timeout = 1800;        /* Half an hour */

int max_users = 256;            /* Limit the number of online users */
int max_slots = 300;            /* Limit the number of connections */
int max_files = 350;            /* Limit the number of file descriptors */

int max_msgfile = 50;           /* Maximum size of all offline mail, Kb */

char *bad_logins[] = {
  "who", "recent", "quit", "exit", "bye", "end", "login", "logout",
  "guest", "nobody", "anonymous", "user", "newuser", "new",
  "fuck", "shit", "crap"
};
int num_bad_logins = sizeof(bad_logins)/sizeof(char *);

/*******************************************************************
 *
 *  This function should read runtime configuration, but currently it
 *  just copies BASE_DIR into base_dir and base_dir_prefix global
 *  variables, stripping any terminating "/" characters (if the string
 *  consists of the only '/', it remains in base_dir but not in the
 *  base_dir_prefix).
 *
 *  Return values: 1 on success, 0 otherwise.
 *
 *  Note: error messages are printed to stderr.
 *
 *******************************************************************/

int read_conf (void)
{
  int len;

  len = strlen(BASE_DIR);
  if (len > PATH_MAX) {
    fprintf (stderr, "BASE_DIR path is too long.\n");
    return 0;
  }

  strcpy (base_dir, BASE_DIR);
  while (len) {
    if (base_dir[len-1] == '/')
      len--;
    else
      break;
  }
  if (len) {
    base_dir_prefix = base_dir;
  } else {
    base_dir_prefix = "\0";
    base_dir[len++] = '/';
  }
  base_dir[len] = '\0';
  return 1;
}

