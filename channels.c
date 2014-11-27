/*****************************************************************************
 *
 * NAME
 *     channels.c
 *
 * CONTENTS
 *   This file contains the code for handling channels.
 *
 *    Copyright 1996 Hyperreal.  All rights reserved.  For more information
 *    consult the file LICENSE.
 *
 *****************************************************************************/

#include "telechat.h"

static void set_channel (char *text);
static void set_ch_name (char *name);
static void disp_channel (char *text);

/* channels array contains elements for each user channel + one extra
	to hold the linked list of currently logged-in users */
CHANNEL channels[NUMCHANNELS + 1];

/****************************************************************************/

void get_channel (void)
{
  writestr (cur_slot, "Channel: ");
  setproc (set_channel, 3, COMPL_NONE);
}

/****************************************************************************/

static void set_channel (char *text)
{
  int chan;

  if (text[0]) {
    chan = atoi (text);

    if (chan >= 1 && chan <= 99) {
      /* security measure -- unverified users can only use
         certain channels  */
      if (chan >= FIRST_FREE_CHAN && !cur_slot->acct.email_verified) {
        writestr (cur_slot, "Your email address has not been verified.\n"
                            "As a result, you must stay on channel 1.\n");
        setproc (sendpub, MAXMSG, 0);
        return;
      }

      if (chan >= 91 && chan <= 99 && channels[chan].count > chan - 91) {
        writestr (cur_slot, "Channel busy.\n");
        setproc (sendpub, MAXMSG, 0);
        return;
      }
        
      if (slots_f) {
        sprintf (msg_buf, "-- #%d Left channel %02d: %s/%s",
                 cur_slot->nslot, cur_slot->acct.chan,
                 cur_slot->acct.id, cur_slot->acct.handle);
      } else {
        sprintf (msg_buf, "-- Left channel %02d: %s/%s",
                 cur_slot->acct.chan, cur_slot->acct.id,
                 cur_slot->acct.handle);
      }
      remove_from_channel (cur_slot, cur_slot->acct.chan);
      write_channel (msg_buf, cur_slot->acct.chan, PA_MSG);

      if (cur_slot->acct.chan == 1)
        write_log_paabout (cur_slot, "Left");

      if (slots_f) {
        sprintf (msg_buf, "-- #%d Joined channel %02d: %s/%s",
                 cur_slot->nslot, chan, cur_slot->acct.id,
                 cur_slot->acct.handle);
      } else {
        sprintf (msg_buf, "-- Joined channel %02d: %s/%s",
                 chan, cur_slot->acct.id, cur_slot->acct.handle);
      }
      write_channel (msg_buf, chan, PA_MSG);
      add_to_channel (cur_slot, chan);

      if (chan >= FIRST_FREE_CHAN && channels[chan].name) 
        /* this is a named channel */
        cur_slot->flags.listed = 1;
      else
        /* moving to an unnamed channel, reset the flag */
        cur_slot->flags.listed = 0;

      cur_slot->acct.chan = chan;
      cur_slot->chan_time = time (NULL);

      if (chan == 1)
        write_log_paabout (cur_slot, "Joined");

      writestr (cur_slot, "Channel changed to ");
      writestr (cur_slot, text);
      if (channels[chan].name)
        /* this is a named channel so state name */ {
        writestr (cur_slot, ": ");
        writestr (cur_slot, channels[chan].name);
      }
      writestr (cur_slot, ".\n");
    }
    else
      writestr (cur_slot, "Channel must be a number from 1 to 99.\n");
  }
  setproc (sendpub, MAXMSG, 0);
}

/****************************************************************************/

void name_channel (void) /* not ok */
{
  time_t now;

  now = time (NULL);

  if (cur_slot->acct.chan < FIRST_FREE_CHAN)
    writestr (cur_slot, "Sorry, you cannot rename this channel.\n");
  else if (cur_slot->acct.level != TOPLEVEL) {
    if ((now - channels[cur_slot->acct.chan].last_named) < 180)  {
      writestr (cur_slot, "Sorry, you must wait a few minutes"
                          " before you can rename this channel.\n");
      return;
    }
    else if (channels[cur_slot->acct.chan].last_named &&
             now - cur_slot->chan_time < 180) {
      writestr (cur_slot, "Sorry, you must wait a few minutes"
                          " before you can rename this channel.\n");
      return;
    }
    writestr (cur_slot, "New name: ");
    setproc (set_ch_name, MAXWIDTH, COMPL_NONE);
  }
  else if (cur_slot->acct.level == TOPLEVEL || 
           now - channels[cur_slot->acct.chan].last_named > 180) {
    writestr (cur_slot, "New name: ");
    setproc (set_ch_name, MAXWIDTH, COMPL_NONE);
  }
}

/****************************************************************************/

static void set_ch_name (char *name) /* ok... */
{
  time_t now;

  if (name[0]) {
    if (channels[cur_slot->acct.chan].name) {
      sprintf (msg_buf, " * Channel %02d \"%s\" is now \"%s\" (%s)",
               cur_slot->acct.chan, channels[cur_slot->acct.chan].name,
               name, cur_slot->acct.id);
      free (channels[cur_slot->acct.chan].name);
    } else {
      sprintf (msg_buf, " * Channel %02d is now \"%s\" (%s)",
               cur_slot->acct.chan, name, cur_slot->acct.id);
    }
    channels[cur_slot->acct.chan].name = (char *) malloc (strlen(name) + 1);
    if (channels[cur_slot->acct.chan].name == NULL) {
      writestr (cur_slot, "Could not rename channel due to"
                          " memory allocation problem.\n");
      setproc (sendpub, MAXMSG, 0);
      return;
    }
    now = time (NULL);
    channels[cur_slot->acct.chan].last_named = now;
    strcpy (channels[cur_slot->acct.chan].name, name);
    write_channel (msg_buf, cur_slot->acct.chan, CHAN_NAME);
  }
  setproc (sendpub, MAXMSG, 0);
}

/****************************************************************************/

void unname_ch (void)
{
  time_t now;

  now = time (NULL);

  if (cur_slot->acct.chan < FIRST_FREE_CHAN) {
    writestr (cur_slot, "Sorry, you cannot unname channel this channel.\n");
    return;
  }
  else if (now - cur_slot->chan_time < 180) {
    writestr (cur_slot, "Sorry, you must wait a few minutes"
                        " before you can unname this channel.\n");
    return;
  }

  sprintf (msg_buf, " * Channel %02d is now unnamed (%s)",
           cur_slot->acct.chan, cur_slot->acct.id);	

  write_channel (msg_buf, cur_slot->acct.chan, CHAN_UNNAME);
  free (channels[cur_slot->acct.chan].name);
  channels[cur_slot->acct.chan].name = NULL;
  channels[cur_slot->acct.chan].last_named = now;
}

/****************************************************************************/

void add_to_channel (SLOT *sl, int chan)
{
  SLOT_LIST *member;

  member = (SLOT_LIST *) malloc (sizeof(SLOT_LIST));
  if (member == NULL) {
    writestr (cur_slot, "Memory allocation failure in add_to_channel.\n");
    transmit (cur_slot);
    inactivate_slot (cur_slot, 0);
    return;
  }

  member->slot = sl;
  member->next = NULL;

  if (channels[chan].members == NULL)
    /* list is empty */
    channels[chan].members = member;
  else
    /* add to the end of the list */
    channels[chan].last_mem->next = member;

  channels[chan].last_mem = member;
  channels[chan].count++;
}

/****************************************************************************/

void remove_from_channel (SLOT *sl, int chan)
{
  SLOT_LIST *which = channels[chan].members;
  SLOT_LIST *prev = NULL;

  while (which) {
    if (which->slot == sl)
      break;
    prev = which;
    which = which->next;
  }

  if (!which)  /* got here while removing a corrupted connection */
    return;

  if (!prev) {
    /* which was first on the list */
    if (!which->next) {
      /* which was the only one on the list */
      free (channels[chan].members);
      channels[chan].members = NULL;
      channels[chan].last_mem = NULL;
    } else {
      channels[chan].members = channels[chan].members->next;
      free (which);
    }
  } else if (which->next == NULL) {
    /* which was at the end of the list */
    free (channels[chan].last_mem);
    channels[chan].last_mem = prev;
    channels[chan].last_mem->next = NULL;
  } else {
    /* which was in the middle of the list */
    prev->next = which->next;
    free (which);
  }

  channels[chan].count--;
  if (chan >= FIRST_FREE_CHAN && !channels[chan].count) {
    free (channels[chan].name);
    channels[chan].name = NULL;
    channels[chan].last_named = 0;
  }
}

/****************************************************************************/

void write_channel (char *msg, int chan, int msg_type)
{
  SLOT *sp;
  SLOT_LIST *mem = channels[chan].members;

  if (chan >= NUMCHANNELS)
    return;

  while (mem) {
    sp = mem->slot;
    if (!sp) {
      mem = mem->next;
      continue;
    }

    if (msg_type == CHAN_NAME) {
      sp->flags.listed = 1;
    } else if (msg_type == CHAN_UNNAME) {
      sp->flags.listed = 0;
    } else if (squelched (sp, cur_slot) || reversed (cur_slot, sp)) {
      mem = mem->next;
      continue;
    }

    if (msg_type == USR_MSG)
      writemsg (sp, msg, 0);
    else {
      select_stop (sp);
      select_wrap (sp);

      begin_color (sp, cur_slot->acct.msgcolor);
      writestr (sp, msg);
      reset_attr (sp);
      writech (sp, '\n');

      clear_stop (sp);
      clear_wrap (sp);
    }

    mem = mem->next;
  }
}

/****************************************************************************/

void list_channel (void)
{
  writestr (cur_slot, "Channel: ");
  if (cur_slot->acct.level == TOPLEVEL)
    setproc (disp_channel, 4, COMPL_NONE);
  else
    setproc (disp_channel, 3, COMPL_NONE);
}

/****************************************************************************/

static void disp_channel (char *text) /* see chan_typ() */
{
  int chan;

  if (text[0])
    chan = atoi (text);
  else
    chan = cur_slot->acct.chan;

  if (cur_slot->acct.level != TOPLEVEL) {
    if (chan < 1 || chan > NUMCHANNELS){
      setproc (sendpub, MAXMSG, 0);
      return;
    }
  }
  else {
    if (chan < 0 || chan >= NUMCHANNELS){
      setproc (sendpub, MAXMSG, 0);
      return;
    }
  }

  if (cur_slot->acct.chan == chan ||
      channels[chan].name ||
      cur_slot->acct.level == TOPLEVEL) {
    if (channels[chan].name) {
      sprintf (msg_buf, "Channel %02d -- %s\n", chan, channels[chan].name);
      writestr (cur_slot, msg_buf);
    }
    display_users (chan);
  }
  else {
    sprintf (msg_buf, "Channel %02d is unnamed or unused\n", chan);
    writestr (cur_slot, msg_buf);
  }

  setproc (sendpub, MAXMSG, 0);
}
