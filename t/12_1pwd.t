#!/usr/bin/perl -w

# Test that the test_enc worker runs through only a single password
# Test that the pwdtest worker not only runs through only a single password,
# but also correctly finds that password.

use strict;
use Test::More;
plan tests => 16;

BEGIN
  {
  chdir 't' if -d 't';
  }

#############################################################################
# single pwd

chdir 'test-worker/';
my $rc = `./test_enc 30 30 31 2 120`;

if ($rc !~ /### start ###\n((\n|.)+)\n### end ###/)
  {
  die "Critical failure, can't find test output!\n";
  }

like ($rc, qr/Did 1 passwords in /, 'did only one password');

#############################################################################
# single correct pwd

$rc = `./pwdtest 313233343536 313233343536 313233343536 2 20`;

like ($rc, qr/Did 1 passwords in /, 'did only one password');
like ($rc, qr/Stopcode is '1'/, 'found result');
like ($rc, qr/Last tested password \(in ISO-8859-1/, 'correct encoding');

#############################################################################
# single correct pwd with single-byte encoding

$rc = `./test_cnv 313233343536 313233343536 313233343536 2 20`;

like ($rc, qr/Did 1 passwords in /, 'did only one password');
like ($rc, qr/Stopcode is '1'/, 'found result');
# no need to do UTF-8:
like ($rc, qr/Last tested password \(in ISO-8859-1/, 'correct encoding');

#############################################################################
# single correct pwd with UTF-8 encoding

$rc = `./test_cnv 3132333435fc 3132333435fc 3132333435c3bc 14 20`;

like ($rc, qr/Did 1 passwords in /, 'did only one password');
like ($rc, qr/Stopcode is '1'/, 'found result');
# no need to do UTF-8:
like ($rc, qr/Last tested password \(in ISO-8859-1/, 'correct encoding');

#############################################################################
# two pwds with UTF-8 encoding

$rc = `./test_cnv 3132333435fc 3132333435fd 3132333435c3ba 14 20`;

like ($rc, qr/Did 4 passwords in /, 'did four passwords (2 iso, 2 utf-8)');
like ($rc, qr/Stopcode is '0'/, 'found no result');
like ($rc, qr/Last tested password \(in UTF-8/, 'correct encoding');

#############################################################################
# three pwds with UTF-8 encoding (result is found for pwd in ISO-8859-1)

$rc = `./test_cnv 3132333435fc 3132333435fd 3132333435c3bd 14 20`;

like ($rc, qr/Did 3 passwords in /, 'did three passwords (2 iso, 1 utf-8)');
like ($rc, qr/Stopcode is '1'/, 'found result');
like ($rc, qr/Last tested password \(in ISO-8859-1/, 'correct encoding');


1;
