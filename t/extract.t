#!/usr/bin/perl -w

# test that the password extraction works

use Test::More;
use strict;
use File::Spec;

BEGIN
  {
  chdir 't' if -d 't';
  plan tests => 634;
  }

my $updir = File::Spec->updir();	# '..'

chdir 'test-worker/';
my (@args);
while (<DATA>)
  {
  chomp();
  next if /^#/;
  next if length($_) == 0;
  my @a = split /,/, $_;
  my $timeout = $a[3] || '';
  print "# $a[0] $timeout\n";
  my $rc = `./pwdtest target/$a[0] $timeout`;

  #print "'$rc'\n"; sleep(6);

  my $pwds = _read_file(File::Spec->catfile($updir,'output','image',$a[2] . '.lst'));
  my $expected_last = $pwds->[-1] || '';

  $rc =~ s/\nAt '(.+)' len ([0-9]+)/my $v = shift @$pwds; $v = 'no more output' if !defined $v; print "# $1 (len $2) $v\n"; is($1,$v,"key $v"); is ($2,length($v) >> 1, "len $v"); '';/eg;

  $rc =~ /Last tested password in hex was '(.*)'/;
  is ("Last pwd '$1'","Last pwd '$expected_last'", 'last pwd');

  $rc =~ /Stopcode is '(.*)'/;
  is ("Stop code '$1'","Stop code '$a[1]'", 'stop');

  while (scalar @$pwds > 0)
    {
    is ('nothing',shift @$pwds, 'nothing');
    }
  }

1; # EOF

#############################################################################

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

  my $hex = 0;

  my (@pwds,$FILE);
  open $FILE, $file or die("Cannot read file '$file': $!");
  while (my $line = <$FILE>)
    {
    chomp($line);
    $hex++, next if $line =~ /^=hex/;
    $line = a2h($line) if $hex == 0;
    push @pwds, $line;
    }
  close $FILE;
  \@pwds;
  }

# format is:
# chunk-targetfile, expected_stopcode, image_and_file_with_expected_pwds, [timeout]
__DATA__
# length N=4..7, extract UPPERCASE (with extra junk), and mutate through lowercase
chunk_0_18.txt,0,image018
# length N=6..6, extract odd/even UPPERCASE
chunk_0_17.txt,0,image017
# length N=6..6, extract odd/even break UPPERCASE
chunk_0_16.txt,0,image016
# length N=4..7, extract UPPERCASE (with extra junk)
chunk_0_15.txt,0,image015
# length N=3, extract letters, skipping 0x00
chunk_0_13.txt,0,image013
# length N=3, extract letters, break on 0x00
chunk_0_14.txt,0,image014
# length N=3..5, extract all bytes (even 0x00 and 0x0a)
chunk_0_12.txt,0,image012
# length N=9..10, extract only letters
chunk_0_11.txt,0,image011
# length N=3
chunk_0_0.txt,0,image000
# length N=3..4
chunk_0_1.txt,0,image001
# length N=3..5
chunk_0_2.txt,0,image002
# length N=3..6
chunk_0_3.txt,0,image003
# length N=3..7
chunk_0_4.txt,0,image004
# length N=4..6
chunk_0_5.txt,0,image005
# length N=4..7
chunk_0_6.txt,0,image006
# length N=4..7, but 'wrong' charset
chunk_0_7.txt,0,image007
# length N=4..7, only extract lowercase
chunk_0_8.txt,0,image008
# length N=4..7, extract UPPERCASE (with extra junk)
chunk_0_9.txt,0,image006
# length N=3..7, only extract numbers, find result
chunk_0_10.txt,1,image010
# timeout test: 
# length N=3..7, only extract numbers, find result
#chunk_0_10.txt,1,image010,2
