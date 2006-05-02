#!/usr/bin/perl -w

# test that the password generator generates the right sequences of passwords

use Test::More;
use strict;
use vars qw/$worker/;

BEGIN
  {
  chdir 't' if -d 't';
  plan tests => 310;
  }

$worker = "./pwdtest";
require 'worker.inc';
