#!/usr/bin/perl -w

# Test that the password dictionay generator generates the right sequences of
# passwords and permutations, even when adding "normal" generated strings from
# a SIMPLE/GROUPED set.

# This file tests that the dictionary options can be set in the chunk
# description file.

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
use File::Spec;

BEGIN
  {
  chdir 't' if -d 't';
  plan tests => 866 * 2;
  }

my $updir = File::Spec->updir(); $updir = File::Spec->catdir($updir,$updir);

unlink 'out';

chdir 'test-worker/linux/';
my (@args, $rc);
while (<DATA>)
  {
  next if /^#/;
  next if /^\s*$/;
  chomp();
  my @a = split /,/, $_;
  print "# $a[0]\n";
  $rc = `./pwdtest target/$a[0] 0`;

  #print "$rc\n"; sleep (5);

  my $pwds = _read_file(File::Spec->catfile($updir,'output',$a[3]));
  $rc =~ s/\nAt '(.+)' len ([0-9]+)/my $v = shift @$pwds; $v = 'no more output' if !defined $v; print "# $1 (len $2) $v\n"; is($1,$v, "key $v"); is ($2,length($v) >> 1, "len $2"); '';/eg;

  $rc =~ /Last tested password in hex was '(.*)'/;
  is ("Last pwd '$1'","Last pwd '$a[2]'", 'last pwd');

  $rc =~ /Stopcode is '(.*)'/;
  is ("Stop code '$1'","Stop code '$a[1]'", 'stop code');

  while (scalar @$pwds > 0)
    {
    is ('nothing',shift @$pwds, 'nothing');
    }
  }

1; # EOF

# convert "ab" to "6566"
sub a2h
  {
  my ($a) = shift;
  return '' if !defined $a || $a eq '';         # not defined or empty?
  unpack ( "H" . (length($a) << 1), $a);
  }

sub _read_file
  {
  my $file = shift;

  my (@pwds,$FILE);
  open $FILE, $file or die("Cannot read file '$file': $!");
  while (my $line = <$FILE>)
    {
    chomp($line);
    push @pwds, a2h($line);
    }
  close $FILE;
  \@pwds;
  }

# format is:
# start,end,target,set,timeout,expected_lastpwd,expected_stopcode,file_with_expected_pwds
__DATA__
# chunk text: partial run through, forward+reverse, lower/upper, prepend a-z
chunk_2_1.txt,0,7a425548,list9.txt
# chunk text: partial run through, forward+reverse, lower/upper, prepend a-z
chunk_2_0.txt,0,7a425548,list9.txt

