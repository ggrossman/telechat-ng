/*******************************************************************
 *
 * $Id: db.c,v 1.11 2000/06/01 08:32:27 const Exp $
 * Implementation of the DB interface.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/

#include "telechat.h"

/* These global vars should be static */
char *acct_fname;
char *acct_passwords;

static GDBM_FILE static_dbf = NULL;
static datum static_key;

static void fill_acct (ACCOUNT *acct, datum record);
static int acct_is_valid (ACCOUNT *acct);

/*******************************************************************
 *
 *  Compose and save names for password and account database files.
 *    dir: path to database directory ("" means root directory);
 *    pwd: filename for password DB;
 *    acct: filename for account DB.
 *
 *******************************************************************/

void db_set_filenames (char *dir, char *pwd, char *acct)
{
  int path_len;

  path_len = strlen (dir);

  acct_fname = (char *) malloc (path_len + strlen(acct) + 2);
  acct_passwords = (char *) malloc (path_len + strlen(pwd) + 2);

  sprintf (acct_fname, "%s/%s", dir, acct);
  sprintf (acct_passwords, "%s/%s", dir, pwd);
}

/*******************************************************************
 *
 *  Check existance of password and account database files and create
 *  new empty DB files if necessary.
 *
 *  Return codes: DB_SUCCESS, DB_EOPEN.
 *
 *******************************************************************/

enum db_err db_init_files (void)
{
  GDBM_FILE dbf1, dbf2;

  dbf1 = gdbm_open (acct_passwords, DB_CHUNK, GDBM_WRCREAT, DB_MODE, NULL);
  if (dbf1 != NULL) {
    gdbm_close (dbf1);
  } else {
    /* It's mostly ok for the password database to be read-only. */
    dbf1 = gdbm_open (acct_passwords, DB_CHUNK, GDBM_READER, DB_MODE, NULL);
    if (dbf1 != NULL)
      gdbm_close (dbf1);
  }
  dbf2 = gdbm_open (acct_fname, DB_CHUNK, GDBM_WRCREAT, DB_MODE, NULL);
  if (dbf2 != NULL)
    gdbm_close (dbf2);

  return (dbf1 != NULL && dbf2 != NULL) ? DB_SUCCESS : DB_EOPEN;
}

/*******************************************************************
 *
 *  Change password field in the passwords database.
 *    id: account name;
 *    enc_pwd: new password, encrypted with crypt().
 *
 *  Return codes: DB_SUCCESS, DB_EOPEN, DB_EUPDATE.
 *
 *******************************************************************/

enum db_err db_pwd_change (char *id, char *enc_pwd)
{
  GDBM_FILE dbf;
  datum key, record;
  int ret;

  dbf = gdbm_open (acct_passwords, DB_CHUNK, GDBM_WRCREAT, DB_MODE, NULL);
  if (dbf == NULL)
    return DB_EOPEN;

  key.dptr = id;
  key.dsize = strlen (id);
  record.dptr = enc_pwd;
  record.dsize = strlen (enc_pwd);

  ret = gdbm_store (dbf, key, record, GDBM_REPLACE);
  gdbm_close (dbf);

  return (ret == 0) ? DB_SUCCESS : DB_EUPDATE;
}

/*******************************************************************
 *
 *  Authenticate user via passwords database.
 *    id: account name;
 *    pwd: cleartext password.
 *
 *  Return codes: DB_SUCCESS, DB_EOPEN, DB_ENOTFOUND, DB_EBADREC,
 *                DB_EPASSWORD.
 *
 *  Note: this function destroys contents of pwd[] if password
 *        is proved to be correct.
 *
 *******************************************************************/

enum db_err db_pwd_compare (char *id, char *pwd)
{
  GDBM_FILE dbf;
  datum key, record;
  int ret;

  dbf = gdbm_open (acct_passwords, DB_CHUNK, GDBM_READER, DB_MODE, NULL);
  if (dbf == NULL) {
    ret = DB_EOPEN;
  } else {
    key.dptr = id;
    key.dsize = strlen (id);
    record = gdbm_fetch (dbf, key);
    gdbm_close (dbf);

    if (record.dptr == NULL) {
      ret = DB_ENOTFOUND;
    } else {
      if (record.dsize != 13) {
        ret = DB_EBADREC;
      } else if (memcmp (record.dptr, crypt(pwd, record.dptr), 13)) {
        ret = DB_EPASSWORD;
      } else {
        memset (pwd, '\0', strlen(pwd));
        ret = DB_SUCCESS;
      }
      free (record.dptr);
    }
  }
  return ret;
}

/*******************************************************************
 *
 *  Check if valid user's password exists in the database.
 *    id: account name.
 *
 *  Return codes: DB_SUCCESS, DB_EOPEN, DB_ENOTFOUND, DB_EBADREC.
 *
 *******************************************************************/

enum db_err db_pwd_exists (char *id)
{
  GDBM_FILE dbf;
  datum key, record;

  dbf = gdbm_open (acct_passwords, DB_CHUNK, GDBM_READER, DB_MODE, NULL);
  if (dbf == NULL)
    return DB_EOPEN;

  key.dptr = id;
  key.dsize = strlen (id);
  record = gdbm_fetch (dbf, key);
  gdbm_close (dbf);

  if (record.dptr == NULL)
    return DB_ENOTFOUND;

  free (record.dptr);

  return (record.dsize == 13) ? DB_SUCCESS : DB_EBADREC;
}

/*******************************************************************
 *
 *  Delete password record from the passwords database.
 *    id: account name.
 *
 *  Return codes: DB_SUCCESS, DB_EOPEN, DB_NOTFOUND.
 *
 *******************************************************************/

enum db_err db_pwd_delete (char *id)
{
  GDBM_FILE dbf;
  datum key;
  int ret = 0;

  dbf = gdbm_open (acct_passwords, DB_CHUNK, GDBM_WRCREAT, DB_MODE, NULL);
  if (dbf == NULL)
    return DB_EOPEN;

  key.dptr = id;
  key.dsize = strlen (id);
  ret = gdbm_delete (dbf, key);
  gdbm_close (dbf);

  return (ret == 0) ? DB_SUCCESS : DB_ENOTFOUND;
}

/*******************************************************************
 *
 *  Read account record from the account database.
 *    id: account name;
 *    acct: place to store the result.
 *
 *  Return codes: DB_SUCCESS, DB_EOPEN, DB_NOTFOUND, DB_EBADREC.
 *
 *******************************************************************/

enum db_err db_acct_read (char *id, ACCOUNT *acct)
{
  GDBM_FILE dbf;
  datum key, record;
  int ret;

  dbf = gdbm_open (acct_fname, DB_CHUNK, GDBM_READER, DB_MODE, 0);

  if (dbf == NULL)
    return DB_EOPEN;

  key.dptr = id;
  key.dsize = strlen (id);
  record = gdbm_fetch (dbf, key);
  gdbm_close (dbf);

  if (record.dptr == NULL)
    return DB_ENOTFOUND;

  fill_acct (acct, record);
  free (record.dptr);

  return acct_is_valid(acct) ? DB_SUCCESS : DB_EBADREC;
}

/*******************************************************************
 *
 *  Save account record in the account database.
 *    acct: account record.
 *
 *  Return codes: DB_SUCCESS, DB_EOPEN, DB_EUPDATE.
 *
 *******************************************************************/

enum db_err db_acct_write (ACCOUNT *acct)
{
  GDBM_FILE dbf;
  datum keydata, record;
  int ret;

  dbf = gdbm_open (acct_fname, DB_CHUNK, GDBM_WRCREAT, DB_MODE, 0);
  if (dbf == NULL)
    return DB_EOPEN;

  keydata.dptr = acct->id;
  keydata.dsize = strlen (acct->id);
  record.dptr = (char *) acct;
  record.dsize = sizeof(ACCOUNT);
  ret = gdbm_store (dbf, keydata, record, GDBM_REPLACE);
  gdbm_close (dbf);

  return (ret < 0) ? DB_EUPDATE : DB_SUCCESS;
}

/*******************************************************************
 *
 *  Delete account record from the account database.
 *    id: account name.
 *
 *  Return codes: DB_SUCCESS, DB_EOPEN, DB_NOTFOUND.
 *
 *  Note: this function does not try to delete corresponding record
 *        from the password database.
 *
 *******************************************************************/

enum db_err db_acct_delete (char *id)
{
  GDBM_FILE dbf;
  datum keydata;
  int ret;

  dbf = gdbm_open (acct_fname, DB_CHUNK, GDBM_WRCREAT, DB_MODE, 0);
  if (dbf == NULL)
    return DB_EOPEN;

  keydata.dptr = id;
  keydata.dsize = strlen(id);

  ret = gdbm_delete (dbf, keydata);
  gdbm_close (dbf);

  return (ret == 0) ? DB_SUCCESS : DB_ENOTFOUND;
}

/*******************************************************************
 *
 *  Begin sequential fetching of records from the account DB.
 *    acct: place to store the result.
 *
 *  Return codes: DB_SUCCESS, DB_EBADREC, DB_EOPEN, DB_NOTFOUND.
 *
 *  Note: The fetching process must stop at the first DB_EOPEN or
 *        DB_ENOTFOUND error, because database file get closed in
 *        these cases. But if db_acct_getfirst() or db_acct_getnext()
 *        return DB_EBADREC, you are supposed either to continue call
 *        db_acct_getnext() or to call db_acct_getdone() explicitely.
 *
 *******************************************************************/

enum db_err db_acct_getfirst (ACCOUNT *acct)
{
  datum record;

  static_dbf = gdbm_open (acct_fname, DB_CHUNK, GDBM_READER, DB_MODE, 0);
  if (static_dbf == NULL)
    return DB_EOPEN;

  static_key = gdbm_firstkey (static_dbf);
  if (static_key.dptr == NULL) {
    db_acct_getdone();
    return DB_ENOTFOUND;
  }
  record = gdbm_fetch (static_dbf, static_key);
  if (record.dptr == NULL) {
    db_acct_getdone();
    return DB_ENOTFOUND;
  }

  fill_acct (acct, record);
  free (record.dptr);

  return acct_is_valid(acct) ? DB_SUCCESS : DB_EBADREC;
}

/*******************************************************************
 *
 *  Continue sequential fetching of records from the account DB.
 *    acct: place to store the result.
 *
 *  Return codes: DB_SUCCESS, DB_EBADREC, DB_EOPEN, DB_NOTFOUND.
 *
 *  Note: The fetching process must stop at the first DB_EOPEN or
 *        DB_ENOTFOUND error, because database file get closed in
 *        these cases. But if db_acct_getfirst() or db_acct_getnext()
 *        return DB_EBADREC, you are supposed either to continue call
 *        db_acct_getnext() or to call db_acct_getdone() explicitely.
 *
 *******************************************************************/

enum db_err db_acct_getnext (ACCOUNT *acct)
{
  datum nextkey, record;

  if (static_dbf == NULL)
    return DB_EOPEN;

  nextkey = gdbm_nextkey (static_dbf, static_key);
  if (nextkey.dptr == NULL) {
    db_acct_getdone();
    return DB_ENOTFOUND;
  }

  free (static_key.dptr);
  static_key = nextkey;

  record = gdbm_fetch (static_dbf, static_key);
  if (record.dptr == NULL) {
    db_acct_getdone();
    return DB_ENOTFOUND;
  }

  fill_acct (acct, record);
  free (record.dptr);

  return acct_is_valid(acct) ? DB_SUCCESS : DB_EBADREC;
}

/*******************************************************************
 *
 *  Finish sequential fetching of records from the account DB.
 *
 *  Note: This function is called automatically when
 *        db_acct_getfirst() or db_acct_getnext() cannot return
 *        next account record. But it is safe to call it more than
 *        one time. If you do not get either DB_ENOTFOUND or
 *        DB_EOPEN error during sequential fetching or DB records,
 *        you MUST call db_acct_getdone() explicitely.
 *
 *******************************************************************/

void db_acct_getdone (void)
{
  if (static_dbf != NULL) {
    gdbm_close (static_dbf);
    static_dbf = NULL;
  }
  if (static_key.dptr != NULL) {
    free (static_key.dptr);
    static_key.dptr = NULL;
  }
}

/*******************************************************************
 *
 *  Fill in account structure from the database record.
 *    acct: account record to store the result;
 *    record: actual record read from the DB.
 *
 *  Note: This is currently done in a dangerous way. We need to
 *        init all fields before copying record from the DB, if
 *        its size is less than sizeof(ACCOUNT).
 *
 *******************************************************************/

static void fill_acct (ACCOUNT *acct, datum record)
{
  if (record.dsize > sizeof(ACCOUNT)) {
    memcpy (acct, record.dptr, sizeof(ACCOUNT));
  } else {
    memcpy (acct, record.dptr, record.dsize);
  }
}

/*******************************************************************
 *
 *  Check correctness of the account record fields, repair record
 *  if possible.
 *    acct: account structure.
 *
 *  Return values:
 *    0: account record is broken, cannot recover;
 *    1: account record is valid or recovered.
 *
 *******************************************************************/

static int acct_is_valid (ACCOUNT *acct)
{
  return 1;
}

