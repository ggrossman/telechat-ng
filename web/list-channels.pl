#!/usr/bin/perl -w
#
# Example Perl script to list active users on telechat server.
# Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
#
# $Id: list-channels.pl,v 1.5 2000/04/30 18:32:23 const Exp $

use strict;

use Telechat;

my $host = shift || 'localhost';
my $port = shift || 7227;

my $t =  Telechat->new (Host    => $host,
                        Port    => $port,
                        Timeout => 20,
                        Cache   => 0);
for my $ch ($t->channels()) {
  print "Channel: \"$ch\"\n";
  for my $user ($t->users_on_channel($ch)) {
    print "   User: $user\n";
    my $info = $t->user_info($user);
    for my $key (sort keys %$info) {
      print "      $key: \"$info->{$key}\"\n";
    }
  }
  print "\n";
}

