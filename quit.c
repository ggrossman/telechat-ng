/*****************************************************************************
 *
 * NAME
 *     quit.c
 *
 * CONTENTS
 *   This file contains the code for quitting the server.
 *
 *    Copyright 1996 Hyperreal.  All rights reserved.  For more information
 *    consult the file LICENSE.
 * 
 *****************************************************************************/

#include "telechat.h"

static void cmd_logoff2 (char *ans);
static void cmd_logoff3 (char *ans);
static void do_quit (void);

void cmd_logoff (void)
{
  writestr (cur_slot, "Are you sure? [y/n] ");
  setproc (cmd_logoff2, 2, COMPL_NONE);
}

static void cmd_logoff2 (char *ans)
{
  SLOT *sp;
  SLOT_LIST *sl;

  if (ans[0] != 'y' && ans[0] != 'Y') {
    setproc (sendpub, MAXMSG, 0);
    return;
  }

  sl = channels[MAINLIST].members;
  while (sl) {
    sp = sl->slot;
    if (!strcmp (sp->pmail, cur_slot->acct.id)) {
      writestr (cur_slot,
                "Someone is sending you a private message.\n"
                "Are you sure you want to quit? [y/n] ");
      setproc (cmd_logoff3, 2, COMPL_NONE);
      return;
    }
    sl = sl->next;
  }
  do_quit();
}

static void cmd_logoff3 (char *ans)
{
  if (ans[0] == 'y' || ans[0] == 'Y')
    do_quit();
  else
    setproc (sendpub, MAXMSG, 0);
}

static void do_quit (void)
{
  paabout (cur_slot, "Logged off");

  writech (cur_slot, '\n');
  transmit (cur_slot);
  remove_from_channel (cur_slot, cur_slot->acct.chan);
  remove_from_channel (cur_slot, MAINLIST);
  cur_slot->acct.last_logout = time (NULL);
  inactivate_slot (cur_slot, 0);
}

