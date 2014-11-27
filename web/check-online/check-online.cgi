#!/usr/bin/perl -Tw
#
# Example CGI script to check if user is online on the telechat server.
# Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
#
# $Id: check-online.cgi,v 1.4 2000/04/20 16:51:47 const Exp $

use strict;
use Telechat;
use CGI ':standard';

my $image_exp = '+61';          # Returned image expires in 61 seconds
my $cache_exp = 60;             # Cache of Telechat.pm expires in 60 seconds
my $cache_dir = 'cached-data';  # Directory to keep cache files

my $user = param('who') || '';
my $host = param('host') || 'localhost';
my $port = param('port') || 7227;
my $theme = param('theme') || 'default';

# We must be very careful with user's data we don't trust
if ($theme =~ /^([\w\d_-]+)$/) {
  $theme = $1;
} else {
  $theme = 'default';
}

my $t = Telechat->new(Host     => $host,
                      Port     => $port,
                      Timeout  => 5,
                      Cache    => $cache_exp,
                      CacheDir => $cache_dir);

my $r = $user ? $t->user_info($user) : '';

my $state;
if (ref $r) {
  $state = 'online';
} else {
  $state = 'offline';
}

open IMG, "img/$theme/$state.gif"
  or die "Cannot open image: img/$theme/$state.gif";
binmode IMG;                    # Hi to Bill
print header (-TYPE => 'image/gif',
              -EXPIRES => $image_exp);
while (<IMG>) {
  print;
}
close IMG;

