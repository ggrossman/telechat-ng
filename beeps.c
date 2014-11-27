/*******************************************************************
 *
 * $Id: beeps.c,v 1.10 2000/06/05 04:13:34 const Exp $
 * Beeping functionality.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/

#include "telechat.h"

static void cmd_beep2 (SLOT *dst_slot, char *id);

void cmd_togglebeeps (void)
{
  cur_slot->acct.beeping = !cur_slot->acct.beeping;
  if (cur_slot->acct.beeping)
    writestr (cur_slot, "Your beeping mode is now ON.\n");
  else
    writestr (cur_slot, "Your beeping mode is now OFF.\n");
}

void cmd_beep (void)
{
  writestr (cur_slot, "To: ");
  setproc_id (cmd_beep2, PROC_ID_ONLINE);
}

static void cmd_beep2 (SLOT *dst_slot, char *id)
{
  char beep_code[] = "\a";

  if (!squelched (dst_slot, cur_slot) && !reversed (cur_slot, dst_slot)) {
    sprintf (msg_buf, " * You are being *beeped* by %s/%s",
             cur_slot->acct.id, cur_slot->acct.handle);
    if (dst_slot->acct.beeping) {
      writestr (dst_slot, beep_code);
    } else
      strcat (msg_buf, "\n * BEEEEEEEEEEEEEEEEP!!!");
    writeaction (dst_slot, msg_buf);
  }
  writestr (cur_slot, "Beep sent.\n");
  setproc (sendpub, MAXMSG, 0);
}

