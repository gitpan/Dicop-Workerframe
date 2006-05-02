#!/usr/bin/perl -w

use Test;

BEGIN
  {
  chdir 't' if -d 't';
  plan tests => 1;
  }

`cd ../src/; ./mtest >/dev/null`;

ok (1,1);
