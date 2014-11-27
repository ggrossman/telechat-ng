/*****************************************************************************
 *
 * FILE
 *    admin.c
 *
 * CONTENTS
 *    This file contains a bunch of stuff for administrative commands, blah
 * 
 *    Copyright 1996 Hyperreal.  All rights reserved.  For more information
 *    consult the file LICENSE.
 * 
 *****************************************************************************/

#include "telechat.h"

static void kickoff2 (SLOT *sp, char *id);
static void op_func2 (char *ars);
static void secret2 (char *);
static void cmd_reset_pwd (SLOT *sp, char *id);
static void shutdown_server (char *answer);

/*****************************************************************************/

static void kickoff2 (SLOT *sp, char *id)
{
  if (cur_slot == sp) {
    writestr (cur_slot, "You cannot kick off yourself!\n");
  } else {    
    operact (cur_slot, sp, "kills");
    if (sp->flags.on) {
      writestr (sp, "\nGood bye.\n");
      transmit (sp);
      writestr (cur_slot, sp->acct.id);
      writestr (cur_slot, " kicked off.\n");
      paabout (sp, "Kicked off");
    }
    if (sp->acct.chan < NUMCHANNELS)
      remove_from_channel(sp, sp->acct.chan);
    remove_from_channel (sp, MAINLIST);
    sp->acct.last_logout = time (NULL);
    inactivate_slot (sp, 0);
  }
  setproc (sendpub, MAXMSG, 0);
}

/****************************************************************************/

void cmd_kickoff (void)
{
  writestr (cur_slot, "Who: ");
  setproc_id (kickoff2, PROC_ID_ONLINE);
}

/*****************************************************************************/

void inquire (void)
{
  writestr (cur_slot, "Who: ");
  setproc_id (acc_viewuser, PROC_ID_ALL);
}

/*****************************************************************************/

void show_status (void)
{
  time_t now;
  struct rusage rusage;
  
  writestr (cur_slot, "Software version           = ");
  writestr (cur_slot, VERSION);
  writestr (cur_slot, "\nLink Date                  = ");
  writestr (cur_slot, LinkDate);
  writestr (cur_slot, "\nProcess ID                 = ");
  writeint (cur_slot, getpid ());
  now = time (NULL);
  writestr (cur_slot, "\nCurrent time               = ");
  writestr (cur_slot, ctime (&now));
  writestr (cur_slot, "Program start up time      = ");
  writestr (cur_slot, ctime (&startup_time));
  writestr (cur_slot, "Total up time              = ");
  writetime (cur_slot, now - startup_time);
  writestr (cur_slot, "\nTotal logins since startup = ");
  writeint (cur_slot, login_count);
  writestr (cur_slot, "\nNumber of slots available  = ");
  writeint (cur_slot, max_slots);
  getrusage (RUSAGE_SELF, &rusage);
  writestr (cur_slot, "\nUser CPU time consumed     = ");
  writetime (cur_slot, rusage.ru_utime.tv_sec);
  writestr (cur_slot, "\nSystem CPU time consumed   = ");
  writetime (cur_slot, rusage.ru_stime.tv_sec);
  writestr (cur_slot, "\nTotal CPU time consumed    = ");
  writetime (cur_slot, rusage.ru_utime.tv_sec + rusage.ru_stime.tv_sec);
  writech (cur_slot, '\n');
  setproc (sendpub, MAXMSG, 0);
}

/*****************************************************************************/

static void op_func2 (char * ars)
{
  switch (*ars)
    {
    case 'A': 
    case 'a': 
      writestr (cur_slot, "<l>list, <v>iew, <c>hange level");
      if (!nodelete_f)
	writestr (cur_slot, ", <d>elete");
      if (verify_f)
        writestr (cur_slot, ", v<e>rify, <u>nverify");
      writestr (cur_slot, ": ");
      setproc (accounting, 2, COMPL_NONE);
      break;
      
    case 'I': 
    case 'i': 
      show_status ();
      break;

    case 'R':
    case 'r':
      if (chpasswd_f) {
	writestr (cur_slot, "Who's password do you want to reset? ");
        setproc_id (cmd_reset_pwd, PROC_ID_ALL);
      } else {
        setproc (sendpub, MAXMSG, 0);
      }
      break;

    case 'S': 
    case 's': 
      writestr (cur_slot,
                "This will shut down the server.\n"
                "Are you sure? [y/n] ");
      setproc (shutdown_server, 2, COMPL_NONE);
      break;
      
    default: 
      setproc (sendpub, MAXMSG, 0);
    }
}

/*****************************************************************************/

void op_func (void)
{
  if (cur_slot->acct.level == TOPLEVEL) {
    if (chpasswd_f) {
      writestr (cur_slot, "<a>ccounting, <i>nfo, <r>eset user password,"
                      " <s>hutdown: ");
    } else
      writestr (cur_slot, "<a>ccounting, <i>nfo, <s>hutdown: ");
    setproc (op_func2, 2, COMPL_NONE);
  }
}

/*****************************************************************************/

void cmd_toggle_email (void)
{
  cur_slot->acct.email_public = !cur_slot->acct.email_public;
  if (cur_slot->acct.email_public)
    writestr (cur_slot, "email address is now public\n");
  else
    writestr (cur_slot, "email address is now private\n");
}

/*****************************************************************************/

void cmd_level (void)
{
  writestr (cur_slot, "Password: ");
  setproc (secret2, MAXPW, READ_NOECHO);
}

/*****************************************************************************/

static void secret2 (char *pw)
{
  int res;
  SLOT *ss;
  SLOT_LIST *sl;

  res = 0;
  /* Not most efficient way, but this is rare operation */
  if (db_pwd_compare ("_level5", pw) == DB_SUCCESS)
    res = 5;
  else if (db_pwd_compare ("_level4", pw) == DB_SUCCESS)
    res = 4;
  else if (db_pwd_compare ("_level3", pw) == DB_SUCCESS)
    res = 3;
  else if (db_pwd_compare ("_level2", pw) == DB_SUCCESS)
    res = 2;

  if (res > cur_slot->acct.level) {
    writestr (cur_slot, "Access level changed to ");
    writeint (cur_slot, res);
    writestr (cur_slot, ".\n");
    cur_slot->acct.level = res;
    write_user (&(cur_slot->acct));

    sprintf (msg_buf, "%s moved to level %d", cur_slot->acct.id, res);
    write_log_system (msg_buf);

    sl = channels[MAINLIST].members;
    while(sl) {
      ss = sl->slot;
      if (ss && (ss->acct.level >= res) && ss != cur_slot) {
        select_wrap (ss);
        select_stop (ss);
        begin_color (ss, cur_slot->acct.msgcolor);
        begin_attr (ss, 1);
        writestr (ss, "** ");
        writestr (ss, msg_buf);
        reset_attr (ss);
        writech (ss, '\n');
        clear_wrap (ss);
        clear_stop (ss);
      }
      sl = sl->next;
    }
  } else
    writestr (cur_slot, "Level unchanged.\n");
  
  setproc (sendpub, MAXMSG, 0);
}

/****************************************************************************/

static void cmd_reset_pwd (SLOT *sp, char *id)
{
  char *the_id;

  the_id = (sp) ? sp->acct.id : id;
  if (db_pwd_change (the_id, crypt(the_id, salt)) == DB_SUCCESS) {
    writestr (cur_slot, "Password changed.\n");
    operact (cur_slot, (SLOT *)NULL, "changed password");
  } else
    writestr (cur_slot, "Password reset failed.\n");

  setproc (sendpub, MAXMSG, 0);
}

/*****************************************************************************/

static void shutdown_server (char *answer)
{
  if (answer[0] == 'y' || answer[0] == 'Y') {
    operact (cur_slot, (SLOT *)NULL, "initiated shut down");
    do_shutdown (0);
  }
  else
    setproc (sendpub, MAXMSG, 0);
}

/*****************************************************************************/

void do_shutdown (int signo)
{
  SLOT * sp;
  SLOT_LIST *sl;
  time_t now;
  
  now = time (NULL);
  sprintf (msg_buf, "Server shut down by %s at %.24s",
           cur_slot->acct.id, ctime (&now));
  write_log_system (msg_buf);
  
  sl = channels[MAINLIST].members;
  while (sl) {
    sp = sl->slot;
    if (sp->flags.on) {
      writestr (sp, "** Server is shutting down.\n");
      transmit (sp);
      inactivate_slot (sp, 0);
    }
    sl = sl->next;
    if (sp)
      sp = NULL;
  }
 
  close (listen_sock);
  close_logs();

  free (slotbase);
  free (closeslots);
  free (acct_fname);
  free (log_fname);
  free (prelogin_fname);
  free (postlogin_fname);
  free (pid_fname);

  exit (0);
  /* NOTREACHED */
}

