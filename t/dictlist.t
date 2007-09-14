#!/usr/bin/perl -w

# test that the password dictionay generator generates the right sequences of
# passwords and permutations, even when adding "normal" generated strings from
# a SIMPLE/GROUPED set

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

use Test;
use strict;
use File::Spec;

BEGIN
  {
  chdir 't' if -d 't';
  plan tests => 11750;
  }

my $updir = File::Spec->updir();	# ".."

unlink 'out';

chdir 'test-worker/';
my (@args, $rc);
while (<DATA>)
  {
  next if /^#/;
  next if /^\s*$/;
  s/[\n\r]//g;		# remove newlines
  my @a = split /,/, $_;
  if ($a[3] !~ /chunk/)
    {
    print "# $a[0] $a[1] $a[2] $a[3] $a[4]\n";
    $rc = `./pwdtest $a[0] $a[1] $a[2] $a[3] $a[4]`;
    }
  else
    {
    print "# $a[3] \n";
    $rc = `./pwdtest $a[3] 0`;
    }

#  print "$rc\n"; sleep (5);

  my $pwds = _read_file(File::Spec->catfile($updir,'output',$a[7]));
  $rc =~ s/\nAt '(.+)' len ([0-9]+)/my $v = shift @$pwds; $v = 'no more output' if !defined $v; print "# $1 (len $2) $v\n"; ok($1,$v); ok ($2,length($v) >> 1); '';/eg;

  $rc =~ /Last tested password in hex was '(.*)'/;
  ok ("Last pwd '$1'","Last pwd '$a[5]'");
  $rc =~ /Stopcode is '(.*)'/;
  ok ("Stop code '$1'","Stop code '$a[6]'");
  while (scalar @$pwds > 0)
    {
    ok ('nothing',shift @$pwds);
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
    $line =~ s/[\n\r]//g;		# remove newlines
    push @pwds, a2h($line);
    }
  close $FILE;
  \@pwds;
  }

# format is:
# start,end,target,set,timeout,expected_lastpwd,expected_stopcode,file_with_expected_pwds
__DATA__
# chunk text: partial run through, forward+reverse, lower/upper, prepend a-z
6469736b,687562,0000,target/chunk_1_0.txt,0,7a425548,0,list9.txt
# chunk text: partial run through, forward+reverse, lower/upper, prepend a-z
6469736b,687562,0000,target/chunk_1_1.txt,0,7a425548,0,list9.txt
# full run through, each word as lower
776f726c64,72696d,0000,target/42a.set,0,72696d,0,list1.txt
# full run through, each word as lower, append 1 digits
776f726c64,72696d,0000,target/42b.set,0,72696d39,0,list2.txt
# full run through, each word as lower, append 1..2 digits
776f726c64,72696d,0000,target/42c.set,0,52494d3939,0,list3.txt
# full run through, each word as lower/upper, append 1 chars
776f726c64,72696d,0000,target/42d.set,0,52494d7a,0,list4.txt
# full run through, forward+reverse, each word as lower/upper, append 1 chars
776f726c64,72696d,0000,target/42e.set,0,4d49527a,0,list5.txt
# full run through, forward+reverse, each word as lower/upper, append 0..9,a..z
776f726c64,72696d,0000,target/42f.set,0,4d49527a,0,list6.txt
# full run through, forward+reverse, each word as lower/upper, prepend a..z
776f726c64,72696d,0000,target/42g.set,0,7a4d4952,0,list7.txt
# partial run through, just forward
6469736b,687562,0000,target/42a.set,0,687562,0,list8.txt
# partial run through, forward+reverse, lower/upper, prepend a-z
6469736b,687562,0000,target/42g.set,0,7a425548,0,list9.txt
