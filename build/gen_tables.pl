#!/usr/bin/perl -w

# Generate all the compressed tables for the character set conversion.

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
# unknown to Perl:
#  CP585
#  CP863

for (1250 .. 1258, qw/437 737 775 850 852 855 857 860 861 862 865 866 869 874/)
  {
  $cp .= "cp$_ ";
  }
for (1 .. 11, 13 .. 16)
  {
  $cp .= "iso-8859-$_ ";
  }
print "#" x 78, "\n# Generating all charset tables:\n\n";
print `perl gen_cp.pl $cp`, "\n";
print "#" x 78, "\n# Generating UTF-8 tables:\n\n";
print `perl gen_utf8.pl`;
