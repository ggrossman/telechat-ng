#!/usr/bin/perl -Tw
#
# Example CGI script to list online users on some telechat servers.
# Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
#
# $Id: list-online.cgi,v 1.5 2000/05/10 15:57:30 const Exp $

use strict;
use Telechat;
use CGI ':standard';
use HTML::Template;

#
# Configuration
#

my $chathost = 'localhost';     # Machine running chat server
my $chatport = 7227;            # TCP port number

# Don't place template in directory which is accessible via HTTP!
my $tmpl_name = "./list-online.tmpl";

my $cache_exp = 60;             # Cache of Telechat.pm expires in 60 seconds
my $cache_dir = 'cached-data';  # Directory to keep cache files

#
# Main part
#

print header;
my $template = HTML::Template->new (filename => $tmpl_name,
                                    die_on_bad_params => 0);
$template->param (CHANNELS => get_info ($chathost, $chatport));
print $template->output;

#
# Get all information into array suitable for HTML::Template
#

sub get_info {
  my ($host, $port) = @_;

  my $t = Telechat->new (Host     => $host,
                         Port     => $port,
                         Timeout  => 5,
                         Cache    => $cache_exp,
                         CacheDir => $cache_dir);
  my $info = [];
  my @channels = $t->channels();
  foreach my $ch (@channels) {
    push @$info, {NAME  => $ch, USERS => []};
    my @users = $t->users_on_channel($ch);
    foreach my $user (@users) {
      my $i = $t->user_info ($user);
      push @{$info->[-1]{USERS}}, {ID     => $user,
                                   ALIAS  => $i->{Alias}};
      if (exists $i->{Email}) {
        $info->[-1]{USERS}[-1]{EMAIL} = urlize($i->{Email});
      }
      my $str = '';
      if (exists $i->{Typing}) {
        $str .= 'typing';
      }
      if (exists $i->{Idle}) {
        $str .= ", " if $str;
        $str .= "idle: $i->{Idle}";
      }
      $info->[-1]{USERS}[-1]{STATUS} = $str if $str;
    }
  }
  if (@$info && !$info->[-1]{NAME}) {
    $info->[-1]{NAME} = 'Other channels';
  }
  return $info;
}

#
# Convert raw text into HTML
#

sub htmlize {
  my $str = shift;

  $str =~ s/\e\[\d+m//g;        # Remove ANSI color codes

  $str =~ s/&/&amp;/g;          # Protect HTML control characters
  $str =~ s/</&lt;/g;           #
  $str =~ s/>/&gt;/g;           #
  $str =~ s/\"/&quot;/g;        #
  return $str;
}

#
# Convert email addresses and Web references into HTML links
#

sub urlize {
  my $str = shift;

  $str =~ tr/&<>\"\0//d;
  if ($str =~ /^\s*([A-Za-z][\w.-]*\@[\w-]+?\.[\w.-]+)\s*$/) {
    $str = "<a href=\"mailto:$1\">$1</a>";
  } elsif ($str =~ /^\s*http:\/\/([\w-]+\.[\w\/~%\.-]+)\s*$/ ||
           $str =~ /^\s*([\w-]+\.[\w\/~%\.-]+)\s*$/) {
    $str = "<a href=\"http://$1\">$1</a>";
  } else {
    $str = htmlize ($str);
  }
  return $str;
}
