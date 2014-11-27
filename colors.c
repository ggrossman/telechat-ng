/*******************************************************************
 *
 * $Id: colors.c,v 1.6 2000/05/29 15:20:43 const Exp $
 * Choosing user's color preferences.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/

#include "telechat.h"

static void choose_msg_color2 (char *number);

static char *plain_names[] = {
  "default", "red", "green", "yellow",
  "blue", "magenta", "cyan", "unknown"
};
static char *colorful_names[] = {
  "default", "\e[31mred\e[0m", "\e[32mgreen\e[0m", "\e[33myellow\e[0m",
  "\e[34mblue\e[0m", "\e[35mmagenta\e[0m", "\e[36mcyan\e[0m", "unknown"
};

char *colorname (int color, int colorify)
{
  return (colorify) ?
    colorful_names[color & 0x07] : plain_names[color & 0x07];
}

void begin_color (SLOT *sp, int color)
{
  char color_code[] = "\e[30m";

  if (sp && sp->acct.usecolor && color) {
    color_code[3] = '0' | (color & 0x07);
    writestr (sp, color_code);
  }
}

void begin_attr (SLOT *sp, int attr)
{
  char attr_code[] = "\e[0m";

  if (sp && sp->acct.usecolor && attr) {
    attr_code[2] = '0' | (attr & 0x0F);
    writestr (sp, attr_code);
  }
}

void reset_attr (SLOT *sp)
{
  if (sp && sp->acct.usecolor)
    writestr (sp, "\e[0m");
}

void toggle_color (void)
{
  cur_slot->acct.usecolor = !cur_slot->acct.usecolor;
  if (cur_slot->acct.usecolor)
    writestr (cur_slot, "ANSI colors are now on\n");
  else
    writestr (cur_slot, "ANSI colors are now off\n");
}

void choose_msg_color (void)
{
  if (cur_slot->acct.usecolor) {
    writestr (cur_slot, "<0> default, <1> \e[31mred\e[0m,"
                        " <2> \e[32mgreen\e[0m, <3> \e[33myellow\e[0m,"
                        " <4> \e[34mblue\e[0m, <5> \e[35mmagenta\e[0m,"
                        " <6> \e[36mcyan\e[0m: ");
  } else {
    writestr (cur_slot, "<0> default, <1> red, <2> green, <3> yellow,"
                        " <4> blue, <5> magenta, <6> cyan: ");
  }
  setproc (choose_msg_color2, 2, COMPL_NONE);
}

static void choose_msg_color2 (char *number)
{
  if (number[0] >= '0' && number[0] <= '6') {
    cur_slot->acct.msgcolor = number[0] & 0x07;
    writestr (cur_slot, "Color has been changed\n");
  }
  setproc (sendpub, MAXMSG, 0);
}
