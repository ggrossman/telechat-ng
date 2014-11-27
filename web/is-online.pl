#!/usr/bin/perl -w
#
# Example Perl script to check if user is online on the telechat server.
# Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
#
# $Id: is-online.pl,v 1.4 2000/04/30 18:30:10 const Exp $

use strict;
use Telechat;

my $user = shift || die "Usage: $0 <user> [host [port]]\n";
my $host = shift || 'localhost';
my $port = shift || 7227;

my $t = Telechat->new (Host    => $host,
                       Port    => $port,
                       Timeout => 20,
                       Cache   => 0);
my $r = $t->user_info ($user);

if (ref $r) {
  my $user_str = "$user/$r->{Alias}";
  if (defined $r->{Email}) {
    $user_str .= " [$r->{Email}]";
  }
  if (defined $r->{Typing}) {
    $user_str .= " (typing)";
  }
  if (defined $r->{Idle}) {
    $user_str .= " (idle: $r->{Idle})";
  }
  my $ch_str = $r->{Channel} ?
    "channel \"$r->{Channel}\"" : "unknown channel";
  print << "EOT";
$user_str
  is online at $host, port $port,
    $ch_str.
EOT
} elsif ($r ne '') {
  print "$user is not online at $host:$port.\n";
} else {
  print "Error connecting to chat server.\n";
}
