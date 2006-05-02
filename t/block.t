#!/usr/bin/perl -w

# test that the password generator generates the right sequences of passwords
# (but uses the pwdblock worker to run the tests to see if that works)

use Test::More;
use strict;
use vars qw/$worker/;

BEGIN
  {
  chdir 't' if -d 't';
  plan tests => 310;
  }

# This worker is outdated, but it should still work:

$worker = "./pwdblock";
require 'worker.inc';


