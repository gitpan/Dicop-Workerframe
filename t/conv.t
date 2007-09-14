#!/usr/bin/perl -w

# Test that the test_cnv worker finds a password when:
# * the input-range is in ISO-8859-1
# * the target is given in UTF-8

use Test::More;
use strict;

BEGIN
  {
  chdir 't' if -d 't';
  plan tests => 8;
  }

chdir 'test-worker/';
my (@args);
while (<DATA>)
  {
  s/[\n\r]//g;		# remove newlines
  next if /^#/;
  next if length($_) == 0;
  my @args = split /,/, $_;
  my @a = splice @args,0,7;

  print "# Testing $a[0] $a[1] $a[2] $a[3] $a[4]\n";
  my $rc = `./test_cnv $a[0] $a[1] $a[2] $a[3] $a[4]`;

  $rc =~ /Last tested password in hex was '(.*)'/;
  is ("Last pwd '$1'","Last pwd '$a[5]'", 'last pwd');

  $rc =~ /Stopcode is '(.*)'/;
  is ("Stop code '$1'","Stop code '$a[6]'", 'stop code');

  }

1; # EOF

# format is:
# start,end,target,set,timeout,expected_lastpwd,expected_stopcode
__DATA__
# some large range
307f30,30ff30,30c3bc30,14,0,30fc30,1
# just the single password to see if the "first pwd" also matches
30fc30,30fc30,30c3bc30,14,0,30fc30,1
# see if the "last pwd" also matches
30fc2f,30fc30,30c3bc30,14,0,30fc30,1
# this doesn't match
307f30,30ff30,30c3bc3030,14,0,30c3bf30,0
