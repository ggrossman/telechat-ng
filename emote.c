/*******************************************************************
 *
 * $Id: emote.c,v 1.8 2000/05/29 15:20:43 const Exp $
 * Emote functionality.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/

#include "telechat.h"

static void cmd_emote2 (char *str);
static void cmd_broadcast_emote2 (char *str);

void cmd_emote (void)
{
  writestr (cur_slot, "Emote: ");
  setproc (cmd_emote2, MAXWIDTH, 0);
}

static void cmd_emote2 (char *str)
{
  if (str[0]) {
    sprintf (msg_buf, "** %s %s **", cur_slot->acct.id, str);
    write_channel (msg_buf, cur_slot->acct.chan, PA_MSG);
    write_log_emote (str);
  }
  setproc (sendpub, MAXMSG, 0);
}

void cmd_broadcast_emote (void)
{
  writestr (cur_slot, "Emote: ");
  setproc (cmd_broadcast_emote2, MAXWIDTH, 0);
}

static void cmd_broadcast_emote2 (char *str)
{
  SLOT *sp;
  SLOT_LIST *members;

  if (str[0]) {
    sprintf (msg_buf, "[Broadcast] ** %s %s **", cur_slot->acct.id, str);
    members = channels[MAINLIST].members;
    while (members) {
      sp = members->slot;
      if (!sp->acct.nostat &&
          !squelched (sp, cur_slot) && !reversed (cur_slot, sp)) {
        select_stop (sp);
        select_wrap (sp);
        begin_color (sp, cur_slot->acct.msgcolor);
        writestr (sp, msg_buf);
        reset_attr (sp);
        writech (sp, '\n');
        clear_stop (sp);
        clear_wrap (sp);
      }
      members = members->next;
    }
    write_log_bcast_emote (str);
  }

  setproc (sendpub, MAXMSG, 0);
}
