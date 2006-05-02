#!/usr/bin/perl -w

# test that the password extraction works with big files
# (e.g. bigger than BUFSIZE)

use Test::More;
use strict;
use File::Spec;

BEGIN
  {
  chdir 't' if -d 't';
  }

my $filesize = 1024 * 1024 * 0.002;		# 0.5 megabytes
my $length = 2;					# AA..ZZ

chdir 'test-worker/linux/';
print "# created sample image file\n";
open FILE, '>test.img' or die ("Cannot write test.img: $!\n");
my $doc = ''; my $a = 'aa';
my @expected = ();
for (0 .. $filesize)
  {
  $doc .= "$a\n"; 
  push @expected, a2h($a) unless length($a) > $length;
  $a++;
  last if length($doc) > $filesize;
  }
my $expected_last = a2h('zz');
print FILE $doc;
close FILE;
  
print "# done. Planning ", scalar @expected * 2 + 2, " tests\n";
plan tests => scalar @expected * 2 + 2;

my $rc = `./pwdtest target/extract.txt`;

$rc =~ s/\nAt '(.+)' len ([0-9]+)/my $v = shift @expected; $v = 'no more output' if !defined $v; print "# $1 (len $2) $v\n"; is($1,$v,"key $v");  is ($2,length($v) >> 1, "len $v"); '';/eg;

$rc =~ /Last tested password in hex was '(.*)'/;
is ("Last pwd '$1'","Last pwd '$expected_last'", 'last pwd');
$rc =~ /Stopcode is '(.*)'/;
is ("Stop code '$1'","Stop code '0'", 'stop code');

while (scalar @expected > 0)
  {
  is ('nothing',shift @expected, 'nothing');
  }

END
  {
  print "# Cleaning up.\n";
  unlink "test.img";
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

