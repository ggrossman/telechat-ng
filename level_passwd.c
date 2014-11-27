/*******************************************************************
 *
 * $Id: level_passwd.c,v 1.4 2000/06/11 08:08:46 const Exp $
 * Utility to change access level passwords.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 *
 *******************************************************************/

#include "telechat.h"
#include <termios.h>

char id[8] = "_levelX";
char pwd1[14];
char pwd2[14];

int main (int argc, char *argv[])
{
  if (argc != 2 || strlen(argv[1]) != 1 ||
      argv[1][0] < '2' || argv[1][0] > '5') {
    printf ("Usage: %s N\n"
            "  where N is one-digit user level (2..5)\n",
            argv[0]);
    return 0;
  }
  id[6] = argv[1][0];

  if (!read_conf()) {
    fprintf (stderr, "%s: Cannot read run-time configuration.\n", argv[0]);
    return 1;
  }
  db_set_filenames (base_dir_prefix, ACCT_PASSWORDS, ACCT_FILENAME);

  if (!check_auth()) {
    printf ("Sorry, you are not authorized to change the password.\n"
            "You must either know the old password"
            " or have root privilegies.\n");
  } else {
    if (ask_pwd ()) {
      if (db_pwd_change (id, pwd1) == DB_SUCCESS) {
        printf ("Level %c password has been changed successfully.\n",
                argv[1][0]);
        return 0;
      } else {
        printf ("Error changing password.\n");
      }
    }
  }

  printf ("Password has not been changed.\n");
  return 1;
}

int check_auth (void)
{
  if (!geteuid())
    return 1;
  if (db_pwd_exists(id) != DB_SUCCESS)
    return 1;

  printf ("Enter old password: ");
  if (!read_pwd (pwd1))
    return 0;
  switch (db_pwd_compare (id, pwd1)) {
    case DB_EPASSWORD:
      printf ("The password is incorrect.\n");
      return 0;
    case DB_SUCCESS:
      return 1;
    default:
      printf ("Cannot validate the password via database.\n");
      return 0;
  }
}

int ask_pwd (void)
{
  printf ("Enter new password: ");
  if (!read_pwd (pwd1))
    return 0;
  printf ("Confirm new password: ");
  if (!read_pwd (pwd2))
    return 0;
  if (strcmp (pwd1, pwd2)) {
    printf ("Sorry, passwords do not match.\n");
    return 0;
  }
  strncpy (pwd1, crypt (pwd2, "XX"), 13);
  pwd1[13] = '\0';
  memset (pwd2, '\0', 14);
  return 1;
}

int read_pwd (char *buf)
{
  struct termios term, noecho;
  int c;
  int i = 0;

  if (tcgetattr (0, &term)) {
    perror ("tcgetattr() error: ");
    exit (2);
  }
  noecho = term;
  noecho.c_lflag &= ~(ECHO | ECHOK | ICANON);
  if (tcsetattr (0, TCSANOW, &noecho)) {
    perror ("tcgetattr() error: ");
    exit (2);
  }
  for (;;) {
    c = getchar();
    if (c != '\n' && c != EOF) {
      if (i < 12)
        buf[i++] = (char)c;
    } else
      break;
  }
  buf[i] = '\0';
  printf ("\n");

  if (tcsetattr (0, TCSANOW, &term)) {
    perror ("tcgetattr() error: ");
    exit (2);
  }
  return (i != 0);
}
