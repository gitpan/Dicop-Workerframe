#!/usr/bin/perl -w

# Test that the password extraction of UTF-8 and the conversion to UTF-16
# works correctly (segfault until revision 555)

# Also tests the "use_encodings" config entry

use Test::More;
use strict;
use File::Spec;

BEGIN
  {
  chdir 't' if -d 't';
  plan tests => 3;
  }
chdir 'test-worker/';

my $rc = `./pwdtest target/utf16.cfg`;

$rc =~ /Last tested password in hex was '(.*)'/;
is ("Last pwd '$1'","Last pwd '9e8a47006500'", 'last pwd');
$rc =~ /Stopcode is '(.*)'/;
is ("Stop code '$1'","Stop code '0'", 'stop code');

#print $rc,"\n";

like ($rc, qr/Did 207 passwords in/, 'did 207 passwords');

1; # EOF

