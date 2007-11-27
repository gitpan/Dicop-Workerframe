#!/usr/bin/perl -w

# Test that the test_enc worker output is ok

use strict;

BEGIN
  {
  chdir 't' if -d 't';
  }

chdir 'test-worker/';
my $rc = `./test_enc 30 31 30 2 120`;

if ($rc !~ /### start ###\n((\n|.)+)\n### end ###/)
  {
  die "Critical failure, can't find test output!\n";
  }

my $output = $1;
$output =~ s/^# //g;		# remove prefix
$output =~ s/\n# /\n/g;		# remove prefix

print $output, "\n";

1;
