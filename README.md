telechat-ng
===========

In high school in the early 90's, I spent an inordinate amount of time
hanging out on BBS's (Bulletin Board Systems), which I'd dial up on
a 2400-baud modem and block up my family's phone line for hours on end.

One of the boards I spent a lot of time on was called Morrison Hotel (MoHo).
It was a multi-line BBS running off 10 phone lines, which was unusual
for the time, and ran some homebrew BBS software written by two Unix
wizards, Mike and Eric, who worked for a company whose name I've
forgotten and the business of which I never learned. One of the
components of the system was a chat line called Unix/CB.

When I was 15, an uncle passed on a discarded Sun 2 workstation to me.
I knew that Unix/CB was written as multiple processes coordinating via
System V IPC message queues. Another user on MoHo, Brett Vickers, a
few years older than me and now a CS professor at UC Irvine, had
written a game for the site called Mordor. His first version of Mordor
also used SysV IPC but the second version was a single process that
multiplexed all I/O via BSD select(). I decided I wanted to learn BSD
socket programming so I started duplicating Unix/CB using sockets and
a select() loop at the core, with every interaction handled using
state transitions via function pointers.

When I got to UC Berkeley, the CSUA (Computer Science Undergraduate
Association) ran a machine called soda.berkeley.edu. (The CS building
at Cal, which opened my junior year, is called Soda Hall.) As a
CS freshman, I immediately started collecting as many Unix accounts
as possible. When I learned that the CSUA handed out free (LIFETIME!)
Unix accounts on soda.berkeley.edu, I immediately joined the CSUA.
I started running my chat server on soda just to try it out.
Of course, on soda, anything you did was being watched by every other
CS major on the box. Several other students saw the open port
(I think I used 5492, the last four digits of my parents' phone
number) and telnet-ed to it.

One of them was named Brian Behlendorf, nickname Vitamin B. He
expressed interest in the chat server and asked if he could have the
code. I gladly gave it to him.  He was a leader in the San Francisco
rave scene centered at the 1015 Folsom club and set up a "virtual
rave" server using my chat server.  "Vrave" would often be set up at
raves as a desktop computer and you could communicate with ravers at
OTHER raves. They added features to the server like the string "BOOM
BOOM BOOM BOOM" being printed occasionally to give a techno feel.
Brian and his fellow developers also undoubtedly fixed numerous bugs
and actually got the software to be stable production code, which
it probably wasn't when I gave it to him.

Even though we were both at Cal at the same time, I never met Brian in
person or even traded more than a few e-mails with him. He, of course,
went on to be one of the founders of the Apache Server and a leading
figure in the open source movement.

His future wife, Laura La Gassa, contacted me at one point and offered
$100 to license the Unix/CB software. I replied that I didn't want any
money but that they were free to use the code for any purpose as long
as my name remained on the copyright. I also wrote that credit should
be given to Skynet since I had basically written an imitation of that
original software.  Laura agreed, and they kept that promise and
inserted a note about Skynet, although probably most people had no
idea who this "Gary Grossman" person was.

Vrave ran for 6 years at hyperreal.org. There is still a page up
about it here: http://hyperreal.org/raves/vrave/

I wasn't paying much attention to Vrave except in the first few months
that it was up. I've never even been to a rave. But I was floored that
marriages were conducted over it. And unfortunately, it sounds like
there was some serious misbehavior by some bad actors that led to
its demise.

I forget what year this happened, but Eric Pederson, the real original
author of the first Unix/CB software, actually logged into Vrave at
some point and was floored that the UI was more or less identical to
something he had written 10+ years earlier. Somehow we reconnected
briefly and he set up an instance of the original Unix/CB and we
both logged into it and chatted briefly.

At some point, I wondered whether my code had ever made it into any
open source project. I found this telechat-ng project on the Web a few
years ago.  Since then, the site that hosted it has ceased to exist,
but I still had a copy of the tarball lying around my hard disk. I
figured I'd check it in to Github for posterity. This code has been
through many hands since I touched it... but a lot of it is still
recognizably the crap I wrote in high school.

-Gary Grossman, 11/26/14

Some history of MoHo from http://bbslist.textfiles.com/714/oldschool.html:

"Morrison Hotel (MoHo) was started in 1986 as a way to learn Unix and
meet girls. It was a 10-line multi-user BBS offering Chat, Message
Boards, Email, Games, File Transfers, Voting, and much more.  "The
sysops were Mike Cantu and myself, both 19 year old (in 1986) college
students going to California State University Long Beach.

"Mike and I spent a number of years as interns administering a IBM
mainframe system used by students in our high school district. On that
system we wrote some of the predecessors of MoHo features (in
Fortran!). We also spent a lot of time online during high school on
different BBSes around the country and in Orange County.

"The software for the MoHo was developed from scratch by us in C,
running Unix System V/3 on a Convergent Technologies miniframe (first)
and then SCO Unix on a 386 tower. We had ten 2400 baud US Robotics
Sportsters.

"The users of the board were primarily college kids and teenagers. It
was a prime place to meet the opposite sex via Chat :) The users got
together for parties all the time.

"Because 10 phone lines weren't cheap (neither was the $10,000
Convergent Miniframe), we charged a $10/month flat rate subscription
(we also had a "credits" plan where you were charged for online
usage). Access to the message boards and email were free, as was
limited access to chat. We strongly believed that the basic features
of our bulletin board should always be free.

"The chat system we developed was heavily influenced by
Diversidial. Lots of cool and unique features (like Jive mode, if you
remember the old Unix jive filter). A clone of the MoHo chat system
(same UI, different code base) was used by Vrave based at
hyperreal.org

"The message boards we developed were very heavily influenced by
PicoSpan (the original software that the Well ran). Featuring threaded
topics, etc.

"The email system and the menuing system we developed were heavily
influenced by Forum-PC (probably the best DOS PC BBS software there
was, followed by WWIV)

"Among the many games (there were lots of freeware games for Unix) was
a game we wrote called TAC - Tactical Armored Command. This was a
real-time, multiuser tank battle game. People spent hours and hours
playing that game!

"File transfers used gz, a freeware Unix file transfer program that
supported x/y/zmodem.

"We did not advertise because the phone lines were always busy (tho we
had a 30 minute timeout after which you were hung up on to let someone
else get a chance to get in). We had numerous phone numbers, so that
MoHo would be local dialing to the widest area.

"A company called US CAD hired us, and adopted Morrison Hotel to try
to turn it into a profitable business (both from subscriptions and
from developing the software to sell to other sysops). The business
never panned out though, for a number of reasons. We were eventually
laid off from US CAD in 1989 and Morrison Hotel was shut down. We did
not have the motivation to start it back up again.

"That is the story of Morrison Hotel." - Eric Pederson
(https://www.linkedin.com/in/ericacm)
