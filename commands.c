/*******************************************************************
 *
 * $Id: commands.c,v 1.25 2000/06/11 08:14:08 const Exp $
 * Code to handle common user's commands.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 * Copyright 1996 Hyperreal.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/

#include "telechat.h"

/* A command specification */
typedef struct {
  char ch;                      /* the letter */
  int level;                    /* the level the user must be at
                                   to issue this comand */
  void (*func)();               /* the function to call */
  char *anno;                   /* a short text annotation */
} CMD;

static void help_summary (CMD *cmdset, int cmdnum, char *header);
static void cmd_help (void);
static void adv_help (void);
static void cmd_version (void);
static void cmd_unroot (void);
static void handle2 (char *h);
static void handle (void);
static void togglestat (void);
static void setwidth2 (char *buf);
static void setwidth (void);
static void toggle_nl (void);
static void change_nl2 (char *ch);
static void change_nl (void);
static void change_pw4 (char *pw);
static void change_pw3 (char *pw);
static void change_pw2 (char *pw);
static void change_pw (void);
static void cmd_ystats (void);
static void notify_tog (void);
static void print_time (void);
static void cmd_read_prelogin (void);
static void cmd_read_postlogin (void);
static void cmd_clear_screen (void);

static CMD cmds_basic[] =
{
  '?', 0, cmd_help,            "List basic system commands",
  '+', 0, adv_help,            "List advanced system commands",
  'p', 0, cmd_pmail,           "Send private message",
  'm', 0, cmd_offmsg_read,     "Read offline messages",
  'w', 0, cmd_emote,           "Emote",
  'a', 0, active,              "Show who is logged in",
  's', 0, see,                 "See how many people are logged in",
  'n', 0, handle,              "Change nickname",
  'i', 1, inquire,             "Inquire about a user",
  'k', 4, cmd_kickoff,         "Kick off another user",
  '=', 4, op_func,             "Operator/Moderator functions",
  'q', 0, cmd_logoff,          "Log off",
};

static CMD cmds_advanced[] =
{
#ifndef NOBROADCAST
  'd', 1, cmd_station,         "Broadcast station message",
  'D', 1, cmd_broadcast_emote, "Broadcast emote",
#endif
  'b', 1, cmd_beep,            "Send a beep to another user",
  'M', 0, cmd_pmail_offmsg,    "Send private offline message",
  ',', 0, cmd_pmail_reply,     "Reply to private message",
  '/', 0, cmd_pmail_again,     "Send one more private message",

  'x', 0, cmd_squelch,         "Squelch another user",
  'r', 0, cmd_rsquelch,        "Reverse squelch",

  'c', 0, get_channel,         "Change channel",
  'N', 1, name_channel,        "Name current channel",
  'U', 1, unname_ch,           "Un-name current channel",
  'C', 1, list_channel,        "List users on channel",

  'e', 0, cmd_typing,          "Check for other users typing",
  'E', 0, cmd_room_typing,     "Check users typing on channel",
  '-', 0, list_recent,         "List recent users",
  't', 0, print_time,          "Display current time",

  'l', 0, cmd_clear_screen,    "Clear screen",
  'y', 0, cmd_ystats,          "Display your stats",
  'v', 0, cmd_version,         "Print code version info",

  '1', 0, cmd_read_prelogin,   "Read pre-login message",
  '2', 0, cmd_read_postlogin,  "Read post-login message",

/* commands for controlling personal settings */
  '&', 0, change_pw,           "Change password",
  '@', 0, change_email,        "Change email address",
  'F', 0, cmd_toggle_email,    "Select email status",
  '3', 0, setwidth,            "Set screen width",
  '#', 0, toggle_color,        "Toggle ANSI colors",
  'g', 0, choose_msg_color,    "Select color of your messages",
#ifndef NOBROADCAST
  '4', 1, togglestat,          "Select station messages",
#endif
  '6', 1, cmd_togglebeeps,     "Select beeping mode",
  '7', 1, notify_tog,          "Select monitoring of PA Messages",
  '*', 1, change_fmt,          "Change message/active format",
  'j', 1, toggle_nl,           "Select newlines",
  'u', 0, change_nl,           "Change newline character",

/* administrative commands */
  'z', 5, cmd_broadcast,       "Broadcast PA message",
  '|', 0, cmd_level,           "Go to higher access level",
  '5', 5, cmd_unroot,          "Go to access level 1",
};

void exec_cmd (char ch)
{
  CMD *cmdset;
  int i, set, cmdlen;

  for (set = 0; set < 2; set++) {
    cmdset = (set == 0) ? cmds_basic : cmds_advanced;
    cmdlen = (set == 0) ?
      sizeof(cmds_basic)/sizeof(CMD) :
      sizeof(cmds_advanced)/sizeof(CMD);

    for (i = 0; i < cmdlen; i++) {
      if (ch == cmdset[i].ch) {
        /* If the user is not at the right level, don't call the function */
        if (cmdset[i].level > cur_slot->acct.level)
          break;

        /* Don't call functions disabled by run-time configuration */
        if (cmdset[i].func == cmd_room_typing && !roomtyping_f)
          break;
        if (cmdset[i].func == change_pw && !chpasswd_f)
          break;

        /* Show the command annotation */
        sprintf (msg_buf, " -> %s\n", cmdset[i].anno);
        writestr (cur_slot, msg_buf);

        /* Call the function */
        cmdset[i].func();
        return;
      }
    }
  }
  writestr (cur_slot, " -> Invalid command, type /? for help.\n");
}

static void help_summary (CMD *cmdset, int cmdnum, char *header)
{
  int i;
  int pos = 0, next_pos;
  int col_width = 40;           /* Constant */

  writestr (cur_slot, header);
  for (i=0; i < cmdnum; i++) {
    if (cmdset[i].level > cur_slot->acct.level)
      continue;

    /* Don't list functions disabled by run-time configuration */
    if (cmdset[i].func == cmd_room_typing && !roomtyping_f)
      continue;
    if (cmdset[i].func == change_pw && !chpasswd_f)
      continue;

    next_pos = (pos / col_width + 1) * col_width;
    if (next_pos + col_width > cur_slot->acct.width) {
      writech (cur_slot, '\n');
      pos = 0;
    } else if (pos) {
      while (pos++ < next_pos)
        writech (cur_slot, ' ');
    }
    pos += sprintf (msg_buf, "/%c  %.*s",
                    cmdset[i].ch, col_width - 6, cmdset[i].anno);
    writestr (cur_slot, msg_buf);
  }
  writech (cur_slot, '\n');
}

static void cmd_help (void)
{
  help_summary (cmds_basic, sizeof(cmds_basic)/sizeof(CMD),
                "Basic commands available at current access level:\n");
}

static void adv_help (void)
{
  help_summary (cmds_advanced, sizeof(cmds_advanced)/sizeof(CMD),
                "Advanced commands available at current access level:\n");
}

static void cmd_version (void)
{
  writestr (cur_slot, VERSION);
  writech (cur_slot, '\n');
  writestr (cur_slot, COPYRIGHT);
  writech (cur_slot, '\n');
}

static void cmd_unroot (void)
{
  cur_slot->acct.level = 1;
  write_user (&cur_slot->acct);
  writestr (cur_slot, "You are now a mere mortal again...\n");
}

static void handle2 (char *h)
{
  if (h[0]) {
    if (!strcmp (h, "NONE"))
      h[0] = '\0';

    /* Inform @'s and %'s of the handle change */
    sprintf (msg_buf, " * %s/%s is now %s/%s",
             cur_slot->acct.id, cur_slot->acct.handle, cur_slot->acct.id, h);
    strcpy (cur_slot->acct.handle, h);
    write_user (&cur_slot->acct);
    write_channel (msg_buf, cur_slot->acct.chan, PA_MSG);
  }
  setproc (sendpub, MAXMSG, 0);
}

static void handle (void)
{
  writestr (cur_slot, "New nickname: ");
  setproc (handle2, MAXHANDLE, COMPL_NONE);
}

#ifndef NOBROADCAST

static void togglestat (void)
{
  cur_slot->acct.nostat = !cur_slot->acct.nostat;
  if (cur_slot->acct.nostat)
    writestr (cur_slot, "Broadcast messages off.\n");
  else
    writestr (cur_slot, "Broadcast messages on.\n");
}

#endif /* NOBROADCAST */

static void setwidth2 (char *buf)
{
  int i;

  if (buf[0]) {
    if ((i = atoi (buf)) >= MINWIDTH && i <= MAXWIDTH) {
      cur_slot->acct.width = i;
      writestr (cur_slot, "Screen width changed.\n");
    } else {
      sprintf (msg_buf, "Value must be between %d and %d.\n",
               MINWIDTH, MAXWIDTH);
      writestr (cur_slot, msg_buf);
      return;
    }
  }
  setproc (sendpub, MAXMSG, 0);
}

static void setwidth (void)
{
  sprintf (msg_buf,
           "Current screen width is %d.\n"
           "New value: ",
           cur_slot->acct.width);
  writestr (cur_slot, msg_buf);
  setproc (setwidth2, 4, COMPL_NONE);
}

static void toggle_nl (void)
{
  cur_slot->acct.newlines = !cur_slot->acct.newlines;
  writestr (cur_slot, cur_slot->acct.newlines ?
            "Newlines on\n" : "Newlines off\n");
}

static void change_nl2 (char *ch)
{
  if (ch[0]) {
    cur_slot->acct.nlchar = ch[0];
    sprintf (msg_buf, "Newline character changed to \"%c\".\n",
             (int)cur_slot->acct.nlchar);
    writestr (cur_slot, msg_buf);
  }
  setproc (sendpub, MAXMSG, 0);
}

static void change_nl (void)
{
  sprintf (msg_buf,
           "Newline character is \"%c\".\n"
           "Enter newline character: ",
           (int)cur_slot->acct.nlchar);
  writestr (cur_slot, msg_buf);
  setproc (change_nl2, 2, COMPL_NONE);
}

static void change_pw4 (char *pw)
{
  if (strcmp (pw, cur_slot->temp)) {
    writestr (cur_slot, "Passwords do not match.\n");
  } else {
    if (db_pwd_change (cur_slot->acct.id, crypt(pw, salt)) == DB_SUCCESS) {
      writestr (cur_slot, "Password changed.\n");
      operact (cur_slot, (SLOT *)NULL, "changed password");
    } else
      writestr(cur_slot, "Password change failed.\n");
  }
  freetemp (cur_slot);
  setproc (sendpub, MAXMSG, 0);
}

static void change_pw3 (char *pw)
{
  if (!alloctemp (cur_slot, MAXPW +1)) {
    setproc (sendpub, MAXMSG, 0);
    return;
  }

  strcpy (cur_slot->temp, pw);
  writestr (cur_slot, "Enter password again to verify: ");
  setproc (change_pw4, MAXPW, READ_NOECHO);
}

static void change_pw2 (char *pw)
{
  int err;

  err = db_pwd_compare (cur_slot->acct.id, pw);
  if (err == DB_SUCCESS) {
    writestr (cur_slot,
              "Enter new password (up to 12 characters in length): ");
    setproc (change_pw3, MAXPW, READ_NOECHO);
  } else {
    if (err == DB_EPASSWORD)
      writestr (cur_slot, "Password incorrect.\n");
    else
      writestr (cur_slot, "Password change failed.\n");
    setproc (sendpub, MAXMSG, 0);
  }
}

static void change_pw (void)
{
  writestr (cur_slot, "Enter old password: ");
  setproc (change_pw2, MAXPW, READ_NOECHO);
}

static void cmd_ystats (void)
{
  dump_userinfo (cur_slot);
}

static void notify_tog (void)
{
  cur_slot->acct.pa_notify = !cur_slot->acct.pa_notify;
  if (cur_slot->acct.pa_notify)
    writestr (cur_slot, "Your PA messages are now on\n");
  else
    writestr (cur_slot, "Your PA messages are now off\n");
}

static void print_time (void)
{
  time_t secs;
  
  secs = time (NULL);
  writestr (cur_slot, "Current time: ");
  writestr (cur_slot, ctime (&secs));
  writestr (cur_slot, "Time on: ");
  writetime (cur_slot, secs - cur_slot->login_time);
  writech (cur_slot, '\n');
}

/*******************************************************************
 *
 *  Set handler waiting for user login name to be entered on current
 *  slot.
 *    dispatch: handler function;
 *    mode: PROC_ID_ONLINE - look for online users only;
 *          PROC_ID_ALL - lookup for all registered users.
 *
 *  Description:
 *    If mode is PROC_ID_ONLINE, the handler will get (SLOT *) pointer
 *    to a slot found as the first argument (sp) and NULL as the
 *    second argument (id). If mode is PROC_ID_ALL, the handler will
 *    one of the following argument sets:
 *      * valid (SLOT *) pointer as sp and id == NULL, if that user is
 *        currently online;
 *      * sp == NULL and a valid login name as id, if such user exists
 *        but is currently offline.
 *    If such user can't be found, the handler won't be called and an
 *    error message will be written to the current slot.
 *
 *  Note: If slots_f flag set, any string beginning with a digit
 *        entered instead of user name will be treated as a slot
 *        number.
 *
 *******************************************************************/

void setproc_id (void (*dispatch)(SLOT *sp, char *id), int mode)
{
  cur_slot->dispid = dispatch;
  cur_slot->flags.extid = mode;
  setproc (proc_id, MAXID, COMPL_STRICT);
}

void proc_id (char *id)
{
  SLOT *sp = NULL;
  char *offline_id = NULL;
  int err = 0;

  if (!id[0]) {
    setproc (sendpub, MAXMSG, 0);
    return;
  }
  if (slots_f && isdigit(id[0])) {
    sp = slotbynumber (id);
    if (!sp) {
      sprintf (msg_buf, "No such active slot: %s.\n", id);
      err = 1;
    }
  } else {
    sp = slotbyname (id);
    if (!sp) {
      if (cur_slot->flags.extid) {
        if (id[0] != '_' && db_pwd_exists (id) == DB_SUCCESS) {
          offline_id = id;
        } else {
          sprintf (msg_buf, "No such user: \"%s\".\n", id);
          err = 1;
        }
      } else {
        sprintf (msg_buf, "No such user online: \"%s\".\n", id);
        err = 1;
      }
    }
  }
  if (err) {
    writestr (cur_slot, msg_buf);
    setproc (sendpub, MAXMSG, 0);
  } else
    cur_slot->dispid (sp, offline_id);
}

static void cmd_read_prelogin (void)
{
  read_prelogin();
}

static void cmd_read_postlogin (void)
{
  read_postlogin();
}

static void cmd_clear_screen (void)
{
  writestr (cur_slot, "\e[H\e[J");
}
