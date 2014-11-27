/*****************************************************************************
 *
 * NAME
 *     broadcast.c
 *
 * CONTENTS
 *   This file contains the code for broadcasting.
 *
 *    Copyright 1996 Hyperreal.  All rights reserved.  For more information
 *    consult the file LICENSE.
 * 
 *****************************************************************************/

#ifndef NOBROADCAST

#include "telechat.h"

static void cmd_broadcast2 (char *msg);
static void cmd_station2 (char *msg);

void cmd_broadcast (void)
{
  writestr (cur_slot, "Msg: ");
  setproc (cmd_broadcast2, MAXMSG, 0);
}

static void cmd_broadcast2 (char *msg)
{
  SLOT *sp;
  SLOT_LIST *sl;
  
  if (msg[0]) {
    sl = channels[MAINLIST].members;
    while (sl) {
      sp = sl->slot;
      if (sp) {                 /*** ??? ***/
        select_stop (sp);
        select_wrap (sp);
        begin_attr (sp, 1);     /* Intensive */
        begin_attr (sp, 5);     /* Blinking  */
        writestr (sp, "-- ");
        writestr (sp, msg);
        reset_attr (sp);
        writech (sp, '\n');
        clear_stop (sp);
        clear_wrap (sp);
      }
      sl = sl->next;
    }
    writestr (cur_slot, "Message sent.\n");
  }
  setproc (sendpub, MAXMSG, 0);
}

void cmd_station (void)
{
  writestr (cur_slot, "Msg: ");
  setproc (cmd_station2, MAXMSG, 0);
}

static void cmd_station2 (char *msg)
{
  SLOT * sp;
  SLOT_LIST *sl;

  write_log_bcast (msg);
  if (msg[0]) {
    sl = channels[MAINLIST].members;
    while (sl) {
      sp = sl->slot;
      if (!sp->acct.nostat &&
          !squelched (sp, cur_slot) && !reversed (cur_slot, sp))
        writemsg (sp, msg, 2);
      sl = sl->next;
    }
  }
  setproc (sendpub, MAXMSG, 0);
}

#endif /* ifndef NOBROADCAST */
