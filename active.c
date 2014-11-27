/*****************************************************************************
 *
 * NAME
 *     active.c
 *
 * CONTENTS
 *   This file contains the code for displaying active formats (/a).
 *
 *    Copyright 1996 Hyperreal.  All rights reserved.  For more information
 *    consult the file LICENSE.
 *
 *****************************************************************************/

#include "telechat.h"

static void help_sharedfmt (void);
static void help_msgfmt (void);
static void help_activefmt (void);
static void change_fmt2 (char *maq);
static void change_fmt3 (char *fmt);
static void change_fmt4 (char *fmt);

/*****************************************************************************
 *
 * NAME
 *    active()
 *
 * OVERVIEW
 *    This procedure lists all the users currently on the system.
 *
 *****************************************************************************/

void active (void)
{
  int chan;
  int anyone = 0;

  for (chan = 1; chan < FIRST_FREE_CHAN; chan++) {
    writestr (cur_slot, "Channel ");
    writetwodig (cur_slot, chan);
    writestr (cur_slot, " -- ");
    writestr (cur_slot, channels[chan].name);
    writech (cur_slot, '\n');
    if (channels[chan].count) {
      display_users (chan);
      anyone = 1;
      if (chan != FIRST_FREE_CHAN - 1)
        writech (cur_slot, '\n');
    }
  }
  for (chan = FIRST_FREE_CHAN; chan < NUMCHANNELS; chan++) {
    if (channels[chan].name) {
      /* channel is in use and is public */
      writestr (cur_slot, "\nChannel ");
      writetwodig (cur_slot, chan);
      writestr (cur_slot, " -- ");
      writestr (cur_slot, channels[chan].name);
      writech (cur_slot, '\n');
      display_users (chan);
      anyone = 1;
    }
  }

  if (channels[MAINLIST].members)
    writestr (cur_slot, "\nAll other channels are unnamed or unused\n");

  for (chan = FIRST_FREE_CHAN; chan < NUMCHANNELS; chan++) {
    if (channels[chan].count && !channels[chan].name) {
      /* channel is in use but is private */
      if (cur_slot->acct.level == TOPLEVEL) {
        writestr (cur_slot, "Channel ");
        writetwodig (cur_slot, chan);
        writestr (cur_slot, "\n");
      }
      display_users (chan);
      anyone = 1;
    }
  }

  if (!anyone)
    writestr (cur_slot, "\nNobody currently online.\n\n");
}

/****************************************************************************
 *
 * NAME
 *    display_users
 *
 * OVERVIEW
 *
 ****************************************************************************/

void display_users (int ch)
{
  char *fmt;
  SLOT_LIST *members;
  SLOT *sp_about;
  long hours, mins, idle;

  members = channels[ch].members;
  while (members) {
    fmt = cur_slot->acct.activefmt;
    sp_about = members->slot;
    while (*fmt) {
      if (*fmt == '%') {
        switch (*++fmt) {
        case 't': 
          /* Possible formats of %t substitution:
             "",                 " (typing)",
             " (idle: 41m)",     " (typing, idle: 41m)",
             " (idle: 2h 05m)",  " (typing, idle: 2h 05m)"
          */
          if (sp_about != cur_slot) {
            idle = (time(NULL) - sp_about->acct.last_typed) / 60;
            if (idle != 0 || sp_about->flags.stopped) {
              writestr (cur_slot, " (");
              if (sp_about && sp_about->flags.stopped) {
                writestr (cur_slot, "typing");
                if (idle != 0)
                  writestr (cur_slot, ", ");
              }
              if (idle != 0) {
                hours = idle / 60;
                mins = (idle - (hours * 60));
                writestr (cur_slot, "idle: ");
                if (hours != 0) {
                  writeint (cur_slot, hours);
                  writestr (cur_slot, "h ");
                }
                writetwodig (cur_slot, mins);
                writech (cur_slot, 'm');
              }
              writech (cur_slot, ')');
            }
          }
          break;

        default: 
          if (shared_spec (cur_slot, sp_about, *fmt))
            break;

          writech (cur_slot, '%');
          if (!*fmt)
            fmt--;
          else
            writech (cur_slot, *fmt);
        }
      } else
        writech (cur_slot, *fmt);
      fmt++;
    }
    writech (cur_slot, '\n');
    members = members->next;
  }
}

/****************************************************************************
 *
 * NAME
 *    shared_spec
 *
 * OVERVIEW
 *
 ****************************************************************************/

int shared_spec (SLOT *sp_to, SLOT *sp_about, char ch)
{
  switch (ch) {
  case '%':
    writech (sp_to, '%');
    break;

  case 's':
    writetwodig (sp_to, sp_about->nslot);
    break;

  case 'S':
    writeint (sp_to, sp_about->nslot);
    if (sp_about->nslot < 10)
      writech (sp_to, ' ');
    break;

  case 'u':
    writestr (sp_to, sp_about->acct.id);
    break;

  case 'M':
    if (sp_to->acct.level == TOPLEVEL ||
        sp_about->acct.email_public)
      writestr (sp_to, sp_about->acct.email_address);
    else
      writestr (sp_to, "*PRIVATE*");
    break;

  case 'v':
    if (verify_f && sp_to->acct.level == TOPLEVEL) {
      if (sp_about->acct.email_verified)
        writestr (sp_to, "Ver");
      else
        writestr (sp_to, "New");
    }
    break;

  case 'h':
    writestr (sp_to, sp_about->acct.handle);
    break;

  case '@':
    if (!nohostname_f || sp_about->acct.email_public ||
        sp_to->acct.level >= MODERATOR) {
      writestr (sp_to, sp_about->hostname);
    }
    break;

  case 'c':
    if (sp_about->acct.chan == sp_to->acct.chan ||
        sp_about->flags.listed || sp_about->acct.chan == 1 ||
        sp_to->acct.level == TOPLEVEL)
      writetwodig (sp_to, sp_about->acct.chan);
    else
      writestr (sp_to, "**");
    break;

  case 'C':
    if (sp_about->acct.chan == sp_to->acct.chan ||
        sp_about->flags.listed || sp_about->acct.chan == 1 ||
        sp_to->acct.level == TOPLEVEL)
      writeint (sp_to, sp_about->acct.chan);
    else
      writestr (sp_to, "*");
    break;

  case '<':
  case '>':
  case '$':
  case 'N':
    break;
 
  default:
    return 0;
  }
  return 1;
}

/*****************************************************************************
 *
 * NAME
 *
 * OVERVIEW
 *
 ****************************************************************************/

static void help_sharedfmt (void)
{
  writestr (cur_slot, "The following macros can be used in the format:\n"
                      "%u - User's loginid       %h - User's handle\n"
                      "%s - Slot number          %S - Slot w/no leading 0\n"
                      "%@ - User's net location  %M - User's email address\n");
  if (cur_slot->acct.level > 1)
    writestr (cur_slot, "%v - Email status         ");
}

/*****************************************************************************
 *
 * NAME
 *
 * OVERVIEW
 *
 ****************************************************************************/

static void help_msgfmt (void)
{
  help_sharedfmt();
  writestr (cur_slot, "%m - Message text - MAKE SURE YOU HAVE A %m !\n"
                      "\n"
                      "Text enclosed in '\\' is only printed if the message\n"
                      "is a private message. Similarly, text enclosed in\n"
                      "'_' is only printed if the message is a broadcast\n"
                      "message.\n"
                      "\n"
                      "Default format:\n");
  writestr (cur_slot, DEF_MSG_FMT);
  writech (cur_slot, '\n');
}

/*****************************************************************************
 *
 * NAME
 *
 * OVERVIEW
 *
 ****************************************************************************/

static void help_activefmt (void)
{
  help_sharedfmt();
  writestr (cur_slot, "%t - (typing)\n"
                      "\n"
                      "Default format:\n");
  writestr (cur_slot, DEF_ACT_FMT);
  writech (cur_slot, '\n');
}

/*****************************************************************************
 *
 * NAME
 *
 * OVERVIEW
 *
 ****************************************************************************/

void change_fmt (void)
{
  writestr (cur_slot,
            "Do not use this command unless you WANT to change the way\n"
            "people's messages or the /a command is displayed to you.\n"
            "If you mess up and get garbage displayed on your screen after\n"
            "using this command, use <d>efault to set your account to the\n"
            "default formats.\n"
            "\n"
            "<m>essage, <a>ctive, <d>efault, <q>uit to do nothing: ");
  setproc (change_fmt2, 2, COMPL_NONE);
}

/*****************************************************************************
 *
 * NAME
 *
 * OVERVIEW
 *
 ****************************************************************************/

static void change_fmt2 (char *maq)
{
  switch (*maq) {
  case 'M': 
  case 'm': 
    sprintf (msg_buf, "Your message format is:\n"
                      "%s\n"
                      "\nEnter new format, <CR> to keep or quit"
                      " or \"?\" for help\n",
             cur_slot->acct.msgfmt);
    writestr (cur_slot, msg_buf);
    setproc (change_fmt3, MAXFMT, COMPL_NONE);
    break;

  case 'A': 
  case 'a': 
    sprintf (msg_buf, "Your active format is:\n"
                      "%s\n"
                      "\nEnter new format, <CR> to keep or quit"
                      " or \"?\" for help\n",
             cur_slot->acct.activefmt);
    writestr (cur_slot, msg_buf);
    setproc (change_fmt4, MAXFMT, COMPL_NONE);
    break;

  case 'D':
  case 'd':
    writestr (cur_slot, "Restoring default active and message formats:\n");
    strcpy (cur_slot->acct.msgfmt, DEF_MSG_FMT);
    writestr (cur_slot, "Message format changed.\n");
    strcpy (cur_slot->acct.activefmt, DEF_ACT_FMT);
    writestr (cur_slot, "Active format changed.\n");
    setproc (sendpub, MAXMSG, 0);
    break;

  case 'Q': 
  case 'q': 
    setproc (sendpub, MAXMSG, 0);
    break;
  }
}

/*****************************************************************************
 *
 * NAME
 *
 * OVERVIEW
 *
 ****************************************************************************/

static void change_fmt3 (char *fmt)
{
  if (!strcmp (fmt, "?")) {
    help_msgfmt();
  } else if (*fmt) {
    strcpy (cur_slot->acct.msgfmt, fmt);
    writestr (cur_slot, "Message format changed.\n");
  }
  setproc (sendpub, MAXMSG, 0);
}

/*****************************************************************************
 *
 * NAME
 *
 * OVERVIEW
 *
 ****************************************************************************/

static void change_fmt4 (char *fmt)
{
  if (!strcmp (fmt, "?")) {
    help_activefmt();
  } else if (*fmt) {
    strcpy (cur_slot->acct.activefmt, fmt);
    writestr (cur_slot, "Active format changed.\n");
  }
  setproc (sendpub, MAXMSG, 0);
}

/*****************************************************************************
 *
 * NAME
 *
 * OVERVIEW
 *
 ****************************************************************************/

void see (void)
{
  sprintf (msg_buf, "There %s currently %d %s visiting.\n",
           (cur_loggedin == 1) ? "is" : "are",
           cur_loggedin,
           (cur_loggedin == 1) ? "person" : "people");
  writestr (cur_slot, msg_buf);
  setproc (sendpub, MAXMSG, 0);
}
