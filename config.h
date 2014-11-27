/*******************************************************************
 *
 * $Id: config.h,v 1.15 2000/05/31 16:29:24 const Exp $
 * Compile-time configuration that is still not moved to config.c.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 * Copyright 1996 Hyperreal.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/

#define DEF_MSG_FMT "%u/%h\\[Private]\\_[Broadcast]_: %m"

/*** Change to "  #%S %U [%M] %t" ***/
#define DEF_ACT_FMT "  #%S %u/%h [%M] %t"

#define SYSTEM_NAME "UNNAMED SYSTEM"
#define DEF_PORT 7227

#define FIRST_FREE_CHAN	3
#define CHANNEL_01_NAME "Main room"
#define CHANNEL_02_NAME "Another room"

#define DISCONNECTED	"Disconnected!"
#define ENTER_LOGIN     "Enter your member name: "

/*******************************************************************
 * 
 *  File names:
 *    BASE_DIR - the directory for all of the resource files
 *    ACCT_FILENAME - the name with the account information in it
 *    LOG_FILENAME - a file with a complete log of the chat session
 *    PRELOGIN_FILENAME - a message to people before they log in
 *    POSTLOGIN_FILENAME - a message to people after they log in
 *
 *******************************************************************/

#define BASE_DIR            "/home/telechat"
#define ACCT_FILENAME       "acct.db"
#define ACCT_PASSWORDS      "pwd.db"
#define LOG_FILENAME        "log"
#define PRELOGIN_FILENAME   "prelogin.msg"
#define POSTLOGIN_FILENAME  "postlogin.msg"

/*******************************************************************
 *
 *  Access levels
 *
 *******************************************************************/

#define	MODERATOR 4
#define TOPLEVEL 5

