/*****************************************************************************
 * 
 * NAME
 *    io.c
 *
 * CONTENTS
 *    This file mostly contains input/output stuff for the chat server.
 *
 *    Copyright 1996 Hyperreal.  All rights reserved.  For more information
 *    consult the file LICENSE.
 * 
 *****************************************************************************/

#include "telechat.h"
extern int h_errno;

static int insert (SLOT *sp, char ch, int asis);
static void wrap_writech (SLOT *sp, char ch);
static void insert_buf (SLOT *sp, char *buf, int n);
static void process_char (unsigned char c);

/*****************************************************************************
 * 
 * NAME
 *    writeaction
 *
 * OVERVIEW
 *    This procedure writes an action message to a slot
 *
 *****************************************************************************/

void writeaction (SLOT *sp, char *msg)
{
  select_stop (sp);
  select_wrap (sp);
  begin_attr (sp, 1);
  begin_color (sp, cur_slot->acct.msgcolor);
  writestr (sp, msg);
  reset_attr (sp);
  writech (sp, '\n');
  clear_stop (sp);
  clear_wrap (sp);
}

/*****************************************************************************
 * 
 * NAME
 *    writestr()
 *
 * OVERVIEW
 *    This procedure writes a string to slot sp
 *
 *****************************************************************************/

void writestr (SLOT *sp, char *s)
{
  while (s && *s)
    writech (sp, *s++);
}


/*****************************************************************************
 * 
 * NAME
 *    writeint
 *
 * OVERVIEW
 *    This procedure writes an integer value to slot sp
 *
 *****************************************************************************/

void writeint (SLOT *sp, int i)
{
  if (i >= 10)
    writeint (sp, i / 10);
  writech (sp, i % 10 + '0');
}

/*****************************************************************************
 * 
 * NAME
 *    writetwodig
 *
 * OVERVIEW
 *    This procedure writes a two-digit value to slot sp
 *
 *****************************************************************************/

void writetwodig (SLOT *sp, int i)
{
  if (i >= 100)
    writeint (sp, i);
  else {
    writech (sp, i / 10 + '0');
    writech (sp, i % 10 + '0');
  }
}

/*****************************************************************************
 * 
 * NAME
 *    writemsg
 *
 * OVERVIEW
 *    This procedure writes a message to a slot
 *
 *****************************************************************************/
/***                     Has to be rewritten                               ***/

void writemsg (SLOT *sp, char *msg, int typ)
{
  char *s;
  char *t;
  
  select_stop (sp);
  select_wrap (sp);
  begin_color (sp, cur_slot->acct.msgcolor);

  /* Emphasize personal messages */
  if (typ == 1)
    begin_attr (sp, 1);

  s = sp->acct.msgfmt;
  while (*s) {
    if (*s == '%') {
      switch (*++s) {
      case '\\': 
        writech (sp, '\\');
        break;

      case '_': 
        writech (sp, '_');
        break;

      case 'm': 
        while (*msg) {
          if (*msg == cur_slot->acct.nlchar && sp->acct.newlines)
            writestr (sp, "\n  ");
          else
            writech (sp, *msg);
          msg++;
        }
        break;
	    
      default: 
        if (shared_spec (sp, cur_slot, *s))
          break;
        writech (sp, '%');
        if (!*s)
          s--;
        else
          writech (sp, *s);
      }
    } else if (*s == '_') {
      t = (char *)index (++s, '_');
      if (typ == 2) {
        if (t != NULL)
          *t = '\0';
        writestr (sp, s);
        if (t != NULL)
          *t = '_';
      }
      if (t == NULL)
        break;
      else
        s = t;
    } else if (*s == '\\') {
      t = (char *)index (++s, '\\');
      if (typ == 1) {
        if (t != NULL)
          *t = '\0';
        writestr (sp, s);
        if (t != NULL)
          *t = '\\';
      }
      if (t == NULL)
        break;
      else
        s = t;
    } else
      writech (sp, *s);
    s++;
  }

  reset_attr (sp);
  writech (sp, '\n');
  clear_stop (sp);
  clear_wrap (sp);
}

void writemsg_raw (char *msg, int color, char nlchar)
{

  select_wrap (cur_slot);
  writestr (cur_slot, "     ");
  begin_color (cur_slot, color);

  while (*msg) {
    if (*msg == nlchar)
      writestr (cur_slot, "\n  ");
    else
      writech (cur_slot, *msg);
    msg++;
  }

  reset_attr (cur_slot);
  writech (cur_slot, '\n');
  clear_wrap (cur_slot);
}

/*****************************************************************************
 *
 * NAME
 *    writetime()
 *
 * OVERVIEW
 *    This procedure writes the time of day and the amount of ontime to a slot
 *
 ****************************************************************************/

void writetime (SLOT * sp, long t)
{
  writetwodig (sp, (int) (t / 3600));
  writech (sp, ':');
  writetwodig (sp, (int) (t / 60 % 60));
  writech (sp, ':');
  writetwodig (sp, (int) (t % 60));
}

/*****************************************************************************
 *
 * NAME
 *
 * OVERVIEW
 *
 ****************************************************************************/
/***                     Has to be rewritten                               ***/

void operact (SLOT *from, SLOT *to, char * msg)
{
  SLOT *sp;
  SLOT_LIST *sl;
  
  sl = channels[MAINLIST].members;
  while(sl) {
    sp = sl->slot;
    if (sp && sp != from && sp->acct.level > 3) {
      select_stop (sp);
      select_wrap (sp);
      if (from)
        begin_color (sp, from->acct.msgcolor);
      writestr (sp, "** ");
      if (from) {
        writestr (sp, from->acct.id);
        writech (sp, '/');
        writestr (sp, from->acct.handle);
        writech (sp, ' ');
      }
      writestr (sp, msg);
      if (to != NULL) {
        writech (sp, ' ');
        writestr (sp, to->acct.id);
        writech (sp, '/');
        writestr (sp, to->acct.handle);
      }
      if (from)
        reset_attr (sp);
      writech (sp, '\n');
      clear_stop (sp);
      clear_wrap (sp);
    }
    sl = sl->next;
  }
}

/*****************************************************************************
 *
 * NAME
 *
 * OVERVIEW
 *
 ****************************************************************************/
/***                     Has to be rewritten                               ***/

void paabout (SLOT *sl, char *msg)
{
  SLOT *sp;
  SLOT_LIST *slist;

  write_log_paabout (sl, msg);

  slist = channels[MAINLIST].members;
  while (slist) {
    sp = slist->slot;

    if (sp && (sp->acct.pa_notify || 
               (sl->acct.chan == sp->acct.chan))) {
      select_stop (sp);
      select_wrap (sp);
      begin_color (sp, sl->acct.msgcolor);
      if (slots_f) {
        writestr (sp, "-- #");
        writeint (sp, sl->nslot);
      } else {
        writestr (sp, "--");
      }
      writech (sp, ' ');
      writestr (sp, msg);
      writestr (sp, " channel ");
      if (sl->acct.chan == sp->acct.chan || 
          sl->flags.listed || sl->acct.chan == 1 ||
          sp->acct.level > 2)
        writetwodig (sp, sl->acct.chan);
      else
        writestr (sp, "**");
      writestr (sp, ": ");
      writestr (sp, sl->acct.id);
      writech (sp, '/');
      writestr (sp, sl->acct.handle);
      reset_attr (sp);
      writech (sp, '\n');
      clear_stop (sp);
      clear_wrap (sp);
    }
    slist = slist->next;
  }
}

/*******************************************************************
 *
 *  Set the function which should process next string entered by the
 *  user on current slot (that function will receive a pointer to the
 *  entered string as its first argument).
 *    func: pointer to the function;
 *    max_input: maximum number of bytes in the input allowed
 *      (including trailing '\0');
 *    flags: options to set special input modes.
 *
 *  Use one of the following constants for flags (or 0 for default):
 *    READ_NOECHO:    disable echoing of typed characters;
 *    COMPL_DEFAULT:  default username completion behaviour (default);
 *    COMPL_STRICT:   strict username completion for entering single
 *                    usernames;
 *    COMPL_NONE:     disable username completion completely.
 *
 *  Notes:
 *    Tab completion won't work if echo is off. And it works only for
 *    users logged in (i.e. not in login prompt etc.).
 *
 *******************************************************************/

void setproc (void (*func)(char *text), int max_input, int flags)
{
  cur_slot->readfunc = func;
  cur_slot->inmax = max_input;

  cur_slot->flags.echo = (flags & READ_NOECHO) ? 0 : 1;
  cur_slot->flags.compl = flags & COMPL_MASK;
}

/*** Make select_* and clear_* static? ***/

void select_stop (SLOT *sp)
{
  if (sp->flags.stopped)
    sp->flags.wstop = 1;
}

void clear_stop (SLOT *sp)
{
  sp->flags.wstop = 0;
}

void select_wrap (SLOT *sp)
{
  sp->flags.wrap = 1;
  sp->last_roll = sp->wrap_base;
}

void clear_wrap (SLOT *sp)
{
  sp->flags.wrap = 0;
}

/*****************************************************************************
 *
 * NAME
 *    insert()
 *
 * OVERVIEW
 *    This function inserts a character into sp's output queue if output is  
 *    not stopped, or the stop queue if sp's output is stopped. 
 *    asis != 0 forces sending symbol as is, without doubling IAC chars
 *    and preceding '\n' with '\r'.
 *
 ****************************************************************************/

static int insert (SLOT *sp, char ch, int asis)
{
  if (sp->flags.die)
    return 0;

  if (ch == '\n' && !asis) {
    if (!insert (sp, '\r', 1))
      return 0;
  }
  if ((unsigned char)ch == IAC && !asis) {
    if (!insert (sp, IAC, 1))
      return 0;
  }

  qinsert (sp->flags.wstop ? &sp->stopq : &sp->outq, ch);

/*
 * In the following code fragment transmit() could block and hang
 * the server for some time. Ideally, transmit() should be called
 * only after successful select() call.
 */

/*
  if (qlength (&sp->outq) > sp->outq.qsize * 3 / 4) {
    if (writequeue (sp, &sp->outq) < 0) {
      if (sp->acct.chan < NUMCHANNELS) {
        remove_from_channel (sp, sp->acct.chan);
        remove_from_channel (sp, MAINLIST);
      }
      if (sp->acct.chan && sp->flags.on) {
        paabout (sp, DISCONNECTED);
        sp->acct.last_logout = time (NULL);
      }
      inactivate_slot (sp, 0);
      return 0;
    }
  }
*/

  return 1;
}

/*****************************************************************************
 * 
 * NAME
 *    wrap_writech()
 *
 * OVERVIEW
 *    This procedure is similar to writech but supports word wrap 
 *
 *****************************************************************************/

static void wrap_writech (SLOT *sp, char ch)
{
  char   *space,
  buf[MAXWIDTH + 1];
  
  if (ch == '\n') 
    {
      insert_buf (sp, sp->wrap_base, sp->wrap_ptr - sp->wrap_base);
      insert (sp, '\n', 0);
      sp->wrap_ptr = sp->wrap_base;
    }
  else {
    *sp->wrap_ptr++ = ch;
    if (sp->wrap_ptr - sp->wrap_base == sp->acct.width - 3) 
      {
	for (space = sp->wrap_ptr; space != sp->last_roll; space--)
	  if (isspace (*space)) 
	    {
	      insert_buf (sp, sp->wrap_base,
			  space - sp->wrap_base);
	      insert (sp, '\n', 0);
	      *sp->wrap_ptr = '\0';
	      strcpy (buf, space + 1);
	      strcpy (sp->wrap_base, "     ");
	      strcat (sp->wrap_base, buf);
	      sp->last_roll = sp->wrap_ptr = sp->wrap_base +
		strlen (sp->wrap_base);
	      return;
	    }
	insert_buf (sp, sp->wrap_base,
		    sp->wrap_ptr - sp->wrap_base);
	insert (sp, '\n', 0);
	strcpy (sp->wrap_base, "     ");
	sp->last_roll = sp->wrap_ptr = sp->wrap_base+
	  strlen(sp->wrap_base);
      }
  }
}

/*****************************************************************************
 * 
 * NAME
 *    writech
 *
 * OVERVIEW
 *    This procedure writes a character to a slot
 *
 *****************************************************************************/

void writech (SLOT *sp, char ch)
{
  if (sp->flags.wrap)
    wrap_writech (sp, ch);
  else
    insert (sp, ch, 0);
}

/*****************************************************************************
 * 
 * NAME
 *    insert_buf
 *  
 * OVERVIEW
 *    This procedure is used by wrap_writech to write chunks 
 *    into the output queue 
 *
 *****************************************************************************/

static void insert_buf (SLOT *sp, char *buf, int n)
{
  while (--n >= 0)
    if (!insert (sp, *buf++, 0))
      break;
}

/****************************************************************************
 * 
 * NAME
 *    sendpub
 *
 * OVERVIEW
 *    This procedure sends an ordinary public message to all interested
 *    parties.
 *
 ****************************************************************************/

void sendpub (char *text)
{
  write_log (text);
  write_channel (text, cur_slot->acct.chan, USR_MSG);
}

/*****************************************************************************
 *
 * NAME
 *
 * OVERVIEW
 *   send_ts sends a TELNET control sequence to the client.
 *
 ****************************************************************************/

void send_ts (char c, char d)
{
  insert (cur_slot, IAC, 1);
  insert (cur_slot, c, 1);
  insert (cur_slot, d, 1);
  insert (cur_slot, '\0', 1);
}

/*****************************************************************************
 *
 * NAME
 *
 * OVERVIEW
 *    process_char processes a single character of input.
 *
 ****************************************************************************/
static void process_char (unsigned char c)
{
  switch (cur_slot->tsmode)
    {
    case TS_NONE: 
      ts_none (c);
      break;

    case TS_IAC: 
      switch (c)
	{
	case IAC:
	  ts_none (c);
	  break;
	case WILL: 
	  cur_slot->tsmode = TS_WILL;
	  break;
	case WONT: 
	  cur_slot->tsmode = TS_WONT;
	  break;
	case DO: 
	  cur_slot->tsmode = TS_DO;
	  break;
	case DONT: 
	  cur_slot->tsmode = TS_DONT;
	  break;
	case EC: 
	  ts_none ('\b');
	  cur_slot->tsmode = TS_NONE;
	  break;
	case EL: 
	  ts_none (24);
	  cur_slot->tsmode = TS_NONE;
	  break;
	case AO: 
	  qflush (&cur_slot->outq);
	  qflush (&cur_slot->stopq);
	  break;
	case AYT: 
	  writestr (cur_slot, "\n[Yes]\n");
	default: 
	  cur_slot->tsmode = TS_NONE;
	  break;
	}
      break;

    case TS_WILL: 
      send_ts (DONT, (char)c);
      cur_slot->tsmode = TS_NONE;
      break;

    case TS_WONT: 
      cur_slot->tsmode = TS_NONE;
      break;

    case TS_DO: 
      if (c != TELOPT_SGA && c != TELOPT_ECHO)
	send_ts (WONT, (char)c);
      cur_slot->tsmode = TS_NONE;
      break;

    case TS_DONT: 
      cur_slot->tsmode = TS_NONE;
      break;
    }
}

/*****************************************************************************
 *
 * NAME
 *
 * OVERVIEW
 *      process_input reads incoming data from connection fd and sends
 *      it to process_char for handling.
 *
 ****************************************************************************/
/***                     Has to be rewritten                               ***/

void process_input (SLOT *sp)
{
  unsigned char buf[MAXBUF];
  int i, len;

  if (sp->flags.die)
    return;

  cur_slot = sp;
  len = read (cur_slot->fd, (char *)buf, MAXBUF);
  if (len > 0) {
    for (i = 0; i < len; i++)
      process_char (buf[i]);
  } else {
    transmit (cur_slot);
    if (!sp->flags.die) {
      if (cur_slot->acct.chan < NUMCHANNELS) {
        /* protect against people whose logins time our or
           something before they're fully "on" the system */
        remove_from_channel (cur_slot, cur_slot->acct.chan);
        remove_from_channel (cur_slot, MAINLIST);
      }
      if (cur_slot->acct.chan && cur_slot->flags.on) {
        paabout (cur_slot, DISCONNECTED);
        cur_slot->acct.last_logout = time(NULL);
      }
      inactivate_slot (cur_slot, 0);
    }
  }
}


/****************************************************************************
 *
 * NAME
 *    transmit()
 *
 * OVERVIEW
 *
 ****************************************************************************/

void transmit (SLOT *sp)
{
  if (sp->flags.die)
    return;

  if (writequeue (sp, &sp->outq) < 0) {
    if (sp->acct.chan < NUMCHANNELS) {
      remove_from_channel (sp, sp->acct.chan); 
      remove_from_channel (sp, MAINLIST); 
    }
    if (sp->acct.chan && sp->flags.on) {
      paabout (sp, DISCONNECTED);
      sp->acct.last_logout = time (NULL);
    }
    inactivate_slot (sp, 0);
    return;
  }

  if (!qempty (&sp->stopq) && !sp->flags.stopped &&
      writequeue (sp, &sp->stopq) < 0) {
    if (sp->acct.chan < NUMCHANNELS) {
      remove_from_channel (sp, sp->acct.chan); 
      remove_from_channel (sp, MAINLIST); 
    }
    if (sp->acct.chan && sp->flags.on) {
      paabout (sp, DISCONNECTED);
      sp->acct.last_logout = time (NULL);
    }
    inactivate_slot (sp, 0);
  }
}

