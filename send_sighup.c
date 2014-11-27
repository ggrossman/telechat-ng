/*******************************************************************
 *
 * $Id: send_sighup.c,v 1.4 2000/06/11 08:09:46 const Exp $
 * Utility to send SIGHUP signal to the chat daemon running on the
 * default port so that it will reopen its log file.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 *
 *******************************************************************/

#include "telechat.h"

int main (int argc, char *argv[])
{
  FILE *fp;
  long pid;
  char *name;

  if (!read_conf()) {
    fprintf (stderr, "%s: Cannot read run-time configuration.\n", argv[0]);
    return 1;
  }

  name = (char *) malloc (strlen(base_dir_prefix) + 20);
  if (name == NULL) {
    fprintf (stderr, "%s: Cannot allocate memory.\n", argv[0]);
    return 1;
  }

  sprintf (name, "%s/.pid.%d", base_dir_prefix, DEF_PORT);
  fp = fopen (name, "r");
  free (name);

  if (fp == NULL || fscanf (fp, "%d", &pid) != 1 || pid <= 1) {
    if (fp != NULL)
      fclose (fp);
    fprintf (stderr, "%s: Cannot read the PID of the daemon.\n", argv[0]);
    return 1;
  }

  fclose (fp);

  if (kill (pid, SIGHUP)) {
    fprintf (stderr, "%s: Cannot send a signal to the daemon.\n", argv[0]);
    return 1;
  }

  return 0;
}

