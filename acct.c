/*****************************************************************************
 *
 * NAME
 *    acct.c
 *
 * CONTENTS
 *    This file contains accounting procedures
 *
 *    Copyright 1996 Hyperreal.  All rights reserved.  For more information
 *    consult the file LICENSE.
 *
 *****************************************************************************/
/***                     Has to be rewritten                               ***/

#include "telechat.h"

static void change_email2 (char *em);
static void acc_deluser (SLOT *sp, char *id);
static void acc_chlevel2 (char *str);
static void acc_chlevel (SLOT *sp, char *id);
static void acc_verify (SLOT *sp, char *id);
static void acc_unverify (SLOT *sp, char *id);
static void list_users (void);

static void acc_deluser (SLOT *sp, char *id)
{
  
  if (sp) {
    if (delete_user (sp->acct.id)) {
      operact (cur_slot, sp, "deletes");
      writestr (sp, "Your account has been deleted.\n");
      transmit (sp);
      remove_from_channel (sp, sp->acct.chan);
      remove_from_channel (sp, MAINLIST);
      inactivate_slot (sp, 1);
    } else {
      writestr (cur_slot, "Error deleting user.\n");
    }
  } else {
    if (delete_user(id))
      writestr (cur_slot, "User deleted.\n");
    else
      writestr (cur_slot, "Error deleting user.\n");
  }
  setproc (sendpub, MAXMSG, 0);
}

void acc_viewuser (SLOT *sp, char *id)
{
  ACCOUNT acct;

  if (sp && (sp->flags.on || cur_slot->acct.level == TOPLEVEL)) {
    if (cur_slot->acct.level >= TOPLEVEL - 1)
      dump_userinfo (sp);
    else
      dump_userinfoshort (&sp->acct, 1);
  } else {
    if (read_user (id, &acct) < 0)
      writestr (cur_slot, "Error reading information about user.\n");
    else
      dump_userinfoshort (&acct, 0);
  }
  setproc (sendpub, MAXMSG, 0);
}

static void acc_chlevel (SLOT *sp, char *id)
{
  if (!alloctemp (cur_slot, MAXID)) {
    setproc (sendpub, MAXMSG, 0);
    return;
  }
  strcpy (cur_slot->temp, (sp) ? sp->acct.id : id);
  writestr (cur_slot, "Level: ");
  setproc (acc_chlevel2, 2, COMPL_NONE);
}

static void acc_chlevel2 (char *str)
{
  int level;
  ACCOUNT acct;
  SLOT *sp;

  if (str[0]) {
    if (!isdigit(str[0]) || (level = xatoi (str)) > TOPLEVEL) {
      writestr (cur_slot, "Invalid level.\n");
    } else if ((sp = slotbyname (cur_slot->temp)) != NULL) {
      sp->acct.level = level;
      write_user (&sp->acct);
      writestr (cur_slot, "Level changed.\n");
    } else if (read_user (cur_slot->temp, &acct) < 0) {
      writestr (cur_slot, "Error reading user's account data.\n");
    } else {
      acct.level = level;
      write_user (&acct);
      writestr (cur_slot, "Level changed.\n");
    }
  }
  freetemp (cur_slot);
  setproc (sendpub, MAXMSG, 0);
}

static void acc_verify (SLOT *sp, char *id)
{
  ACCOUNT acct;

  if (sp) {
    sp->acct.email_verified = 1;
    sp->acct.level = 1;
    write_user (&sp->acct);
    writestr (cur_slot, "User verified.\n");
  } else if (read_user (id, &acct) < 0) {
    writestr (cur_slot, "Error reading information about user.\n");
  } else {
    acct.email_verified = 1;
    acct.level = 1;
    if (write_user (&acct) > 0)
      writestr (cur_slot, "User verified.\n");
    else
      writestr (cur_slot, "Verification failed.\n");
  }
  setproc (sendpub, MAXMSG, 0);
}

static void acc_unverify (SLOT *sp, char *id)
{
  ACCOUNT acct;

  if (sp) {
    sp->acct.email_verified = 0;
    write_user (&sp->acct);
    writestr (cur_slot, "User unverified.\n");
  } else if (read_user(id, &acct) < 0) {
    writestr (cur_slot, "Error reading information about user.\n");
  } else {
    acct.email_verified = 0;
    if (write_user(&acct) > 0)
      writestr (cur_slot, "User unverified.\n");
    else
      writestr (cur_slot, "Unverification failed.\n");
  }
  setproc (sendpub, MAXMSG, 0);
}

static void list_users (void)
{
  ACCOUNT acct;
  enum db_err err;
  int ver = 0;                  /* number of verified users */
  int unver = 0;                /* number of unverified users */

  err = db_acct_getfirst (&acct);
  while (err == DB_SUCCESS || err == DB_EBADREC) {
    if (err == DB_EBADREC) {
      writestr (cur_slot, "<corrupted account DB record skipped!>\n");
    } else {
      if (verify_f) {
        sprintf (msg_buf, "%s/%s\t%s\t%s\n",
                 acct.id, acct.handle, acct.email_address,
                 acct.email_verified ? "<verified>" : "<NEW>");
      } else {
        sprintf (msg_buf, "%s/%s\t%s\n",
                 acct.id, acct.handle, acct.email_address);
      }
      writestr (cur_slot, msg_buf);

      if (verify_f && acct.email_verified)
        ver++;
      else
        unver++;
    }
    err = db_acct_getnext (&acct);
  }       
  db_acct_getdone();

  if (verify_f) {
    sprintf (msg_buf, 
             "That gives a total of %d verified and %d unverified users,\n"
             "for a grand total of %d users.\n",
             ver, unver, ver+unver);
  } else
    sprintf (msg_buf, "That gives a total of %d users.\n", unver);

  writestr (cur_slot, msg_buf);
}

void accounting (char *ch)
{
  switch (*ch)
    {
    case 'D': 
    case 'd': 
      if (nodelete_f) {
        setproc (sendpub, MAXMSG, 0);
      } else {
        writestr (cur_slot, "Who do you want to delete? ");
        setproc_id (acc_deluser, PROC_ID_ALL);
      }
      break;
      
    case 'V': 
    case 'v': 
      writestr(cur_slot, "Who do you want to view? ");
      setproc_id (acc_viewuser, PROC_ID_ALL);
      break;
      
    case 'E': 
    case 'e': 
      if (verify_f) {
        writestr (cur_slot, "Who do you want to verify? ");
        setproc_id (acc_verify, PROC_ID_ALL);
      } else
        setproc (sendpub, MAXMSG, 0);
      break;
      
    case 'U': 
    case 'u': 
      if (verify_f) {
        writestr (cur_slot, "Who do you want to unverify? ");
        setproc_id (acc_unverify, PROC_ID_ALL);
      } else
        setproc (sendpub, MAXMSG, 0);
      break;

    case 'C': 
    case 'c': 
      writestr (cur_slot, "Who's level do you want to change? ");
      setproc_id (acc_chlevel, PROC_ID_ALL);
      break;

    case 'L': 
    case 'l': 
      writestr (cur_slot, "Current users:\n");
      list_users();
      setproc (sendpub, MAXMSG, 0);
      break;
      
    default: 
      setproc (sendpub, MAXMSG, 0);
    }
}

static void change_email2 (char *em)
{
  if (!em[0]) {
    writestr (cur_slot, "Email address unchanged.\n");
  } else {
    strcpy (cur_slot->acct.email_address, em);
    writestr (cur_slot, "Email address changed.\n");
    operact (cur_slot, (SLOT *)NULL, "changed email");
#ifdef EMAIL_CHANGE_UNVERIFY
    writestr (cur_slot, "Your new email address is unverified.\n"
                    "Please submit a new verification request\n.");
    cur_slot->acct.email_verified = 0;
#endif
  }
  setproc (sendpub, MAXMSG, 0);
}

void change_email (void)
{
  writestr (cur_slot, "Your current email is: ");
  writestr (cur_slot, cur_slot->acct.email_address);
  writestr (cur_slot, "\nEnter new email: <CR> for no change\n");
  setproc (change_email2, MAXEMAIL, COMPL_NONE);
}

