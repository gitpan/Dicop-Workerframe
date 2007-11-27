#!/usr/bin/perl -w

# Generate all the compressed tables for the character set conversion.

use Encode;

chdir 'build' if -d 'build';
my $cp = 'koi8-r koi8-u ' . join(" ", qw/
  MacRoman
  MacCentralEurRoman
  MacIcelandic
  MacCroatian
  MacRomanian
  MacRumanian
  MacCyrillic
  MacUkrainian
  MacGreek
  MacTurkish
  MacSami
  /) . ' ';
  
# Problems with tables
#  MacHebrew
#  MacThai
#  MacArabic
#  MacFarsi

for (1250 .. 1258, qw/437 737 775 850 852 855 857 858 860 861 862 863 865 866 869 874/)
  {
  # guard against unknown encodings (CP863 for instance)
  my $enc = Encode::find_encoding("cp$_");
  $cp .= "cp$_ " if defined $enc;
  }
for (1 .. 11, 13 .. 16)
  {
  $cp .= "iso-8859-$_ ";
  }
print "#" x 78, "\n# Generating all charset tables:\n\n";
print `perl gen_cp.pl $cp`, "\n";
print "#" x 78, "\n# Generating UTF-8 tables:\n\n";
print `perl gen_utf8.pl`;
