/*****************************************************************************
 *
 * NAME
 *   enter.c
 *
 * CONTENTS
 *    This file contains stuff for starting up the simulator
 *    and for when a user first logs in
 *
 *    Copyright 1996 Hyperreal.  All rights reserved.  For more information
 *    consult the file LICENSE.
 * 
 *****************************************************************************/

#include "telechat.h"

static void get_password (char *pw);
static void get_new_login (char *id);
static void get_new_handle (char *handle);
static void get_new_pw (char *pw);
static void get_new_email (char *email);
static void enter (void);
static void create_new_acct_record (char *pw);
static void set_initial_values (void);

char *salt = "XX";

void login (char *id)
{
  enum db_err err_pwd, err_acct;

  if (!id[0]) {
    setproc (login, MAXID, 0);
    writestr (cur_slot, ENTER_LOGIN);
  }
  else if (!strcmp(id, "who") || !strcmp(id, "w")) {
    cur_slot->acct.level = 1;
    /*** Change to "    %u/%h [%M]  (%t)" after DB format changing !!! ***/
    strcpy (cur_slot->acct.activefmt, "    %u/%h [%M] %t");
    active();
    writestr (cur_slot, ENTER_LOGIN);
    setproc (login, MAXID, 0);
  }
  else if (!strcmp(id, "recent") || !strcmp(id, "r")) {
    list_recent();
    writech (cur_slot, '\n');
    writestr (cur_slot, ENTER_LOGIN);
    setproc (login, MAXID, 0);
  }
  else if (newusers_f && (!strcmp(id, "new") || !strcmp(id, "n"))) {
    writestr (cur_slot, "\nWelcome, new user!\n\n"
                        "Please enter a name for yourself.\n"
                        "This will be your account name: ");
    setproc (get_new_login, MAXID, 0);
  }
  else if (!strcmp(id, "quit") || !strcmp(id, "q")) {
    writestr (cur_slot, "\n");
    transmit (cur_slot);
    inactivate_slot (cur_slot, 1);
  }
  else if (login_is_valid(id) != VAL_OK) {
    writestr (cur_slot, "Invalid login name.\n\n");
    setproc (login, MAXID, 0);
    writestr (cur_slot, ENTER_LOGIN);
  }
  else {
    err_pwd = db_pwd_exists (id);
    err_acct = db_acct_read (id, &cur_slot->acct);

    /* Registered users */
    if (err_pwd == DB_SUCCESS && err_acct == DB_SUCCESS) {
      writestr (cur_slot, "Password: ");
      setproc (get_password, MAXPW, READ_NOECHO);
    }
    /* Nonexistent users */
    else if (err_pwd == DB_ENOTFOUND) {
      if (err_acct != DB_ENOTFOUND)
        db_acct_delete (id);
      writestr (cur_slot, "No account by that name.\n\n");
      writestr (cur_slot, ENTER_LOGIN);
      setproc (login, MAXID, 0);
    }
    /* Users with passwords but with no account record */
    else if (err_pwd == DB_SUCCESS && err_acct == DB_ENOTFOUND) {
      strcpy (cur_slot->acct.id, id);
      strcpy (cur_slot->acct.handle, id);
      writestr (cur_slot, "Password: ");
      setproc (create_new_acct_record, MAXPW, READ_NOECHO);
    }
    /* Problem with password or account database */
    else {
      writestr (cur_slot,
                "Sorry, the system is not able to log you in\n"
                "because of an internal database problem.\n\n");
      writestr (cur_slot, ENTER_LOGIN);
      setproc (login, MAXID, 0);
    }
  }
}

static void get_password (char *pw)
{
  SLOT *sp;
  int i;

  switch (db_pwd_compare (cur_slot->acct.id, pw)) {
  case DB_SUCCESS:
    if (cur_loggedin >= max_users) {
      writestr (cur_slot,
                "Sorry, there are too many people logged in.\n\n");
      writestr (cur_slot, ENTER_LOGIN);
      setproc (login, MAXID, 0);
      return;
    }
    /* check the slot table to see if you are anywhere */
    for (i = 1; i <= slots_used; i++) {
      sp = slotbase[i];
      /* have we found someone who might be us? */
      if (sp && sp->flags.on && !strcmp(sp->acct.id, cur_slot->acct.id)) {
        memcpy (&cur_slot->acct, &sp->acct, sizeof(ACCOUNT));
        cur_slot->acct.last_logout = cur_slot->acct.last_typed = time(NULL);

        writech (cur_slot, '\n');
        begin_attr (cur_slot, 1);
        writestr (cur_slot, "The dead version of you has been kicked off.");
        reset_attr (cur_slot);
        writech (cur_slot, '\n');

        paabout (sp, "Account froze on");
        transmit (sp);

        remove_from_channel (sp, sp->acct.chan);
        remove_from_channel (sp, MAINLIST);

        inactivate_slot (sp, 0);
        break;
      }
    }
    enter();
    break;

  case DB_EPASSWORD:
    writestr (cur_slot, "Incorrect password.\n\n");
    writestr (cur_slot, ENTER_LOGIN);
    setproc (login, MAXID, 0);
    break;

  default:
    writestr (cur_slot, "Internal database problem.\n\n");
    writestr (cur_slot, ENTER_LOGIN);
    setproc (login, MAXID, 0);
    break;
  }
}

static void get_new_login (char *id)
{
  int val;
  char *s;
  ACCOUNT acct;
  enum db_err err;
  
  if (!id[0]) {
    writestr (cur_slot, "Please enter account name or \"q\" to disconnect: ");
    return;
  }

  if (!strcmp(id, "quit") || !strcmp(id, "q")) {
    writestr (cur_slot, "\n");
    transmit (cur_slot);
    inactivate_slot (cur_slot, 1);
    return;
  }

  /* Check if the login name is correct */
  val = login_is_valid(id);
  if (val == VAL_INVALID) {
    writestr (cur_slot,
              "Account names must be alphanumeric lowercase characters\n"
              "and start with a lowercase alphabetic character.\n"
              "Single-character names are not allowed.\n"
              "Please enter correct account name: ");
    return;
  } else if (val != VAL_OK) {
    writestr (cur_slot,
              "Sorry, that was an invalid or reserved account name.\n"
              "Please choose some another account name: ");
    return;
  }

  /* Make sure the login name is not used by someone else */
  err = db_pwd_exists (id);
  if (err == DB_EOPEN) {
    writestr (cur_slot,
              "Sorry, access to the account database failed.\n"
              "Please try to visit our server later.\n\n");
    transmit (cur_slot);
    inactivate_slot (cur_slot, 1);
    return;
  } else if (err != DB_ENOTFOUND) {
    writestr (cur_slot,
              "That login is already in use, enter another: ");
    return;
  }

  strcpy (cur_slot->acct.id, id);

  writestr (cur_slot,
            "\nEnter your initial nickname (it may be empty as well).\n"
            "You can change this at any time with the /n command.\n"
            "Nickname: ");
  setproc (get_new_handle, MAXHANDLE, 0);
}

static void get_new_handle (char *handle)
{
  if (!strcmp(handle, "quit") || !strcmp(handle, "q")) {
    writestr (cur_slot, "\n");
    transmit (cur_slot);
    inactivate_slot (cur_slot, 1);
    return;
  }

  strcpy (cur_slot->acct.handle, handle);
  writestr (cur_slot,
            "\nEnter your password (up to 12 characters in length): ");
  setproc (get_new_pw, MAXPW, READ_NOECHO);
}

static void get_new_pw (char *pw)
{
  if (db_pwd_change (cur_slot->acct.id, crypt(pw, salt)) != DB_SUCCESS) {
    writestr (cur_slot,
              "Cannot save the password."
              " Sorry, account creation failed.\n\n");
    writestr (cur_slot, ENTER_LOGIN);
    setproc (login, MAXID, 0);
    return;
  }

  if (verify_f)
    writestr (cur_slot, "\nEnter a verifiable email address: ");
  else
    writestr (cur_slot, "\nEnter your email address: ");

  setproc (get_new_email, MAXEMAIL, 0);
}

static void get_new_email (char *email)
{
  if (verify_f && !email[0]) {
    writestr (cur_slot,
              "You must give an email address to create an account.\n"
              "Enter a verifiable email address: ");
    setproc (get_new_email, MAXEMAIL, 0);
    return;
  }

  strcpy (cur_slot->acct.email_address, email);
  strcpy (cur_slot->acct.created_from, cur_slot->hostname);
  set_initial_values();

  if (write_user (&cur_slot->acct) < 0) {
    writestr (cur_slot, "Error creating account.\n\n");
    transmit (cur_slot);
    inactivate_slot (cur_slot, 1);
  } else {
    if (verify_f) {
      writestr (cur_slot,
                "\nYou have been temporarily granted an account.\n"
                "Once your email address has been verified,\n"
                "your account will become permanent.\n");
    }
    enter();
  }
}

static void enter (void)
{
  int users_online;

#ifdef NOBROADCAST
  cur_slot->acct.nostat = 0;
#endif

  writestr (cur_slot, "\n");
  if (read_postlogin())
    writestr (cur_slot, "\n");

  if (slots_f) {
    sprintf (msg_buf, "Entering %s on slot #%d.\n\n",
             SYSTEM_NAME, cur_slot->nslot);
  } else {
    sprintf (msg_buf, "Entering %s.\n\n",
             SYSTEM_NAME);
  }
  writestr (cur_slot, msg_buf);

  if (offmsg_check()) {
    begin_attr (cur_slot, 1);
    writestr (cur_slot,
              "You have offline messages (type /m to read).\n");
    reset_attr (cur_slot);
  }

  writestr (cur_slot,
            "To chat, just type the text and then press Enter.\n"
            "Type /? for help on basic commands.\n");

  setproc (sendpub, MAXMSG, 0);
  cur_slot->login_time = cur_slot->chan_time = time (NULL);
  cur_slot->flags.stopped = 0;

  FD_ZERO (&cur_slot->squelch);
  FD_ZERO (&cur_slot->reverse);

  cur_slot->pmail[0] = '\0';
  cur_slot->pmail_sent[0] = '\0';
  cur_slot->pmail_rcvd[0] = '\0';

  if (channels[cur_slot->acct.chan].name)
    cur_slot->flags.listed = 1;
  else
    cur_slot->flags.listed = 0;

  add_to_channel (cur_slot, cur_slot->acct.chan);
  add_to_channel (cur_slot, MAINLIST);

  cur_loggedin++;
  login_count++;

  users_online = cur_loggedin - closeslots_num;

  sprintf (msg_buf, "\nThere %s currently %d %s visiting.\n"
                    "Type /a to see who is here.\n\n",
           (users_online == 1) ? "is" : "are",
           users_online,
           (users_online == 1) ? "person" : "people");
  writestr (cur_slot, msg_buf);

  cur_slot->flags.on = 1;
  paabout (cur_slot, "Logged on");
}

static void create_new_acct_record (char *pw)
{
  SLOT *sp;

  switch (db_pwd_compare (cur_slot->acct.id, pw)) {
  case DB_SUCCESS:
    strcpy (cur_slot->acct.created_from, cur_slot->hostname);
    set_initial_values();
    if (write_user (&cur_slot->acct) < 0) {
      writestr (cur_slot, "Error creating account.\n\n");
      transmit (cur_slot);
      inactivate_slot (cur_slot, 1);
    } else {
      writestr (cur_slot,
                "\nAttention: the preferences for your account were lost,\n"
                "creating new account record using default settings.\n");
      enter();
    }
    break;

  case DB_EPASSWORD:
    writestr (cur_slot, "Incorrect password.\n\n");
    writestr (cur_slot, ENTER_LOGIN);
    setproc (login, MAXID, 0);
    break;

  default:
    writestr (cur_slot, "Internal database problem.\n\n");
    writestr (cur_slot, ENTER_LOGIN);
    setproc (login, MAXID, 0);
    break;
  }
}

static void set_initial_values (void)
{
  /* id[], nick[] and email[] should not be touched here */

  cur_slot->acct.chan = 1;
  cur_slot->acct.width = 80;
  cur_slot->acct.level = (verify_f) ? 0 : 1;
  cur_slot->acct.nlchar = '\\';

  cur_slot->acct.pa_notify = 1;
  cur_slot->acct.nostat = 0;
  cur_slot->acct.newlines = 1;
  cur_slot->acct.beeping = 1;
  cur_slot->acct.usecolor = 0;
  cur_slot->acct.msgcolor = 0;

  strcpy (cur_slot->acct.activefmt, DEF_ACT_FMT);
  strcpy (cur_slot->acct.msgfmt, DEF_MSG_FMT);

  cur_slot->acct.email_verified = (verify_f) ? 0 : 1;
  cur_slot->acct.email_public = 0;
  cur_slot->acct.last_typed = cur_slot->acct.last_logout = time (NULL);

  cur_slot->flags.listed = 0;
}

int read_prelogin (void)
{
  FILE *fp;
  int c;

  fp = fopen (prelogin_fname, "r");
  if (fp == NULL)
    return 0;

  while ((c = getc (fp)) != EOF)
    writech (cur_slot, c);
  fclose (fp);
  return 1;
}

int read_postlogin (void)
{
  FILE *fp;
  int c;

  fp = fopen (postlogin_fname, "r");
  if (fp == NULL)
    return 0;

  while ((c = getc (fp)) != EOF)
    writech (cur_slot, c);
  fclose (fp);
  return 1;
}

