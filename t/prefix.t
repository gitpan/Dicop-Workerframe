#!/usr/bin/perl -w

# test that the password dictionay generator generates the right sequences of
# passwords and permutations even when using a fixed prefix

# permuatations:

#LOWER                      => 1;   # lowercase
#UPPER                      => 2;   # UPPERCASE
#LOWER_FIRST                => 4;   # lOWERFIRST
#UPPER_FIRST                => 8;   # Upperfirst
#LOWER_LAST                 => 16;  # LOWERLASt
#UPPER_LAST                 => 32;  # upperlasT
#UPPER_ODD                  => 64;  # uPpErOdD
#UPPER_EVEN                 => 128; # UpPeReVEn
#UPPER_VOWELS               => 256; # UppErvOwEls
#UPPER_CONSONANTS           => 512; # uPPeRCoNSoNaNTS

# stages:
# forward, reverse

use Test::More;
use strict;

BEGIN
  {
  chdir 't' if -d 't';
  plan tests => 58;
  }

chdir 'test-worker/';
my (@args);
while (<DATA>)
  {
  chomp();
  next if /^#/;
  next if length($_) == 0;
  my @args = split /,/, $_;
  my @a = splice @args,0,7;

  print "# Testing $a[0] $a[1] $a[2] $a[3] $a[4]\n";
  my $rc = `./pwdtest $a[0] $a[1] $a[2] $a[3] $a[4]`;

  # print "result: $rc\n"; sleep(5);
  $rc =~ s/\nAt '(.+)'/my $v = shift @args; $v = 'no more output' if !defined $v; print "# $1 $v\n"; is($1,$v, "key $v"); '';/eg;

  $rc =~ /Last tested password in hex was '(.*)'/;
  is ("Last pwd '$1'","Last pwd '$a[5]'", 'last pwd');

  $rc =~ /Stopcode is '(.*)'/;
  is ("Stop code '$1'","Stop code '$a[6]'", 'stop code');

  while (scalar @args > 0)
    {
    is ('nothing',shift @args);
    }
  }

# test timeout
my $rc = `./pwdtest 61 4141414141 0000 5 2`;
$rc =~ /Stopcode is '(.*)'/;
is ("Stop code '$1'","Stop code '2'");

1; # EOF

# format is:
# start,end,target,set,timeout,expected_lastpwd,expected_stopcode,expected_pwds
__DATA__
# full run through, each word as lower
776f726c64,72696d,0000,target/p42.set,0,507265666978313272696d,0,5072656669783132776f726c64,50726566697831326469736b,5072656669783132656c657068616e74,5072656669783132747572746c65,50726566697831327370616365,5072656669783132687562,507265666978313272696d
# run through until a certain word, each word as lower
776f726c64,747572746c65,0000,target/p42.set,0,5072656669783132747572746c65,0,5072656669783132776f726c64,50726566697831326469736b,5072656669783132656c657068616e74,5072656669783132747572746c65
# start not at the first word, until end, each word as lower
656c657068616e74,72696d,0000,target/p42.set,0,507265666978313272696d,0,5072656669783132747572746c65,50726566697831327370616365,5072656669783132687562,507265666978313272696d
# start not at the first word, not til end, each word as lower
747572746c65,687562,0000,target/p42.set,0,5072656669783132687562,0,50726566697831327370616365,5072656669783132687562
# start not at the first word, not til end, each word as lower and UPPER
747572746c65,687562,0000,target/p3.set,0,5072656669783132485542,0,50726566697831327370616365,50726566697831325350414345,5072656669783132687562,5072656669783132485542
# start not at the first word, not til end, each word as UPPER
747572746c65,687562,0000,target/p2.set,0,5072656669783132485542,0,50726566697831325350414345,5072656669783132485542
# start not at the first word, not til end, each word as 10 possible mutations
747572746c65,687562,0000,target/p1023.set,0,5072656669783132487542,0,50726566697831327370616365,50726566697831325350414345,50726566697831327350414345,50726566697831325370616365,50726566697831325350414365,50726566697831327370616345,50726566697831327350614365,50726566697831325370416345,50726566697831327370416345,50726566697831325350614365,5072656669783132687562,5072656669783132485542,5072656669783132685542,5072656669783132487562,5072656669783132485562,5072656669783132687542,5072656669783132685562,5072656669783132487542,5072656669783132685562,5072656669783132487542,
