#############################################################################
#
# NAME
#    Makefile
#
# CONTENTS
#    This file contains the make(1) rules for the chat system
#
#############################################################################

# Force the `date' utility to use default C locale

LC_ALL = C
LANG = C

# Note: general compilation options such as -DVERIFY are now set in config.h

IFLAGS =	-I. -I/usr/local/include

# Production
#CFLAGS =	-O2 $(IFLAGS)

# Debug
CFLAGS =	-g $(IFLAGS)

PROG = 	telechat-ng

OBJS = 	main.o queue.o recent.o users.o utils.o active.o \
	io.o acct.o init.o pmail.o beeps.o admin.o signals.o \
	channels.o typing.o enter.o broadcast.o quit.o \
	emote.o db.o logging.o config.o colors.o validate.o \
	commands.o userinfo.o squelch.o linked.o

SRCS =	main.c queue.c recent.c users.c utils.c active.c io.c \
	acct.c init.c pmail.c beeps.c enter.c admin.c signals.c \
	channels.c typing.c broadcast.c quit.c \
	emote.c db.c logging.c config.c colors.c validate.c \
	commands.c userinfo.c squelch.c

LINUX_LDFLAGS   = -lm -lgdbm -lcrypt
FREEBSD_LDFLAGS = -lm -lgdbm -lcrypt -L/usr/local/lib

LDFLAGS = $(FREEBSD_LDFLAGS)

CC = gcc

default: $(PROG)

install: $(PROG) kill_idlers send_sighup level_passwd

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LDFLAGS)

kill_idlers: telechat.h config.h kill_idlers.o config.o
	$(CC) $(CFLAGS) -o kill_idlers kill_idlers.o config.o

send_sighup: telechat.h config.h send_sighup.o config.o
	$(CC) $(CFLAGS) -o send_sighup send_sighup.o config.o

level_passwd: telechat.h config.h db.h level_passwd.o config.o db.o
	$(CC) $(CFLAGS) -o level_passwd level_passwd.o config.o db.o $(LDFLAGS)

clean: 
	rm -f $(OBJS) *core* ./# ./*~ ./*.bak $(PROG) linked.c

clear: 
	rm -f $(OBJS) *core* ./# ./*~ ./*.bak $(PROG) linked.c \
	kill_idlers.o send_sighup.o level_passwd.o \
	kill_idlers send_sighup level_passwd

depend: $(SRCS)
	makedepend -Y -I. $(SRCS) 2> /dev/null

linked.c: $(SRCS)
	echo "char *LinkDate = \"`date`\";" > linked.c

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

# DO NOT DELETE THIS LINE -- make depend depends on it.

main.o: telechat.h config.h prototypes.h db.h
queue.o: telechat.h config.h prototypes.h db.h
recent.o: telechat.h config.h prototypes.h db.h
users.o: telechat.h config.h prototypes.h db.h
utils.o: telechat.h config.h prototypes.h db.h
active.o: telechat.h config.h prototypes.h db.h
io.o: telechat.h config.h prototypes.h db.h
acct.o: telechat.h config.h prototypes.h db.h
init.o: telechat.h config.h prototypes.h db.h
pmail.o: telechat.h config.h prototypes.h db.h
beeps.o: telechat.h config.h prototypes.h db.h
enter.o: telechat.h config.h prototypes.h db.h
admin.o: telechat.h config.h prototypes.h db.h
signals.o: telechat.h config.h prototypes.h db.h
channels.o: telechat.h config.h prototypes.h db.h
typing.o: telechat.h config.h prototypes.h db.h
broadcast.o: telechat.h config.h prototypes.h db.h
quit.o: telechat.h config.h prototypes.h db.h
emote.o: telechat.h config.h prototypes.h db.h
db.o: telechat.h config.h prototypes.h db.h
logging.o: telechat.h config.h prototypes.h db.h
config.o: telechat.h config.h prototypes.h db.h
colors.o: telechat.h config.h prototypes.h db.h
validate.o: telechat.h config.h prototypes.h db.h
commands.o: telechat.h config.h prototypes.h db.h
userinfo.o: telechat.h config.h prototypes.h db.h
squelch.o: telechat.h config.h prototypes.h db.h
