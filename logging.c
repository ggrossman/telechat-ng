/*******************************************************************
 *
 * $Id: logging.c,v 1.5 2000/05/29 15:20:43 const Exp $
 * Logging functionality.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/

#include "telechat.h"

/*******************************************************************
 *
 *  Write to the chat log file.
 *    msg: the string to write to the log.
 *
 *******************************************************************/

void write_log (char *msg)
{
  if (cur_slot && cur_slot->acct.chan == 1) {
    fprintf (log_fp, "(01) %s: %s\n", cur_slot->acct.id, msg);
    fflush (log_fp);
  }
}

/*******************************************************************
 *
 *  Write system message (no channel/username) to the chat log file.
 *    msg: the string to write to the log.
 *
 *******************************************************************/

void write_log_system (char *msg)
{
  fprintf (log_fp, "*** %s\n", msg);
  fflush (log_fp);
}

/*******************************************************************
 *
 *  Write emote to the chat log file.
 *    msg: the string to write to the log.
 *
 *******************************************************************/

void write_log_emote (char *msg)
{
  if (cur_slot && cur_slot->acct.chan == 1) {
    fprintf (log_fp, "(01) ** %s %s **\n",
             cur_slot->acct.id, msg);
    fflush (log_fp);
  }
}

/*******************************************************************
 *
 *  Write a broadcast message to the chat log file.
 *    msg: the string to write to the log.
 *
 *******************************************************************/

void write_log_bcast (char *msg)
{
  if (cur_slot) {
    fprintf (log_fp, "(**) %s: [Broadcast] %s\n",
             cur_slot->acct.id, msg);
    fflush (log_fp);
  }
}

/*******************************************************************
 *
 *  Write a broadcast emote to the chat log file.
 *    msg: the string to write to the log.
 *
 *******************************************************************/

void write_log_bcast_emote (char *msg)
{
  if (cur_slot) {
    fprintf (log_fp, "(**) [Broadcast] ** %s %s **\n",
             cur_slot->acct.id, msg);
    fflush (log_fp);
  }
}

/*******************************************************************
 *
 *  Write a PA message to the chat log file.
 *    slot:  number of slot caused this message to appear;
 *    msg: the string to write to the log.
 *
 *******************************************************************/

void write_log_paabout (SLOT *sp, char *msg)
{
  time_t now;

  if (sp) {
    now = time (NULL);
    fprintf (log_fp, "--- %s channel %02d: %s/%s [%s] %s",
             msg, sp->acct.chan, sp->acct.id, sp->acct.handle,
             sp->hostname, ctime (&now));
    fflush (log_fp);
  }
}

/*******************************************************************
 *
 *  Open log files.
 *
 *******************************************************************/

void open_logs (void) /* NOT ok */
{
  log_fp = fopen (log_fname, "a");
  if (log_fp == NULL) {
    printf ("Failed to open log file! Logging disabled.\n");
    log_fp = fopen ("/dev/null", "w");
  }
}

/*******************************************************************
 *
 *  Close log files.
 *
 *******************************************************************/

void close_logs (void)
{
  fclose (log_fp);
}

/*******************************************************************
 *
 *  Reopen log files.
 *
 *******************************************************************/

void reopen_logs (void)
{
  close_logs();
  open_logs();
}

