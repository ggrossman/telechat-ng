# Perl module to access lists of active users on telechat servers.
# Copyright (c) Const Kaplinsky <const@ce.cctpu.edu.ru>, 2000.
#
# $Id: Telechat.pm,v 1.8 2000/05/10 12:56:38 const Exp $

package Telechat;

use strict;
use Carp;
use Fcntl ':flock';
use Net::Telnet;

sub new {
  my $classname = shift;
  my $self = {};
  bless ($self, $classname);
  $self->_init(@_);
  return $self;
}

sub _init {
  my $self = shift;
  my %param = @_;

  # Assign default values to uninitialized and incorrect keys
  unless (exists $param{Host}) {
    $param{Host} = 'localhost';
  } elsif ($param{Host} =~ /^([\w\d][\w\d\.]+[\w\d])$/) {
    $param{Host} = $1;          # Untaint
  } else {
    croak('Bad host name supplied for Telechat object constructor');
  }
  unless (exists $param{Port}) {
    $param{Port} = 7227;
  } elsif ($param{Port} =~ /^(\d+)$/) {
    $param{Port} = $1;          # Untaint
  } else {
    croak('Bad port number supplied for Telechat object constructor');
  }
  unless (exists $param{Prompt}) {
    $param{Prompt} = '/(Name: )|(member name: +)$/';
  }
  unless (exists $param{Timeout} &&
          $param{Timeout} > 0 && $param{Timeout} <= 60) {
    $param{Timeout} = 3;
  }
  unless (exists $param{Cache} &&
          $param{Cache} >= 0 && $param{Cache} <= 3600) {
    $param{Cache} = 60;
  }
  unless (exists $param{CacheDir}) {
    # Assuming that caller will never pass tainted CacheDir value
    $param{CacheDir} = 'cached-data';
  }

  my $cache_file = "$param{CacheDir}/$param{Host}.$param{Port}";
  my @lines = ();

  # Read cached data if available and not expired
  if ($param{Cache} && -f $cache_file) {
    if (open CACHE, $cache_file) {
      flock CACHE, LOCK_SH;
      my $time = <CACHE>;
      chomp $time;
      if (time - $time <= $param{Cache}) {
        @lines = <CACHE>;
      }
      flock CACHE, LOCK_UN;
      close CACHE;
    }
  }

  my $new_data = 0;

  # Get data from the server
  unless (@lines) {
    my %tparam = (Errmode => 'return',
                  Host    => $param{Host},
                  Port    => $param{Port},
                  Prompt  => $param{Prompt});

    my $t = Net::Telnet->new(%tparam);
    if ($t && $t->waitfor($tparam{Prompt}) && (@lines = $t->cmd('who'))) {
      $t->cmd('quit');
    } else {
      undef $t;
    }
    $new_data = 1;
  }

  # Save data to cache file
  if ($param{Cache} && $new_data) {
    if (open CACHE, "> $cache_file") {
      flock CACHE, LOCK_EX;
      my $time = time;
      print CACHE "$time\n";
      foreach my $line (@lines) {
        print CACHE $line;
      }
      unless (@lines) {
        print CACHE "\n";       # Caching of negative result
      }
      flock CACHE, LOCK_UN;
      close CACHE;
    }
  }

  # Parse results
  if (@lines && !(@lines == 1 && $lines[0] eq "\n")) {
    $self->parse_list(\@lines);
  }
}

#
# Parse list of online users and fill in the object data
#

sub parse_list {
  my $self = shift;
  my $lines = shift;            # Array reference

  unless ($lines->[-1] =~ /\n$/) {
    $#$lines--;
  }
  my $channel_names = [];
  my $channels = {};
  my $users = {};
  foreach (@$lines) {
    chomp;
    if (/^Users online:\s*$/ ||
        /^Nobody currently online\.\s*$/) {
      next;
    }
    elsif (/^All other rooms are invisible or unused:\s*$/ ||
           /^All other channels are unnamed or unused\s*$/) {
      push @$channel_names, '';
    }
    elsif (/^\s?(\S.*?)\s*$/ &&
           (!@$channel_names || $channel_names->[-1])) {
      push @$channel_names, $1;
    }
    elsif (@$channel_names && s/^\s+([a-z][a-z\d]+)//) {
      my $name = $channel_names->[-1];
      my $id = $1;
      my $idle = undef;
      if (s/\((typing)\)//          ||
          s/\((idle[hms:\s\d]+)\)// ||
          s/\((typing, idle[hms:\s\d]+)\)//) {
        $idle = $1;
      }
      my $email = undef;
      if (s/\[(.+)\][^\]]*$//) {
        $email = $1;
      }
      my $alias = '';
      if (/^\/?\s*(.*?)\s*$/) {
        $alias = $1;
      }
      if (defined $id && defined $email) {
        $channels->{$name}{$id} = { Alias => $alias };
        $users->{$id} = { Channel => $name,
                          Alias   => $alias };
        if ($email ne '*PRIVATE*' && $email ne '<not set>') {
          $channels->{$name}{$id}{Email} = $email;
          $users->{$id}{Email} = $email;
        }
        if (defined $idle) {
          if ($idle =~ s/^typing,?//) {
            $channels->{$name}{$id}{Typing} = 1;
            $users->{$id}{Typing} = 1;
          }
          if ($idle =~ /^\s*idle:?\s+(.*)$/) {
            $channels->{$name}{$id}{Idle} = $1;
            $users->{$id}{Idle} = $1;
          }
        }
      }
    }
  }

  # Remove unnamed channels with no users on them
  if (@$channel_names &&
      $channel_names->[-1] eq '' &&
      !keys %{$channels->{''}}) {
    pop @$channel_names;
  }

  # Initialize our object data
  $self->{ChannelNames} = $channel_names;
  $self->{Channels} = $channels;
  $self->{Users} = $users;
}

#
# Get list of available channels
#

sub channels {
  my $self = shift;

  unless (exists $self->{ChannelNames}) {
    return ();
  }
  return @{ $self->{ChannelNames} };
}

#
# Get list of all online users
#

sub users {
  my $self = shift;

  unless (exists $self->{Users}) {
    return ();
  }
  return keys %{ $self->{Users} };
}

#
# Get list of users on channel
#

sub users_on_channel {
  my $self = shift;
  my $channel = shift;

  unless (exists $self->{Channels} &&
          exists $self->{Channels}{$channel}) {
    return ();
  }
  return keys %{ $self->{Channels}{$channel} };
}

#
# Get information about particular user
#

sub user_info {
  my $self = shift;
  my $user = shift;

  unless (exists $self->{Users}) {
    return '';
  }
  if (exists $self->{Users}{$user}) {
    return $self->{Users}{$user};
  } else {
    return 0;
  }
}

1;
