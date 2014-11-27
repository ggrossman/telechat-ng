/*******************************************************************
 *
 * $Id: typing.c,v 1.10 2000/06/01 05:57:38 const Exp $
 * Code to show users currently typing.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/

#include "telechat.h"

static void cmd_room_typing2 (char *room);
static void print_typing (SLOT *sp);

void cmd_typing (void)
{
  SLOT *sp;
  int i;
  int nobody = 1;

  for (i = 0; i <= slots_used; i++) {
    sp = slotbase[i];
    if (!sp || sp == cur_slot || !sp->flags.on || !sp->flags.stopped)
      continue;
    if ((squelched (cur_slot, sp) || reversed (sp, cur_slot)) &&
        cur_slot->acct.level != TOPLEVEL)
      continue;
    if (nobody) {
      writestr (cur_slot, "Users currently typing:\n");
      nobody = 0;
    }
    print_typing (sp);
  }
  if (nobody)
    writestr (cur_slot, "Nobody is currently typing.\n");
}

void cmd_room_typing (void)
{
  writestr (cur_slot, "Room number [default is the current room]: ");
  setproc (cmd_room_typing2, 3, COMPL_NONE);
}

static void cmd_room_typing2 (char *room)
{
  int r;
  SLOT_LIST *sl;
  SLOT *sp;
  int nobody = 1;

  if (room[0]) {
    r = atoi (room);
    if (r < 1 || r > 99) {
      writestr (cur_slot, "Incorrect room number.\n");
      setproc (sendpub, MAXMSG, 0);
      return;
    }
  } else
    r = cur_slot->acct.chan;

  if (r != cur_slot->acct.chan &&
      !channels[r].name &&
      cur_slot->acct.level != TOPLEVEL) {
    writestr (cur_slot, "Sorry, you cannot look into that room.\n");
    setproc (sendpub, MAXMSG, 0);
    return;
  }

  for (sl = channels[r].members; sl; sl = sl->next) {
    sp = sl->slot;
    if (sp == cur_slot || !sp->flags.stopped)
      continue;
    if ((squelched (cur_slot, sp) || reversed (sp, cur_slot)) &&
        cur_slot->acct.level != TOPLEVEL)
      continue;
    if (nobody) {
      if (channels[r].name)
        sprintf (msg_buf,
                 "Users currently typing in room %02d (%s):\n",
                 r, channels[r].name);
      else
        sprintf (msg_buf,
                 "Users currently typing in room %02d (unnamed):\n", r);
      writestr (cur_slot, msg_buf);
      nobody = 0;
    }
    print_typing (sp);
  }

  if (nobody) {
    if (channels[r].name)
      sprintf (msg_buf, "Nobody is typing in room %02d (%s).\n",
               r, channels[r].name);
    else
      sprintf (msg_buf, "Nobody is typing in room %02d (unnamed).\n", r);
    writestr (cur_slot, msg_buf);
  }

  setproc (sendpub, MAXMSG, 0);
}

static void print_typing (SLOT *sp)
{
  sprintf (msg_buf, "%s/%s\n", sp->acct.id, sp->acct.handle);
  writestr (cur_slot, "  ");
  begin_color (cur_slot, sp->acct.msgcolor);
  writestr (cur_slot, msg_buf);
  reset_attr (cur_slot);
}

