/*******************************************************************
 *
 * $Id: main.c,v 1.60 2000/06/11 09:20:40 const Exp $
 * Main module of telechat-ng.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 * Copyright 1996 Hyperreal.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/

#include "telechat.h"

/* The socket listening for incoming connections */
int listen_sock;

/* The buffer to use for some formatted output */
char msg_buf[MAXMSG];

/* The array of pointers to SLOT structures associated with slots,
   max_slots+1 is a number of elements in this array, slot==0 and
   slotbase[0] are not used */
SLOT **slotbase;

/* Currently executing slot */
SLOT *cur_slot = NULL;

/* The index of last used slot */
int slots_used = 0;

/* The highest file descriptor used */
int maxfd_used = 0;

/* The list of slots to close */
SLOT **closeslots;

/* The number of slots to close */
int closeslots_num = 0;

/* The system time at program startup */
time_t startup_time;

/* The number of users that have logged in since startup */
int login_count = 0;

/* The number of users that are currently logged in */
int cur_loggedin = 0;

/* The list of file descriptors used */
fd_set used;

/* If set, a signal to logout idlers has arrived */
int logout_idle_flag = 0;

/* The IP port the program is running on */
int inet_port = DEF_PORT;

/*******************************************************************/

static int noforkFlag = 0;

static void logout_idle (void);
static void complete_id (void);
static void complete_id_incr (void);
static void get_completions (void);
static void reset_completions (void);
static void undo_completions (void);
static void purge_slots (void);
static void mainloop (void);
static void accept_connection (void);
static void get_write (fd_set *w);
static void Usage (char *name);

/*******************************************************************
 *
 *  Find the slot corresponding to the given login name of some user
 *  beeng online.
 *    id: pointer to the login id.
 *
 *  Return values:
 *    NULL: there is no such user online;
 *    Valid (SLOT *) pointer: that slot found.
 *
 *******************************************************************/

SLOT *slotbyname (char *id)
{
  SLOT *sp;
  int i;
  
  if (login_is_valid(id) == VAL_OK) {
    for (i = 1; i <= slots_used; i++) {
      sp = slotbase[i];
      if (sp && sp->flags.on && !strcmp (sp->acct.id, id))
        return sp;
    }
  }
  return NULL;
}

/*******************************************************************
 *
 *  Find the slot corresponding to the given slot number of some user
 *  beeng online.
 *    id: pointer to the login id.
 *
 *  Return values:
 *    NULL: there is no such user online;
 *    Valid (SLOT *) pointer: that slot found.
 *
 *******************************************************************/

SLOT *slotbynumber (char *text)
{
  SLOT *sp;
  int i;
  
  /* Remove xatoi() call */
  if (isdigit(text[0]) && (i = xatoi(text)) != -1) {
    if (i > 0 && i <= slots_used) {
      sp = slotbase[i];
      if (sp && sp->flags.on)
        return sp;
    }
  }
  return NULL;
}

/*******************************************************************
 *
 *  Allocate temporary memory space for a given slot.
 *    sp: slot;
 *    size: amount of memory.
 *
 *  Return values:
 *    0: failure;
 *    1: success.
 *
 *  Notes:
 *    Prints error message to the current slot on failure.
 *
 *******************************************************************/

int alloctemp (SLOT *sp, size_t size)
{
  if (sp) {
    sp->temp = (char *) malloc (size);
    if (sp->temp)
      return 1;
    writestr (cur_slot, "Error allocating memory.\n");
  }
  return 0;
}

/*******************************************************************
 *
 *  Free temporary memory space for a given slot.
 *    sp: slot.
 *
 *  Notes:
 *    It's ok to call this functions when memory was not allocated.
 *
 *******************************************************************/

void freetemp (SLOT *sp)
{
  if (sp && sp->temp) {
    free (sp->temp);
    sp->temp = NULL;
  }
}

/*******************************************************************
 *
 *  Mark the slot in order to be destroyed as soon as possible.
 *    sp: slot;
 *    nosave: do not save account record if 1.
 *
 *  Notes:
 *    Slots are physically destroyed in several certain places within
 *    main.c module. Other functions should use inactivate_slot() to
 *    mark slots for deletion.
 *
 *******************************************************************/

void inactivate_slot (SLOT *sp, int nosave)
{
  int i;

  if (!sp->flags.die) {
    sp->flags.die = 1;
    sp->flags.nosave = nosave;
    closeslots[closeslots_num++] = sp;
  }
}

/*******************************************************************
 *
 *  Destroy all slots marked to be destroyed.
 *
 *******************************************************************/

static void purge_slots (void)
{
  while (closeslots_num)
    destroy_slot (closeslots[--closeslots_num]);
}

/*******************************************************************
 *
 *  Description.
 *    <arg>: <description>.
 *
 *  Return values:
 *    <value>: <description>.
 *
 *  Notes:
 *    <notes>.
 *
 *******************************************************************/

void destroy_slot (SLOT *sp)
{
  SLOT *sq;
  int i, fd, maxfd;

  if (sp->flags.on) {
    add_recent (sp->acct.id, sp->login_time);
    cur_loggedin--;

    for (i = 1; i <= slots_used; i++) {
      sq = slotbase[i];
      if (sq && sq != sp) {
        FD_CLR (sp->nslot, &sq->squelch);
        FD_CLR (sp->nslot, &sq->reverse);
      }
    }

    if (sp->acct.chan >= 91 && sp->acct.chan <= 99)
      sp->acct.chan = 1;

    if (!sp->flags.nosave)
      write_user (&sp->acct);
  }

  freetemp (sp);
  if (sp->compls)
    free (sp->compls);
  
  qdispose (&sp->outq);
  qdispose (&sp->stopq);

  fd = sp->fd;
  FD_CLR (fd, &used);

  slotbase[sp->nslot] = NULL;

  /* Update slots_used and maxfd_used */
  if (sp->nslot == slots_used)
    while (slots_used && !slotbase[slots_used])
      slots_used--;
  if (fd == maxfd_used) {
    maxfd = listen_sock;
    for (i = slots_used; i > 0; i--)
      if (slotbase[i] && slotbase[i]->fd > maxfd)
        maxfd = slotbase[i]->fd;
    maxfd_used = maxfd;
  }

  free (sp);
  close (fd);
}

/*******************************************************************
 *
 *  Description.
 *    <arg>: <description>.
 *
 *  Return values:
 *    <value>: <description>.
 *
 *  Notes:
 *    <notes>.
 *
 *******************************************************************/

static void logout_idle (void)
{
  time_t now;
  int i;
  SLOT *sp;

  now = time (NULL);

  for (i = 1; i <= slots_used; i++) {
    sp = slotbase[i];
    if (sp && now - sp->acct.last_typed >= idle_timeout) {
      remove_from_channel (sp, sp->acct.chan);
      remove_from_channel (sp, MAINLIST);
      if (sp->flags.on) {
        sp->acct.last_logout = time (NULL);
        paabout (sp, DISCONNECTED);
      }
      destroy_slot (sp);
    }
  }
  logout_idle_flag = 0;
}

/*******************************************************************
 *
 *  Description.
 *    <arg>: <description>.
 *
 *  Return values:
 *    <value>: <description>.
 *
 *  Notes:
 *    <notes>.
 *
 *******************************************************************/

void ts_none (unsigned char c)
{
  SLOT *temp_sp;

  if (c == IAC) {
    if (cur_slot->tsmode != TS_IAC) {
      cur_slot->tsmode = TS_IAC;
      return;
    } else
      cur_slot->tsmode = TS_NONE;
  }

  cur_slot->acct.last_typed = time (NULL);

  /* check it out */
  if (cur_slot->readfunc == sendpub && cur_slot->inp == cur_slot->in)
    startqueue (cur_slot);

  if (c != '\t' && c != '\e')
    reset_completions();

  switch (c)
    {
    case '\r':
      cur_slot->flags.gotcr = 1;
      break;

    case NULL:
      if (!cur_slot->flags.gotcr)
	break;
      /* PASS THROUGH */

    case '\n':
      cur_slot->flags.gotcr = 0;
      if (cur_slot->readfunc == sendpub && cur_slot->inp == cur_slot->in)
	break;
      *cur_slot->inp = '\0';
      cur_slot->inp = cur_slot->in;

      /* Take care about expanding slot numbers to user names */
      if (slots_f && cur_slot->readfunc == proc_id &&
          isdigit(cur_slot->inp[0])) {
        temp_sp = slotbynumber (cur_slot->inp);
        if (temp_sp) {
          strcpy (cur_slot->inp, temp_sp->acct.id);
          writestr (cur_slot, " (");
          writestr (cur_slot, cur_slot->inp);
          writech (cur_slot, ')');
        }
      }
      writech (cur_slot, '\n');
      cur_slot->readfunc (cur_slot->in);
      break;

    case 0x17:                  /* Erase a word with Ctrl-W */
      if (cur_slot->inp != cur_slot->in && *cur_slot->inp == ' ') {
        if (cur_slot->flags.echo)
          writestr (cur_slot, "\b \b");
        cur_slot->inp--;
      }
      while (cur_slot->inp != cur_slot->in && *cur_slot->inp != ' ') {
        writestr (cur_slot, "\b \b");
        cur_slot->inp--;
      }
      break;
      
    case 0x15:                  /* Erase whole line with Ctrl-U */
      while (cur_slot->inp != cur_slot->in) {
        if (cur_slot->flags.echo)
          writestr (cur_slot, "\b \b");
        cur_slot->inp--;
      }
      break;
      
    case '\b':                  /* Backspace/Delete/Ctrl-X */ 
    case 0x7F:
    case 0x18:
      if (cur_slot->inp != cur_slot->in) {
        if (cur_slot->flags.echo)
          writestr (cur_slot, "\b \b");
        cur_slot->inp--;
      }
      break;
      
    case '\t':                  /* Username completion with Tab */
      if (cur_slot->readfunc == sendpub &&
          cur_slot->in[0] == '/'        &&
          cur_slot->inp == cur_slot->in + 1)
        break;
      if (cur_slot->flags.on && cur_slot->flags.echo) {
        if (icomplete_f)
          complete_id_incr();
        else
          complete_id();
      }
      break;

    case '\e':
      undo_completions();
      break;

    default: 
      if (isprint(c) && cur_slot->inp - cur_slot->in < cur_slot->inmax - 1) {
        /* put the char on the input queue */
        *cur_slot->inp++ = c;

        /* echo the char typed back to the user */
        if (cur_slot->flags.echo)
          writech (cur_slot, c);

        /* did we get a command? */
        if (cur_slot->readfunc == sendpub &&
            cur_slot->in[0] == '/' && cur_slot->inp == cur_slot->in + 2) {
          cur_slot->inp = cur_slot->in;
          exec_cmd (cur_slot->in[1]);
        }
      }
    }
  
  if (cur_slot->readfunc == sendpub &&
      cur_slot->inp == cur_slot->in && cur_slot->flags.stopped)
    clearqueue (cur_slot);
}

/*******************************************************************
 *
 *  Tab completion of usernames for ts_none() function.
 *
 *  Notes:
 *    This function operates on the current slot. It completes
 *    username if possible, otherwise unambigious part of username. If
 *    current mode is COMPL_DEFAULT and the input queue is empty
 *    (beginning of an ordinary message), it adds comma and space
 *    after complete name, otherwise a space only. Current user name
 *    is ignored in this case. If current processing function is
 *    sendpub, users in other rooms are also beeng ignored.
 *
 *******************************************************************/

static void complete_id (void)
{
  int i, j;
  int start_len, tail_max, found_len, found_part;
  char *start, *id, *found_id;
  SLOT *sp;

  if (cur_slot->flags.compl == COMPL_NONE)
    return;                     /* Completion is not allowed */

  /* Find first character of possible user name */
  start = cur_slot->inp;
  while (start != cur_slot->in) {
    if (!isalnum(*(start-1)))
      break;
    start--;
  }
  if (start != cur_slot->inp && isdigit(*start))
    return;                     /* Names cannot begin with digit */

  start_len = cur_slot->inp - start;
  tail_max = cur_slot->in + cur_slot->inmax - cur_slot->inp - 1;

  found_part = 0;
  found_len = -1;

  for (i = 1; i <= slots_used; i++) {
    sp = slotbase[i];
    if (sp && sp->flags.on) {
      if (cur_slot->flags.compl == COMPL_DEFAULT &&
          cur_slot->inp - cur_slot->in == start_len) {
        if (sp == cur_slot)
          continue;
        if (cur_slot->readfunc == sendpub &&
            sp->acct.chan != cur_slot->acct.chan)
          continue;
        if (squelched (cur_slot, sp) || reversed (cur_slot, sp))
          continue;
      }
      id = sp->acct.id;
      for (j = 0; j < start_len; j++) {
        if (id[j] == '\0' || id[j] != tolower(start[j]))
          break;
      }
      if (j != start_len)
        continue;

      if (found_len == -1) {    /* First match */
        found_id = id;
        found_len = strlen (&found_id[start_len]);
        if (found_len > tail_max) {
          found_len = tail_max;
          found_part = 1;
        }
      } else {                  /* More matches */
        for (; j < start_len + found_len; j++) {
          if (id[j] == '\0' || id[j] != found_id[j])
            break;
        }
        found_len = j - start_len;
        found_part = 1;
      }
    }
  }

  if (found_len != -1) {
    /* Assuming that echo is on and found_id[] is alphanumeric */
    for (j = start_len; j < start_len + found_len; j++) {
      *cur_slot->inp++ = found_id[j];
      writech (cur_slot, found_id[j]);
    }
    if (!found_part && cur_slot->flags.compl == COMPL_DEFAULT &&
        cur_slot->inp - cur_slot->in == start_len + found_len) {
      ts_none (',');
      ts_none (' ');
    }
  }
  /* Beep if completion is not found or is ambiguous */
  if (found_len == -1 || (found_part && found_len == 0))
    writech (cur_slot, '\a');
}

/*******************************************************************
 *
 *  Incremental Tab completion of usernames for ts_none() function.
 *
 *******************************************************************/

static void complete_id_incr (void)
{
  COMPLS *compls;
  int i;
  char c;

  if (cur_slot->flags.compl == COMPL_NONE)
    return;                     /* Completion is not allowed */

  if (!cur_slot->compls)
    get_completions();
  compls = cur_slot->compls;
  if (!compls) {
    writech (cur_slot, '\a');
    return;                     /* No completions available */
  }
  if (compls->current == compls->total) {
    compls->current = 0;
    compls->charidx = 0;
  } else {
    while (compls->names[compls->charidx++]) {
      if (cur_slot->inp != cur_slot->in) {
        if (cur_slot->flags.echo)
          writestr (cur_slot, "\b \b");
        cur_slot->inp--;
      }
    }
    compls->current++;
  }
  if (compls->current < compls->total) {
    for (i = 0; (c = compls->names[compls->charidx + i]) != '\0'; i++) {
      *cur_slot->inp++ = c;
      writech (cur_slot, c);
    }
  }
}

/*******************************************************************
 *
 *  Prepare a list of usernams for incremental completion. Allocates
 *  memory if necessary.
 *
 *******************************************************************/

static void get_completions (void)
{
  COMPLS *compls = NULL;
  SLOT *sp;
  char *start, *id;
  int i, j, start_len, tail_max;
  int address_mode = 0;

  /* Find first character of possible user name */
  start = cur_slot->inp;
  while (start != cur_slot->in) {
    if (!isalnum(*(start-1)))
      break;
    start--;
  }
  if (start != cur_slot->inp && isdigit(*start))
    return;                     /* Names cannot begin with digit */

  start_len = cur_slot->inp - start;
  tail_max = cur_slot->in + cur_slot->inmax - cur_slot->inp - 1;

  if (cur_slot->flags.compl == COMPL_DEFAULT &&
      cur_slot->inp - cur_slot->in == start_len)
    address_mode = 1;

  for (i = 1; i <= slots_used + 1; i++) {
    /* Process current slot after any others */
    if (i != slots_used + 1) {
      sp = slotbase[i];
      if (sp == cur_slot)
        continue;
    } else
      sp = cur_slot;

    if (sp && sp->flags.on) {
      /* Limiting range of completions in COMPL_DEFAULT mode */
      if (address_mode) {
        if (sp == cur_slot)
          continue;
        if (cur_slot->readfunc == sendpub &&
            sp->acct.chan != cur_slot->acct.chan)
          continue;
        if (squelched (cur_slot, sp) || reversed (cur_slot, sp))
          continue;
      }

      /* Check if this variant matches */
      id = sp->acct.id;
      for (j = 0; j < start_len; j++) {
        if (id[j] == '\0' || id[j] != tolower(start[j]))
          break;
      }
      if (j != start_len)
        continue;

      /* Allocate memory if that's a first completion */
      if (!compls) {
        cur_slot->compls = malloc (sizeof(COMPLS) + MAXID * slots_used);
        compls = cur_slot->compls;
        if (!compls)
          return;
        compls->current = compls->total = compls->charidx = 0;
      }

      /* Save matching username */
      for (; j < start_len + tail_max && id[j] != '\0'; j++)
        compls->names[compls->charidx++] = id[j];
      if (address_mode) {
        if (j++ < start_len + tail_max)
          compls->names[compls->charidx++] = ',';
        if (j < start_len + tail_max)
          compls->names[compls->charidx++] = ' ';
      }
      compls->names[compls->charidx++] = '\0';
      compls->current++;
      compls->total++;
    }
  }
}

/*******************************************************************
 *
 *  Free memory holding a list of completions available. The function
 *  should be caller when a user has pressed any key which is not a
 *  completion key (Tab or Esc).
 *
 *******************************************************************/

static void reset_completions (void)
{
  if (cur_slot->compls) {
    free (cur_slot->compls);
    cur_slot->compls = NULL;
  }
}

/*******************************************************************
 *
 *  Undo last completion attempts. The function is called when a user
 *  has pressed the Esc key. It wipes back current tail of completion.
 *
 *******************************************************************/

static void undo_completions (void)
{
  COMPLS *compls;

  compls = cur_slot->compls;
  if (compls) {
    if (compls->current != compls->total) {
      while (compls->names[compls->charidx++]) {
        if (cur_slot->inp != cur_slot->in) {
          if (cur_slot->flags.echo)
            writestr (cur_slot, "\b \b");
          cur_slot->inp--;
        }
      }
    }
    free (compls);
    cur_slot->compls = NULL;
  }
}

/*******************************************************************
 *
 *  Main loop. This function is waiting while next I/O operation is
 *  possible and calls appropriate functions to accept connections,
 *  read or write on the user's slots.
 *
 *******************************************************************/

static void mainloop (void)
{
  int fd, slot;
  SLOT *sp;
  fd_set r, w;
  struct timeval timeout;

  FD_ZERO (&used);
  FD_SET (listen_sock, &used);
  while (1) {
    memcpy ((char *) &r, (char *) &used, sizeof (fd_set));
    get_write (&w);
    timeout.tv_sec = 60;	/* set timeout for one minute */
    timeout.tv_usec = 0;
    if (select (maxfd_used + 1, &r, &w, (fd_set *)NULL, &timeout) > 0) {
      for (slot = 1; slot <= slots_used; slot++) {
        sp = slotbase[slot];
        if (sp) {
          fd = sp->fd;
          /* Process input */
          if (FD_ISSET (fd, &r)) {
            if (fd == listen_sock)
              accept_connection();
            else
              process_input (sp);
          }
          /* Process pending output */
          if (FD_ISSET (fd, &w))
            transmit (sp);
          /* Purge slots with broken connections */
          purge_slots();
        }
      }
      /* Process incoming connections */
      if (FD_ISSET (listen_sock, &r))
        accept_connection();
    } else {
      /* Do something in idle periods */
      reopen_logs();
    }
    if (logout_idle_flag)
      logout_idle();
  }
}

/*******************************************************************
 *
 *  Accept incoming connection.
 *
 *******************************************************************/

static void accept_connection (void)
{
  int len, fd, nslot;
  SLOT *sp;
  struct sockaddr_in sockaddr;
  struct hostent *hp;

  len = sizeof (struct sockaddr);
  fd = accept (listen_sock, (struct sockaddr *) &sockaddr, &len);
  if (fd >= 0) {
    /* Find first free slot */
    for (nslot = 1; nslot <= max_slots && slotbase[nslot]; nslot++);
    if (nslot > max_slots) {
      close (fd);
      return;                   /* Too many connections */
    }
    /* Allocate memory for SLOT structure, initialize with zeros */
    sp = (SLOT *) calloc (1, sizeof(SLOT));
    if (!sp) {
      close (fd);
      return;                   /* Memory allocation error */
    }
    slotbase[nslot] = sp;
    sp->nslot = nslot;
    sp->fd = fd;
    cur_slot = sp;

    /* Record information about hostname or IP address */
    /* Note: this is a blocking I/O operation! */
    hp = gethostbyaddr ((char *) &sockaddr.sin_addr,
                        sizeof (struct in_addr),
                        sockaddr.sin_family);
    if (hp == NULL) {
      if (h_errno == NO_RECOVERY) {
        /* something is odd with the connection */
        slotbase[nslot] = NULL;
        free (sp);
        close (fd);
        return;
      }
      strncpy (sp->hostname, inet_ntoa(sockaddr.sin_addr), MAXHOST-1);
    } else
      strncpy (sp->hostname, hp->h_name, MAXHOST-1);

    sp->temp = NULL;
    sp->compls = NULL;

    if (!qcreate (&sp->outq, MAXOUTQ) ||
        !qcreate (&sp->stopq, MAXSTOPQ)) {
      qdispose (&sp->outq);
      qdispose (&sp->stopq);
      slotbase[nslot] = NULL;
      free (sp);
      close (fd);
      return;
    }

    if (nslot > slots_used)
      slots_used = nslot;
    if (fd > maxfd_used)
      maxfd_used = fd;

    FD_SET (fd, &used);

    /*** Initialize more fields explicitely? ***/
    sp->flags.gotcr = 0;
    sp->inp = sp->in;
    sp->wrap_ptr = sp->wrap_base;
    sp->flags.wstop = sp->flags.wrap = 0;
    sp->tsmode = TS_NONE;

    send_ts (WILL, TELOPT_SGA);
    send_ts (WILL, TELOPT_ECHO);

    sprintf (msg_buf, "\n%s\n%s\nLinked on %s\n\n",
             VERSION, COPYRIGHT, LinkDate);
    writestr (sp, msg_buf);

    if (read_prelogin())
      writech (sp, '\n');;

    writestr (sp, "These commands can be used"
                  " instead of real member name:\n");
    if (newusers_f) {
      writestr (sp, "   \"n\" or \"new\" to register as a new user,\n");
    }
    writestr (sp,
              "   \"w\" or \"who\" to see who is currently online,\n");
    writestr (sp,
              "   \"r\" or \"recent\" to see who has been online recently,\n");
    writestr (sp,
              "   \"q\" or \"quit\" to close connection immediately.\n\n");

    writestr (sp, ENTER_LOGIN);
    setproc (login, MAXID, 0);
  }
}

/*******************************************************************
 *
 *  Get a list of filehandles pending for write operations.
 *    w: bit array to store the result.
 *
 *******************************************************************/

static void get_write (fd_set *w)
{
  int nslot;
  SLOT *sp;

  FD_ZERO (w);

  for (nslot = 1; nslot <= slots_used; nslot++) {
    sp = slotbase[nslot];
    if (!sp)
      continue;
    if (!qempty (&sp->outq) ||
        (!qempty (&sp->stopq) && !sp->flags.stopped))
      FD_SET (sp->fd, w);
  }
}

/*******************************************************************
 *
 *  Show usage syntax for the program.
 *
 *******************************************************************/

static void Usage (char *name)
{
  printf ("\nUsage: %s [-n] [-fg] [-p<PORT>]\n\n", name);
  printf ("Options:\n");
  printf ("  -n        Don't print any startup info (useful for crontabs).\n");
  printf ("  -fg       Don't fork a separate process.\n");
  printf ("  -p<PORT>  Start the process on TCP port <PORT>.\n\n");
}

/*******************************************************************
 *
 *  main() function.
 *
 *******************************************************************/

int main (int argc, char **argv)
{
  int i;
  struct rlimit lim;
  FILE *pid_file;
  pid_t my_pid;

  setlocale (LC_ALL, "");
  
  if (argc > 3) {
    Usage (argv[0]);
    return 1;
  }

  for (i = 1; i < argc; i++) {
    if (strcmp (argv[i], "-fg") == 0) {
      noforkFlag = 1;
    } else if (strncmp (argv[i], "-p", 2) == 0) {
      inet_port = atoi (argv[i]+2);
    } else if (strncmp (argv[i], "-n", 2) == 0) {
      if ((freopen ("/dev/null", "a", stdout) == NULL))
        printf ("Failed to redirect streams for use with crontab.\n");
      if ((freopen ("/dev/null", "a", stderr) == NULL))
        printf ("Failed to redirect streams for use with crontab.\n");
    } else {
      Usage (argv[0]);
      return 1;
    }
  }

  startup_time = time (NULL);

  printf ("Server starting up...\n");

  if (!read_conf()) {
    fprintf (stderr, "telechat-ng: Cannot read run-time configuration.\n");
    exit(1);
  }

  printf ("Starting on Internet port %d\n", inet_port);
  printf ("Working directory is %s\n", base_dir);

  /* Initialize some important globals */
  initvars();

  /* Open up file pointers to log files */
  open_logs();

  /* Change current directory */
  if (chdir (base_dir))
    panic ("telechat-ng: chdir");

  /* Ensure the DB files can be accessed, create them if necessary */
  if (db_init_files() != DB_SUCCESS) {
    fprintf (stderr, "telechat-ng: Cannot access users database.\n");
    exit (1);
  }

  /* Limit the number of file descriptors */
  getrlimit (RLIMIT_NOFILE, &lim);
  if (max_files && max_files < 5) {
    fprintf (stderr,
             "%s: Requested max number of open files is too small,"
             " using 10.\n", argv[0]);
    lim.rlim_cur = 10;
  } else if (max_files > lim.rlim_max) {
    fprintf (stderr,
             "%s: Requested max number of open files is too big,"
             " using %d.\n", argv[0], lim.rlim_max);
    lim.rlim_cur = lim.rlim_max;
  } else {
    lim.rlim_cur = max_files;
  }
  if (max_files && setrlimit (RLIMIT_NOFILE, &lim))
    perror ("setrlimit");

  /* Open the socket and bind to it */
  initsock();

  /* Open up some slots for users */
  initslots();

  /* Set up array of names for named channels */
  initchannels();

  /* Maybe fork the process to the background */
  if (!noforkFlag) {
    if (fork ())
      return 0;

    /* Signals related to tty i/o */
    signal (SIGTTIN, SIG_IGN);
    signal (SIGTTOU, SIG_IGN);
    signal (SIGTSTP, SIG_IGN);

    /* If we are forked, we don't want to print out to stdout or stderr */
    closestd ();
  }

  set_signals ();

  sprintf (msg_buf, "Server started on port %d at %.24s",
	   inet_port, ctime (&startup_time));
  write_log_system (msg_buf);

  my_pid = getpid();
  pid_file = fopen (pid_fname, "w");
  if (pid_file == NULL)
    panic ("telechat-ng: fopen");

  fprintf (pid_file, "%d", my_pid);
  fclose (pid_file);

  mainloop();
  return 0;
}

