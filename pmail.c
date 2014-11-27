/*******************************************************************
 *
 * $Id: pmail.c,v 1.25 2000/06/09 06:49:21 const Exp $
 * Code to send private messages.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/

#include "telechat.h"

static void cmd_pmail2 (SLOT *sp, char *id);
static void cmd_pmail3 (char *msg);
static void cmd_pmail_offmsg2 (SLOT *sp, char *id);
static void cmd_pmail_offmsg3 (char *msg);
static int offmsg_send (char *id, char *msg);
static long offmsg_print (long offset);
static void cmd_offmsg_read2 (char *text);
static char *offmsg_alloc_path (char *id);

void cmd_pmail_reply (void)
{
  if (cur_slot->pmail_rcvd[0] == '\0') {
    writestr (cur_slot,
              "You have not received private messages in this session yet.\n");
    setproc (sendpub, MAXMSG, 0);
    return;
  }

  strcpy (cur_slot->pmail, cur_slot->pmail_rcvd);
  strcpy (cur_slot->pmail_sent, cur_slot->pmail);

  sprintf (msg_buf, "Sending private message back to %s.\n", cur_slot->pmail);
  writestr (cur_slot, msg_buf);

  writestr (cur_slot, "Msg: ");
  setproc (cmd_pmail3, MAXMSG, 0);
}

void cmd_pmail_again (void)
{
  if (cur_slot->pmail_sent[0] == '\0') {
    writestr (cur_slot,
              "You have not sent private messages in this session yet.\n");
    setproc (sendpub, MAXMSG, 0);
    return;
  }

  strcpy (cur_slot->pmail, cur_slot->pmail_sent);

  sprintf (msg_buf, "Sending private message to %s.\n", cur_slot->pmail);
  writestr (cur_slot, msg_buf);

  writestr (cur_slot, "Msg: ");
  setproc (cmd_pmail3, MAXMSG, 0);
}

void cmd_pmail (void)
{
  writestr (cur_slot, "To: ");
  setproc_id (cmd_pmail2, PROC_ID_ALL);
}

static void cmd_pmail2 (SLOT *sp, char *id)
{
  strcpy (cur_slot->pmail, (sp) ? sp->acct.id : id);
  writestr (cur_slot, "Msg: ");
  setproc (cmd_pmail3, MAXMSG, 0);
}

static void cmd_pmail3 (char *msg)
{
  SLOT *sp;

  if (msg[0]) {
    if ((sp = slotbyname (cur_slot->pmail)) == NULL) {
      sprintf (msg_buf,
               "Warning: %s is currently offline.", cur_slot->pmail);
      begin_attr (cur_slot, 1);
      writestr (cur_slot, msg_buf);
      reset_attr (cur_slot);
      writestr (cur_slot, "\nTrying to send offline message ... ");
      if (offmsg_send (cur_slot->pmail, msg))
        writestr (cur_slot, "done.\n"
                  "Addressee will read it on the next logon.\n");
      else
        writestr (cur_slot, "FAILED.\n");
    } else {
      if (!squelched (sp, cur_slot) && !reversed (cur_slot, sp))
        writemsg (sp, msg, 1);
      strcpy (cur_slot->pmail_sent, sp->acct.id);
      strcpy (sp->pmail_rcvd, cur_slot->acct.id);
      sprintf (msg_buf, "Message sent to %s.\n", cur_slot->pmail);
      writestr (cur_slot, msg_buf);
    }
  }
  cur_slot->pmail[0] = '\0';
  setproc (sendpub, MAXMSG, 0);
}

void cmd_pmail_offmsg (void)
{
  writestr (cur_slot, "To: ");
  setproc_id (cmd_pmail_offmsg2, PROC_ID_ALL);
}

static void cmd_pmail_offmsg2 (SLOT *sp, char *id)
{
  strcpy (cur_slot->offmsg_to, (sp) ? sp->acct.id : id);
  writestr (cur_slot, "Msg: ");
  setproc (cmd_pmail_offmsg3, MAXMSG, 0);
}

static void cmd_pmail_offmsg3 (char *msg)
{
  SLOT *sp;
  int success;

  if (msg[0]) {
    success = offmsg_send (cur_slot->offmsg_to, msg);
    if (success) {
      sp = slotbyname (cur_slot->offmsg_to);
      if (sp && !squelched (sp, cur_slot) && !reversed (cur_slot, sp)) {
        sprintf (msg_buf, " * New offline message from %s/%s (/m to read)",
                 cur_slot->acct.id, cur_slot->acct.handle);
        writeaction (sp, msg_buf);
      }
      writestr (cur_slot, "Offline message sent.\n");
    } else {
      begin_attr (cur_slot, 1);
      writestr (cur_slot, "Sending offline message failed.\n");
      reset_attr (cur_slot);
    }
  }
  setproc (sendpub, MAXMSG, 0);
}

/*******************************************************************
 *
 *  Send offline message (to be stored in a file).
 *    id: user name;
 *    msg: message to send.
 *
 *  Return values: 1 on success, 0 otherwise.
 *
 *  Notes:
 *    * This function overwrites msg_buf global variable.
 *    * This function does not use file locking properly.
 *
 *******************************************************************/

static int offmsg_send (char *id, char *msg)
{
  enum db_err err;
  char *fname;
  int fd;
  long pos;
  int hdr_bytes, msg_bytes;

  if (db_pwd_exists (id) != DB_SUCCESS)
    return 0;

  fname = offmsg_alloc_path (id);
  if (fname == NULL)
    return 0;

  fd = open (fname, O_WRONLY | O_CREAT, 0600);
  free (fname);
  if (fd < 0)
    return 0;

  msg_bytes = strlen (msg);
  pos = (long) lseek (fd, 0, SEEK_END);
  if (pos < 0 || pos + msg_bytes + 20 > max_msgfile * 1024) {
    close (fd);
    return 0;
  }

  hdr_bytes = sprintf (msg_buf, "%s %d %c %lu\n",
                       cur_slot->acct.id,
                       (int) cur_slot->acct.msgcolor,
                       (int) cur_slot->acct.nlchar,
                       (unsigned long) time (NULL));
  if (write (fd, msg_buf, hdr_bytes) != hdr_bytes ||
      write (fd, msg, msg_bytes) != msg_bytes ||
      write (fd, "\n", 1) != 1) {
    close (fd);
    return 0;
  }

  close (fd);
  return 1;
}

/*******************************************************************
 *
 *  Dump next offline message to the current slot.
 *    offset: position of the first byte of a message in the file.
 *
 *  Return value:
 *    offset of the next message on success;
 *     0 on normal eof;
 *    -1 on error if at least a part of message was displayed;
 *    -2 on error if message was not even seen.
 *
 *  Notes:
 *    * This function overwrites msg_buf global variable.
 *    * This function does not use file locking properly.
 *
 *******************************************************************/

static long offmsg_print (long offset)
{
  char *fname;
  FILE *fp;
  long new_offset = -1;
  int i, c;
  int lines = 0;
  int too_long = 0;

  char id[64];
  int color;
  char space, nlchar;
  unsigned long time_sent;

  /* Open file */
  fname = offmsg_alloc_path (cur_slot->acct.id);
  if (fname == NULL)
    return -2;
  fp = fopen (fname, "r");
  free (fname);
  if (fp == 0)
    return -2;

  /* Skip messages before the one we need */
  if (fseek (fp, offset, SEEK_SET) == -1) {
    fclose (fp);
    return -2;                  /* Error setting position in the file */
  }

  /* Look ahead for possible EOF */
  if ((c = getc (fp)) == EOF) {
    fclose (fp);
    return 0;
  } else
    ungetc (c, fp);

  /* Read header and print headline */
  if (fscanf (fp, "%63s %d%c%c %lu",
              id, &color, &space, &nlchar, &time_sent) != 5 ||
      space != ' ' || getc(fp) != '\n') {
    fclose (fp);
    return -2;                  /* Wrong format of message header */
  }
  begin_attr (cur_slot, 1);
  sprintf (msg_buf, "Message from %s, %.24s", id, ctime(&time_sent));
  writestr (cur_slot, msg_buf);

  /* Read a message itself, get current position and close file */
  i = 0;
  while (i < MAXMSG - 1) {
    if ((c = getc(fp)) == EOF || c == '\n')
      break;
    if (isprint(c))
      msg_buf[i++] = c;
  }
  msg_buf[i] = '\0';
  if (i == MAXMSG - 1 && getc(fp) != '\n')
    too_long = 1;

  /* Diagnostics */
  if (c == EOF) {
    writestr (cur_slot, " [unexpected EOF on read]:\n");
  } else if (i == 0) {
    writestr (cur_slot, " [empty].\n");
    new_offset = ftell (fp);
  } else if (too_long) {
    writestr (cur_slot, " [too long, truncated]:\n");
    while ((c = getc(fp)) != EOF && c != '\n');
    if (c != EOF)
      new_offset = ftell (fp);
  } else {
    writestr (cur_slot, ":\n");
    new_offset = ftell (fp);
  }
  reset_attr (cur_slot);

  /* Output if not empty */
  if (msg_buf[0])
    writemsg_raw (msg_buf, color, nlchar);

  fclose (fp);
  return (new_offset <= offset) ? -1 : new_offset;
}

void offmsg_read (void)
{
  long off = 0;
  int count = 0;

  do {
    off = offmsg_print (off);
    if (off > 0 || off == -1)
      count++;
  } while (off > 0);

  if ((off == 0 || off == -1) && count) {
    writestr (cur_slot, "-- end of offline messages --\n");
  } else {
    begin_attr (cur_slot, 1);
    writestr (cur_slot, "-- error reading offline messages --\n");
    reset_attr (cur_slot);
  }
}

int offmsg_check (void)
{
  char *fname;
  FILE *fp;

  fname = offmsg_alloc_path (cur_slot->acct.id);
  if (fname == NULL)
    return 0;

  fp = fopen (fname, "r");
  free (fname);
  if (fp == 0)
    return 0;
  if (fseek(fp, 0, SEEK_END) != -1 && ftell(fp) != 0) {
    fclose (fp);
    return 1;
  } else {
    fclose (fp);
    return 0;
  }
}

int offmsg_erase_all (void)
{
  char *fname;
  int ret = 1;

  fname = offmsg_alloc_path (cur_slot->acct.id);
  if (fname == NULL)
    return 0;

  if (unlink (fname) == -1) {
    writestr (cur_slot, "Error erasing offline messages.\n");
    ret = 0;
  }

  free (fname);
  return ret;
}

void cmd_offmsg_read (void)
{
  if (offmsg_check()) {
    offmsg_read();
    writestr (cur_slot, "Erase messages? [y/n] ");
    setproc (cmd_offmsg_read2, 2, COMPL_NONE);
  } else
    writestr (cur_slot, "You have no offline messages.\n");
}

static void cmd_offmsg_read2 (char *text)
{
  if (text[0] == 'y' || text[0] == 'Y')
    if (offmsg_erase_all())
      writestr (cur_slot, "Offline messages erased.\n");

  setproc (sendpub, MAXMSG, 0);
}

static char *offmsg_alloc_path (char *id)
{
  char *fname;

  fname = (char *) malloc (strlen(base_dir_prefix) + strlen(id) + 6);
  if (fname)
    sprintf (fname, "%s/%s.msg", base_dir_prefix, id);

  return fname;
}
