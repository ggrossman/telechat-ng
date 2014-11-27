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
rave" server using my chat server.  "VRave" would often be set up at
raves as a desktop computer and you could communicate with ravers at
OTHER raves. They added features to the server like the string "BOOM
BOOM BOOM BOOM" being printed occasionally to give a techno feel.
Brian and his fellow developers also undoubtedly fixed numerous bugs
and actually got the software to be stable production code, which
it probably wasn't when I gave it to him.

Even though we were both at Cal at the same time, I never met Brian in
person or even traded more than few e-mails with him. He, of course,
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

I forget what year this happened, but Eric Pedersen, the real original
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