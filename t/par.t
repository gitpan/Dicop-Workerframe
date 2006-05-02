#!/usr/bin/perl -w

# test the new parallel interface (it should work just like the old one)

use Test::More;
use strict;
use vars qw/$worker/;

BEGIN
  {
  chdir 't' if -d 't';
  plan tests => 310;
  }

$worker = "./test_par";
require 'worker.inc';

