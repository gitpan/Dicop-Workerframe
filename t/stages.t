#!/usr/bin/perl -w

# test that the password dictionay generator generates the right sequences of
# passwords and permutations even when using a fixed prefix
# this also does two stages, forward and backward (reversed)

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

# stages: forward, reverse

use Test::More;

BEGIN
  {
  chdir 't' if -d 't';
  plan tests => 100;
  }

chdir 'test-worker/';
my (@args);
while (<DATA>)
  {
  s/[\n\r]//g;		# remove newlines
  next if /^#/;
  next if length($_) == 0;
  @args = split /,/, $_;
  @a = splice @args,0,7;

  print "# Testing $a[0] $a[1] $a[2] $a[3] $a[4]\n";
  $rc = `./pwdtest $a[0] $a[1] $a[2] $a[3] $a[4]`;

  # print "result: $rc\n"; sleep(5);
  $rc =~ s/\nAt '(.+)'/$v = shift @args; $v = 'no more output' if !defined $v; print "# $1 $v\n"; is($1,$v, "key $v"); '';/eg;

  $rc =~ /Last tested password in hex was '(.*)'/;
  is ("Last pwd '$1'","Last pwd '$a[5]'", 'last pwd');

  $rc =~ /Stopcode is '(.*)'/;
  is ("Stop code '$1'","Stop code '$a[6]'", 'stop code');

  while (scalar @args > 0)
    {
    is ('nothing',shift @args, 'nothing');
    }
  }

1; # EOF

# format is:
# start,end,target,set,timeout,expected_lastpwd,expected_stopcode,expected_pwds
__DATA__
# full run through, each word as lower
776f726c64,72696d,0000,target/ps42.set,0,50726566697831326d6972,0,5072656669783132776f726c64,5072656669783132646c726f77,50726566697831326469736b,50726566697831326b736964,5072656669783132656c657068616e74,5072656669783132746e616870656c65,5072656669783132747572746c65,5072656669783132656c74727574,50726566697831327370616365,50726566697831326563617073,5072656669783132687562,5072656669783132627568,507265666978313272696d,50726566697831326d6972
# run through until a certain word, each word as lower
776f726c64,747572746c65,0000,target/ps42.set,0,5072656669783132656c74727574,0,5072656669783132776f726c64,5072656669783132646c726f77,50726566697831326469736b,50726566697831326b736964,5072656669783132656c657068616e74,5072656669783132746e616870656c65,5072656669783132747572746c65,5072656669783132656c74727574
# start not at the first word, until end, each word as lower
656c657068616e74,72696d,0000,target/ps42.set,0,50726566697831326d6972,0,5072656669783132747572746c65,5072656669783132656c74727574,50726566697831327370616365,50726566697831326563617073,5072656669783132687562,5072656669783132627568,507265666978313272696d,50726566697831326d6972
# start not at the first word, not til end, each word as lower
747572746c65,687562,0000,target/ps42.set,0,5072656669783132627568,0,50726566697831327370616365,50726566697831326563617073,5072656669783132687562,5072656669783132627568
# start not at the first word, not til end, each word as lower and UPPER
747572746c65,687562,0000,target/ps3.set,0,5072656669783132425548,0,50726566697831327370616365,50726566697831325350414345,50726566697831326563617073,50726566697831324543415053,5072656669783132687562,5072656669783132485542,5072656669783132627568,5072656669783132425548
# start not at the first word, not til end, each word as UPPER
747572746c65,687562,0000,target/ps2.set,0,5072656669783132425548,0,50726566697831325350414345,50726566697831324543415053,5072656669783132485542,5072656669783132425548
# start not at the first word, not til end, each word as 10 possible mutations
747572746c65,687562,0000,target/ps1023.set,0,5072656669783132427548,0,50726566697831327370616365,50726566697831325350414345,50726566697831327350414345,50726566697831325370616365,50726566697831325350414365,50726566697831327370616345,50726566697831327350614365,50726566697831325370416345,50726566697831327370416345,50726566697831325350614365,50726566697831326563617073,50726566697831324543415053,50726566697831326543415053,50726566697831324563617073,50726566697831324543415073,50726566697831326563617053,50726566697831326543615073,50726566697831324563417053,50726566697831324563417073,50726566697831326543615053,5072656669783132687562,5072656669783132485542,5072656669783132685542,5072656669783132487562,5072656669783132485562,5072656669783132687542,5072656669783132685562,5072656669783132487542,5072656669783132685562,5072656669783132487542,5072656669783132627568,5072656669783132425548,5072656669783132625548,5072656669783132427568,5072656669783132425568,5072656669783132627548,5072656669783132625568,5072656669783132427548,5072656669783132625568,5072656669783132427548
