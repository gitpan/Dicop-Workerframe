#!/usr/bin/perl -w

use strict;
# qiuck and dirty generator for listX.txt

my @append = ('0' .. '99', 'a'..'z');
#my @append = ('a'..'z');

while (my $l = <>)
  {
  chomp($l);

  my $stages = [ $l ];
  push @$stages, scalar reverse("$l");
  for my $line (@$stages)
    {
    print lc($line)."\n";
    for (@append)
      {
      # print "$line$_\n";
      print "$_$line\n";
      }
    print uc($line)."\n";
    for (@append)
      {
      # print uc($line)."$_\n";
      print $_ . uc($line)."\n";
      }
    }

  }
