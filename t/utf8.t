#!/usr/bin/perl -w

# Test that the password extraction with UTF-8 works

use Test::More;
use strict;
use File::Spec;

BEGIN
  {
  chdir 't' if -d 't';
  plan tests => 3;
  }
chdir 'test-worker/';

my $rc = `./pwdtest target/utf8.cfg`;

$rc =~ /Last tested password in hex was '(.*)'/;
is ("Last pwd '$1'","Last pwd 'e8aa9e4765'", 'last pwd');
$rc =~ /Stopcode is '(.*)'/;
is ("Stop code '$1'","Stop code '0'", 'stop code');

like ($rc, qr/Did 207 passwords in/, 'did 207 passwords');

1; # EOF

#############################################################################

# convert "ab" to "6566"
sub a2h
  {
  my ($a) = shift;
  return '' if !defined $a || $a eq '';         # not defined or empty?
  unpack ( "H" . (length($a) << 1), $a);
  }

