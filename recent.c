/*******************************************************************
 *
 * $Id: recent.c,v 1.8 2000/06/02 05:47:06 const Exp $
 * Maintaining the list of recent users.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/

#include "telechat.h"

#define MAX_RECENT 20

static int ring_empty = 1, ring_head = 0, ring_tail = 0;

typedef struct {
  char    id[MAXID];
  time_t  login_time;
  time_t  logout_time;
} RECENT_USER;

static RECENT_USER recent_users[MAX_RECENT];

void add_recent (char *id, time_t login_time)
{
  if (ring_tail == ring_head && !ring_empty)
    if (++ring_head == MAX_RECENT)
      ring_head = 0;

  memcpy (recent_users[ring_tail].id, id, MAXID-1);
  recent_users[ring_tail].id[MAXID-1] = '\0';

  recent_users[ring_tail].login_time = login_time;
  recent_users[ring_tail].logout_time = time (NULL);

  if (++ring_tail == MAX_RECENT)
    ring_tail = 0;

  ring_empty = 0;
}

void list_recent (void)
{
  int i;
  char saved_ctime[20];

  if (ring_empty) {
    writestr (cur_slot, "Nobody has been online yet\n");
  } else {
    i = ring_head;
    do {
      strncpy (saved_ctime, ctime (&recent_users[i].login_time), 19);
      saved_ctime[19] = '\0';
      sprintf (msg_buf, "%.19s - %.19s   %s\n",
               saved_ctime, ctime (&recent_users[i].logout_time),
               recent_users[i].id);
      writestr (cur_slot, msg_buf);
      if (++i == MAX_RECENT)
        i = 0;
    } while (i != ring_tail);
  }
}
