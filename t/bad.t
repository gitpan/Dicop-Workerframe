#!/usr/bin/perl -w

# test that the password generator refuses to work on bad inputs/files

use Test::More;
use strict;

BEGIN
  {
  chdir 't' if -d 't';
  plan tests => 40;
  }

chdir 'test-worker/';
my (@args);
while (<DATA>)
  {
  next if /^#/;
  next if /^\s*$/;
  chomp();
  my @a = split /,/, $_;
  print "# ", join (' ', @a), "\n";
  my $arguments = join(' ',@a);
  my $rc = `./pwdtest $arguments`;
#   print "$rc\n"; sleep(5);

  like ($rc, qr/(Couldn\'t initialize password generator|Error in dofunction)/, 'got some error');

  my $error = <DATA>; chomp($error);
  $error = quotemeta($error);
  like ($rc, qr/$error/, $error);
  }

1; # EOF

# format is:
# start,end,target,set,timeout,expected_stopcode
# expected_error_msg
__DATA__
613030,303030,0000,2,0
Error: Illegal character 61 at pos 0 in start pwd.
305130,303030,0000,2,0
Error: Illegal character 51 at pos 1 in start pwd.
303030,623030,0000,2,0
Error: Illegal character 62 at pos 0 in end pwd.
303030,306730,0000,2,0
Error: Illegal character 67 at pos 1 in end pwd.
303030,3030,0000,2,0
Error: End password shorter than start password.
303030,30303030,0000,target/42bad.set,0
Error: Invalid key 'password_prefix_faulty'
303030,30303030,0000,../target/42bad.set,0
Error: Invalid key 'password_prefix_faulty'
303030,30303030,0000,42bad.set,0
Error: Cannot find file '42bad.set'
chunk_0_300.txt,1,2
Error: Expected 1, 2, 4 or 5 arguments, but got 3.
target/bad_1_0.txt
Error: Couldn't read chunk description file 'target/bad_1_0.txt'
target/bad_0_0.txt
Error: Character set with id 500 not found.
target/bad_0_1.txt
Error: Malformed line at line 4
target/bad_0_2.txt
Error: Malformed line at line 4
target/bad_0_3.txt
Error: Malformed line at line 4
target/bad_0_4.txt
Error: Key 'start' re-defined at line 8.
target/bad_0_5.txt
Error: Malformed line at line 4
target/bad_0_6.txt
Extraction charset id 0 does not exist - aborting.
target/bad_0_7.txt
Cannot use charset 300 (a non-simple one) as extract set - aborting.
target/bad_0_8.txt
Cannot use charset 100 (a non-simple one) as extract set - aborting.
# partial run through dictionary, forward+reverse, lower/upper, prepend a-z
# but wrong offset
target/bad_0_9.txt
Error: Cannot find start word in dictionary.
