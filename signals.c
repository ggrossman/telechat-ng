/*******************************************************************
 *
 * $Id: signals.c,v 1.8 2000/05/29 15:20:43 const Exp $
 * Signal handling code.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/

#include "telechat.h"

static void sh_logout_idle (int signo);
static void sh_reopen_logs (int signo);

void set_signals (void)
{
  signal (SIGUSR1, sh_logout_idle);
  signal (SIGUSR2, SIG_IGN);
  signal (SIGPIPE, SIG_IGN);
  signal (SIGHUP, sh_reopen_logs);
}

static void sh_logout_idle (int signo)
{
  logout_idle_flag = 1;
  signal (signo, sh_logout_idle);
}

static void sh_reopen_logs (int signo)
{
  reopen_logs();
  signal (signo, sh_reopen_logs);
}
