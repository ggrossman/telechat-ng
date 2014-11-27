/*******************************************************************
 *
 * $Id: validate.c,v 1.3 2000/05/29 15:20:44 const Exp $
 * Routines to check correctness of strings entered by users.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/

#include "telechat.h"

int email_is_valid (char *address)
{
  char temp_email[MAXEMAIL];
  char *s;
  int count, i;

  if (address && strlen(address) > 6 && strchr(address, '@') &&
      !strchr(address, ' ') && !strchr(address, ';')) {

    s = strchr(address, '@') + 1;
    count = strlen (s);
    for (i = 0; i <= count; i++)
      temp_email[i] = toupper(s[i]);
    temp_email[i] = '\0';

    if (strcmp (temp_email, "WHITEHOUSE.GOV"))
      return 1;
  }
  return 0;
}

/*** Use email_is_valid instead
static int email_is_bogus (char *email)
{
  return (strlen (email) < 6   ||
          strlen (email) > 128 ||
          !strchr (email, '@') ||
          !strchr (email, '.') ||
          strchr (email, ' ')  ||
          strchr (email, '\t') ||
          strchr (email, '|')  ||
          strchr (email, '/')  ||
          strchr (email, '\\')  ||
          strchr (email, ':')  ||
          strchr (email, '#')  ||
          strchr (email, ';'));
}
*/

int login_is_valid (char *id)
{
  int i;

  if (strlen(id) < 2 || !isalpha(id[0]) || !islower(id[0]))
    return VAL_INVALID;

  for (i = 1; id[i]; i++)
    if (!(isalpha(id[i]) && islower(id[i])) && !isdigit(id[i]))
      return VAL_INVALID;

  for (i=0; i<num_bad_logins; i++)
    if (!strcmp(id, bad_logins[i]))
      return VAL_RESERVED;

  return VAL_OK;
}

