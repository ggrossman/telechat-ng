/*******************************************************************
 *
 * $Id: userinfo.c,v 1.6 2000/06/11 08:25:53 const Exp $
 * Dumping information about registered users.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 * Copyright 1996 Hyperreal.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/
/***                     Rewrite before 0.9.6                    ***/

#include "telechat.h"

void dump_userinfoshort (ACCOUNT *acct, int logged_in)
{
  ACCOUNT *cur_acct;
  long idle, hours, mins, secs;

  cur_acct = &cur_slot->acct;

  writestr (cur_slot, "Login/Handle: ");
  begin_color (cur_slot, acct->msgcolor);
  writestr (cur_slot, acct->id);
  writech (cur_slot, '/');
  writestr (cur_slot, acct->handle);
  reset_attr (cur_slot);

  writestr (cur_slot, "\nChannel: ");
  if (cur_acct->level == TOPLEVEL  ||
      channels[acct->chan].name    ||
      acct->chan == cur_acct->chan ||
      !strcmp(acct->id, cur_acct->id)) {
    writeint (cur_slot, acct->chan);
    writestr (cur_slot, "\n");
  } else
    writestr (cur_slot, "**\n");

  if (cur_acct->level >= 2 ||
      acct->email_public   ||
      !strcmp(acct->id, cur_acct->id)) {
    writestr (cur_slot, "Email address is ");
    writestr (cur_slot, acct->email_address);
    writestr (cur_slot, acct->email_public ? " [public]\n" : " [private]\n");
  } else
    writestr (cur_slot, "Email address is *PRIVATE*\n");

  writestr (cur_slot, "Last logout: ");
  writestr (cur_slot, ctime(&acct->last_logout));

  if (logged_in) {
    /* calculate idle time if the user is logged in */
    idle = time (NULL) - acct->last_typed;
    hours = idle / 3600;
    idle -= hours * 3600;
    mins = idle / 60;
    secs = idle - (mins * 60);
    sprintf (msg_buf, "Last typed: %.24s (idle: %dh %02dm %02ds)\n",
             ctime(&acct->last_typed), hours, mins, secs);
    writestr (cur_slot, msg_buf);
  } else {
    writestr (cur_slot, "Last typed: ");
    writestr (cur_slot, ctime(&acct->last_typed));
  }
}

void dump_userinfo (SLOT *sp)
{
  ACCOUNT *acct;
  SLOT *sq;
  int i;

  acct = &sp->acct;

  dump_userinfoshort (acct, 1);

  writestr (cur_slot, "Account created from: ");
  writestr (cur_slot, acct->created_from);
  writestr (cur_slot, "\nMessage format: ");
  writestr (cur_slot, acct->msgfmt);
  writestr (cur_slot, "\nActive format: ");
  writestr (cur_slot, acct->activefmt);
  writestr (cur_slot, "\nNewline character: ");
  writech (cur_slot, acct->nlchar);
  writestr (cur_slot, "\nNewlines ");
  writestr (cur_slot, acct->newlines ? "on" : "off");
  writestr (cur_slot, "\nBeeping ");
  writestr (cur_slot, acct->beeping ? "on" : "off");
#ifndef NOBROADCAST
  writestr (cur_slot, "\nBroadcast messages ");
  writestr (cur_slot, acct->nostat ? "off" : "on");
#endif
  writestr (cur_slot, "\nPA messages ");
  writestr (cur_slot, acct->pa_notify ? "off" : "on");
  writestr (cur_slot, "\nANSI colors ");
  writestr (cur_slot, acct->usecolor ? "on" : "off");
  writestr (cur_slot, "\nEmail status: ");
  writestr (cur_slot, acct->email_verified ? "verified" : "new");

  writestr (cur_slot, "\nUser has squelched:");
  for (i = 1; i <= slots_used; i++) {
    sq = slotbase[i];
    if (sq && squelched (sp, sq)) {
      writestr (cur_slot, " ");
      writestr (cur_slot, sq->acct.id);
    }
  }
  writestr (cur_slot, "\nUser has reversed:");
  for (i = 1; i <= slots_used; i++) {
    sq = slotbase[i];
    if (sq && reversed (sp, sq)) {
      writestr (cur_slot, " ");
      writestr (cur_slot, sq->acct.id);
    }
  }
  writestr (cur_slot, "\nUser is being squelched by:");
  for (i = 1; i <= slots_used; i++) {
    sq = slotbase[i];
    if (sq && squelched (sq, sp)) {
      writestr (cur_slot, " ");
      writestr (cur_slot, sq->acct.id);
    }
  }
  writestr (cur_slot, "\nUser is being reversed by:");
  for (i = 1; i <= slots_used; i++) {
    sq = slotbase[i];
    if (sq && reversed (sq, sp)) {
      writestr (cur_slot, " ");
      writestr (cur_slot, sq->acct.id);
    }
  }
  writech (cur_slot, '\n');
}

