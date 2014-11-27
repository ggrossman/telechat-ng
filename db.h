/*******************************************************************
 *
 * $Id: db.h,v 1.8 2000/05/31 18:26:15 const Exp $
 * Declaration of the DB interface.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/

#ifndef _telechatDB_H
#define _telechatDB_H

#include <gdbm.h>

#define DB_CHUNK  512
#define DB_MODE   0600

enum db_err {
  DB_SUCCESS = 0,               /* No error */
  DB_EOPEN = -100,              /* Cannot open database file */
  DB_EUPDATE,                   /* Cannot insert or update record */
  DB_ENOTFOUND,                 /* Requested record not found */
  DB_EBADREC,                   /* Extracted record is corrupted */
  DB_EPASSWORD                  /* Passwords differ */
};

extern char *acct_fname;
extern char *acct_passwords;

extern void db_set_filenames (char *dir, char *pwd, char *acct);
extern enum db_err db_init_files (void);

/* Password database handling functions */
extern enum db_err db_pwd_change (char *id, char *enc_pwd);
extern enum db_err db_pwd_compare (char *id, char *pwd);
extern enum db_err db_pwd_exists (char *id);
extern enum db_err db_pwd_delete (char *id);

/* Account database handling functions */
extern enum db_err db_acct_read (char *id, ACCOUNT *acct);
extern enum db_err db_acct_write (ACCOUNT *acct);
extern enum db_err db_acct_delete (char *id);
extern enum db_err db_acct_getfirst (ACCOUNT *acct);
extern enum db_err db_acct_getnext (ACCOUNT *acct);
extern void db_acct_getdone (void);

#endif /* #ifndef _telechatDB_H */
