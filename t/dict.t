#!/usr/bin/perl -w

# test that the password dictionay generator generates the right sequences of
# passwords and permutations

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

use Test::More;
use strict;

BEGIN
  {
  chdir 't' if -d 't';
  plan tests => 77;
  }

chdir 'test-worker/';
my (@args);
while (<DATA>)
  {
  chomp();
  next if /^#/; next if length($_) == 0;

  my @args = split /,/, $_;
  my @a = splice @args,0,7;

  print "# Testing: $a[0] $a[1] $a[2] $a[3] $a[4]\n";

  my $rc = `./pwdtest $a[0] $a[1] $a[2] $a[3] $a[4]`;

#   print "result: $rc\n"; sleep(5);
  $rc =~ s/\nAt '(.+)'/my $v = shift @args; $v = 'no more output' if !defined $v; print "# $1 $v\n"; is($1,$v, '$v'); '';/eg;
  
  $rc =~ /Last tested password in hex was '(.*)'/;
  is ("Last pwd '$1'","Last pwd '$a[5]'", 'last pwd');

  $rc =~ /Stopcode is '(.*)'/;
  is ("Stop code '$1'","Stop code '$a[6]'", 'stopcode');

  while (scalar @args > 0)
    {
    is ('nothing',shift @args);
    }
  }

# test timeout (1 second)
my $rc = `./pwdtest 61726d73 776f726c64 0000 target/42h.set 1`;
#print "$rc\n"; sleep(5);
$rc =~ /Stopcode is '(.*)'/;
print "$rc\n" unless
  is ("Stop code '$1'","Stop code '2'", 'stop code 2');

1; # EOF

# format is:
# start,end,target,set,timeout,expected_lastpwd,expected_stopcode,expected_pwds
__DATA__
# (with digits, these should produce no upper/lower combinations)
# start at the first word, til end, each word as 10 possible mutations
3030,61626364,0000,target/1023a.set,0,61424344,0,3030,31323334,61626364,41424344,61424344,41626364,41424364,61626344,61426344,41624364,41626364,61424344,
# (with digits, these should produce no upper/lower combinations and only some reverse)
# start at the first word, NOT til end, each word as 10 possible mutations
3030,31323334,0000,target/1023b.set,0,34333231,0,3030,31323334,34333231,
# full run through, each word as lower
776f726c64,72696d,0000,target/42.set,0,72696d,0,776f726c64,6469736b,656c657068616e74,747572746c65,7370616365,687562,72696d
# run through until a certain word, each word as lower
776f726c64,747572746c65,0000,target/42.set,0,747572746c65,0,776f726c64,6469736b,656c657068616e74,747572746c65
# start NOT at the first word, until end, each word as lower
656c657068616e74,72696d,0000,target/42.set,0,72696d,0,747572746c65,7370616365,687562,72696d
# start NOT at the first word, not til end, each word as lower
747572746c65,687562,0000,target/42.set,0,687562,0,7370616365,687562
# start NOT at the first word, not til end, each word as lower and UPPER
747572746c65,687562,0000,target/3.set,0,485542,0,7370616365,5350414345,687562,485542
# start NOT at the first word, not til end, each word as UPPER
747572746c65,687562,0000,target/2.set,0,485542,0,5350414345,485542
# start NOT at the first word, not til end, each word as 10 possible mutations
747572746c65,687562,0000,target/1023.set,0,487542,0,7370616365,5350414345,7350414345,5370616365,5350414365,7370616345,7350614365,5370416345,7370416345,5350614365,687562,485542,685542,487562,485562,687542,685562,487542,685562,487542,
