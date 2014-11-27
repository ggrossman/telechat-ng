/*******************************************************************
 *
 * $Id: squelch.c,v 1.5 2000/06/02 12:20:45 const Exp $
 * Squelching functionality.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 * Copyright 1996 Hyperreal.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/

#include "telechat.h"

static void cmd_squelch2 (SLOT *sp, char *id);
static void cmd_rsquelch2 (SLOT *sp, char *id);

int squelched (SLOT *sp, SLOT *sq)
{
  return FD_ISSET(sq->nslot, &sp->squelch);
}

int reversed (SLOT *sp, SLOT *sq)
{
  return FD_ISSET(sq->nslot, &sp->reverse);
}

void cmd_squelch (void)
{
  writestr (cur_slot, "Who: ");
  setproc_id (cmd_squelch2, PROC_ID_ONLINE);
}

static void cmd_squelch2 (SLOT *sp, char *id)
{
  if (!FD_ISSET (sp->nslot, &cur_slot->squelch)) {
    FD_SET (sp->nslot, &cur_slot->squelch);
    writestr (cur_slot, "User squelched.\n");
    operact (cur_slot, sp, "squelches");
  } else {
    FD_CLR (sp->nslot, &cur_slot->squelch);
    writestr (cur_slot, "User unsquelched.\n");
    operact (cur_slot, sp, "unsquelches");
  }
  setproc (sendpub, MAXMSG, 0);
}

void cmd_rsquelch (void)
{
  writestr (cur_slot, "Who: ");
  setproc_id (cmd_rsquelch2, PROC_ID_ONLINE);
}

static void cmd_rsquelch2 (SLOT *sp, char *id)
{
  if (!FD_ISSET (sp->nslot, &cur_slot->reverse)) {
    FD_SET (sp->nslot, &cur_slot->reverse);
    writestr (cur_slot, "User reversed.\n");
    operact (cur_slot, sp, "reverses");
  } else {
    FD_CLR (sp->nslot, &cur_slot->reverse);
    writestr (cur_slot, "User unreversed.\n");
    operact (cur_slot, sp, "unreverses");
  }
  setproc (sendpub, MAXMSG, 0);
}

