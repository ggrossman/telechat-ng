/*******************************************************************
 *
 * $Id: users.c,v 1.13 2000/06/11 08:25:53 const Exp $
 * Account manipulation routines.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/
/***               Remove this file before 0.9.6                 ***/

#include "telechat.h"

int read_user (char *id, ACCOUNT *acct)
{
  return (db_acct_read (id, acct) == DB_SUCCESS) ? 1 : -1;
}

int write_user (ACCOUNT *acct)
{
  return (db_acct_write (acct) == DB_SUCCESS) ? 1 : -1;
}

int delete_user (char *id)
{
  enum db_err err;

  err = db_acct_delete (id);
  /*** Try to delete both passwords and accounts in any cases ***/
  if (err == DB_EOPEN) {
    writestr (cur_slot, "Error deleting user: can't open account file.\n");
  } else if (err != DB_SUCCESS) {
    writestr (cur_slot, "Error deleting user account.\n");
  } else {
    err = db_pwd_delete (id);
    if (err == DB_EOPEN) {
      writestr (cur_slot, "Error deleting user: can't open password file.\n");
    } else if (err != DB_SUCCESS) {
      writestr (cur_slot, "Error deleting user password.\n");
    }
  }
  return (err == DB_SUCCESS) ? 1 : -1;
}

