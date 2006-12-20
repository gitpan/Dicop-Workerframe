#!/usr/bin/perl -w

# Regenerate patch.h from a template

use strict;
use warnings;
use File::Spec;

# change to the correct directory
chdir 'workerframe' if -d 'workerframe';
chdir 'build' if -d 'build';

# read in the template file
my $patch_h = _read_file('patch.tpl');

# get the revision number from SVN (meaning we need to be in a local
# working copy!)
my $rc = `svn info`;
$rc =~ /Revision: (\d+)/;

my $rev = $1 || 'unknown';

# replace template placeholders:
$patch_h =~ s/##MODIFIED##/scalar localtime()/eg;
$patch_h =~ s/##REVISION##/$rev/g;

# write out the generated file
my $up = File::Spec->updir();
_write_file( File::Spec->catfile($up,'src','include', 'patch.h'), $patch_h );

#############################################################################

sub _read_file
  {
  my $file = shift;

  open my $FH, "$file" or die ("Cannot read $file: $!");
  local $/ = undef;
  my $doc = <$FH>;
  close $FH;
  $doc;
  }

sub _write_file
  {
  my ($file,$doc) = @_;

  open my $FH, ">$file" or die ("Cannot write $file: $!");
  print $FH $doc;
  close $FH;
  }

