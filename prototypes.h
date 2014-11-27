/*******************************************************************
 *
 * $Id: prototypes.h,v 1.26 2000/06/06 16:32:53 const Exp $
 * ANSI prototypes for external functions.
 *
 * Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
 *
 * Original "Telechat" is copyrighted by Hyperreal, 1996.
 * For more information consult the file LICENSE.
 *
 *******************************************************************/

/* config.c */
extern int read_conf (void);

/* squelch.c */
extern int squelched (SLOT *sp, SLOT *sq);
extern int reversed (SLOT *sp, SLOT *sq);
extern void cmd_squelch (void);
extern void cmd_rsquelch (void);

/* init.c */
extern void initchannels(void);
extern void initvars(void);
extern void initsock(void);
extern void initslots(void);

/* quit.c */
extern void cmd_logoff (void);

/* broadcast.c */
extern void cmd_broadcast (void);
extern void cmd_station (void);

/* enter.c */
extern void login (char *id);
extern int read_prelogin (void);
extern int read_postlogin (void);

/* typing.c */
extern void cmd_typing (void);
extern void cmd_room_typing (void);

/* channels.c */
extern void get_channel (void);
extern void name_channel (void);
extern void unname_ch (void);
extern void add_to_channel (SLOT *sl, int chan);
extern void remove_from_channel (SLOT *sl, int chan);
extern void write_channel (char *msg, int chan, int msg_type);
extern void list_channel (void);

/* admin.c */
extern void cmd_kickoff (void);
extern void inquire (void);
extern void do_shutdown (int signo);
extern void show_status (void);
extern void op_func (void);
extern void cmd_toggle_email (void);
extern void cmd_level (void);

/* acct.c */
extern void acc_viewuser (SLOT *sp, char *id);
extern void accounting (char *ch);
extern void change_email (void);

/* pmail.c */
extern void cmd_pmail_reply (void);
extern void cmd_pmail_again (void);
extern void cmd_pmail (void);
extern void cmd_pmail_offmsg (void);
extern void offmsg_read (void);
extern void cmd_offmsg_read (void);
extern int offmsg_check (void);
extern int offmsg_erase_all (void);

/* recent.c */
extern void add_recent (char *id, time_t login_time);
extern void list_recent (void);

/* userinfo.c */
extern void dump_userinfoshort (ACCOUNT *acct, int logged_in);
extern void dump_userinfo (SLOT *sp_about);

/* main.c */
extern SLOT *slotbyname (char *id);
extern SLOT *slotbynumber (char *text);
extern int alloctemp (SLOT *sp, size_t size);
extern void freetemp (SLOT *sp);
extern void inactivate_slot (SLOT *sp, int nosave);
extern void destroy_slot (SLOT *sp);
extern void ts_none (unsigned char c);

/* commands.c */
extern void exec_cmd (char ch);
extern void setproc_id (void (*dispatch)(SLOT *sp, char *id), int mode);
extern void proc_id (char *id);

/* logging.c */
extern void write_log (char *msg);
extern void write_log_plain (char *msg);
extern void write_log_emote (char *msg);
extern void write_log_bcast (char *msg);
extern void write_log_bcast_emote (char *msg);
extern void write_log_paabout (SLOT *sp, char *msg);
extern void open_logs (void);
extern void close_logs (void);
extern void reopen_logs (void);

/* queue.c */
extern void startqueue (SLOT *sp);
extern void clearqueue (SLOT *sp);
extern void qinsert (QUEUE *q, char ch);
extern int qempty (QUEUE *q);
extern void qflush (QUEUE *q);
extern void qdispose (QUEUE *q);
extern int qcreate (QUEUE *q, size_t sz);
extern int writequeue (SLOT *sp, QUEUE *q);
extern int qlength (QUEUE *q);

/* colors.c */
extern char *colorname (int color, int colorify);
extern void begin_color (SLOT *sp, int color);
extern void begin_attr (SLOT *sp, int attr);
extern void reset_attr (SLOT *sp);
extern void toggle_color (void);
extern void choose_msg_color (void);

/* signals.c */
extern void set_signals (void);

/* emote.c */
extern void cmd_emote (void);
extern void cmd_broadcast_emote (void);

/* validate.c */
extern int login_is_valid (char *id);

/* beeps.c */
extern void cmd_togglebeeps (void);
extern void cmd_beep (void);

/* active.c */
extern void active (void);
extern void display_users (int ch);
extern int shared_spec (SLOT *sp_to, SLOT *sp_about, char ch);
extern void change_fmt (void);
extern void see (void);

/* utils.c */
extern int xatoi (char *s);
extern void panic (char *msg);
extern void closestd (void);

/* users.c */
extern int read_user (char *id, ACCOUNT *acct);
extern int write_user (ACCOUNT *acct);
extern int delete_user (char *id);

/* io.c */
extern void operact (SLOT *sp_from, SLOT *sp_to, char *msg);
extern void paabout (SLOT *sp, char *msg);

extern void writech (SLOT *sp, char ch);
extern void writestr (SLOT *sp, char *str);
extern void writeint (SLOT *sp, int value);
extern void writetwodig (SLOT *sp, int value);
extern void writemsg (SLOT *sp, char *msg, int type);
extern void writemsg_raw (char *msg, int color, char nlchar);
extern void writetime (SLOT *sp, long t);

extern void select_stop (SLOT *sp);
extern void clear_stop (SLOT *sp);
extern void select_wrap (SLOT *sp);
extern void clear_wrap (SLOT *sp);

extern void setproc (void (*func)(char *text), int max_input, int flags);
extern void sendpub (char *msg);
extern void send_ts (char c, char d);
extern void process_input (SLOT *sp);
extern void writeaction (SLOT *sp, char *msg);
extern void transmit (SLOT *sp);

