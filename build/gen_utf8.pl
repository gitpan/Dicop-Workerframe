#!/usr/bin/perl -w

# Generate a few tables for UTF-8 conversions.

# Usage: perl build/gen_utf8.pl

use Encode;
use strict;
use File::Spec;

_generate();

print "All done.\n";

#############################################################################

sub _generate
  {
  # generate an array with the amount of following bytes for each possible
  # start byte:
  my @followers = ();
  for (my $i = 0; $i < 128; $i++)
    {
    push @followers, 0;
    }
  for (my $i = 128; $i < 256; $i++)
    {
    if (0x80 == ($i & 0xC0))
      {
      push @followers, -1;
      }
    elsif (0xc0 == ($i & 0xe0))
      {
      push @followers, 1;
      }
    elsif (0xe0 == ($i & 0xf0))
      {
      push @followers, 2;
      }
    elsif (0xf0 == ($i & 0xf8))
      {
      push @followers, 3;
      }
    elsif (0xf8 == ($i & 0xfc))
      {
      push @followers, 4;
      }
    elsif (0xfc == ($i & 0xfe))
      {
      push @followers, 5;
      }
    }

  my $table = <<EOF
/** Map start byte to number of continuation bytes for UTF-8 */
unsigned long utf8_followers[256] = {
EOF
 . _format_table(\@followers) . "\n  };";

  my $tpl = _read_file('conv.tpl');

  # generate the case mapping tables
  my ($lc,$uc) = _parse_file('CaseFolding.txt');

  my $table_lc = <<EOF
/** How many entries are in the case folding map? */
#define CASE_FOLDINGS ##CF##

/** Map UPPERCASE to lowercase for UTF-8 */
unsigned long utf8_to_lower[CASE_FOLDINGS] = {
EOF
  . _format_table( $lc ) . "\n  };";

  my $table_uc = <<EOF

/** Map lowercase to UPPERCASE for UTF-8 */
unsigned long utf8_to_upper[CASE_FOLDINGS] = {
EOF
  . _format_table( $uc ) . "\n  };";

  $tpl =~ s/##UTF8##//g;
  $tpl =~ s/##START##(.|\n)+##STOP##/$table\n$table_lc\n$table_uc/;
  $tpl =~ s/##SAFENAME##/UTF8/g;
  $tpl =~ s/##NAME##/UTF-8/g;
  $tpl =~ s/##DATE##/ scalar localtime() /eg;
  $tpl =~ s/##CF##/ scalar @$uc;/eg;

  _write_file(File::Spec->catfile(
     File::Spec->updir(),'src','include','encoding', 'utf8.h' ), $tpl);
  }
  
#############################################################################

sub _format_table
  {
  my ($val) = @_;

  my $table = ''; my $line = '  ';
  for my $u (@$val)
    {
    $line .= "$u, ";
    if (length($line) > 76)
      {
      $table .= $line . "\n"; $line = "  ";
      }
    }
  $table .= $line;
  $table =~ s/,\s+\z//;
  $table;
  }

sub _read_file
  {
  my $file = shift;

  open my $FH, "$file" or die ("Cannot read $file: $!");
  local $/ = undef;
  my $doc = <$FH>;
  close $FH;
  $doc;
  }

sub _parse_file
  {
  my $file = shift;

  open my $FH, "$file" or die ("Cannot read $file: $!");

  my $max = 0;
  my (@uc, @lc);
  while (my $line = <$FH>)
    {
  
    # format:
    # <code>; <status>; <mapping>; # <name>

    # skip any non-matching line
    # we consider only cases C and S (simple mapping)
    next unless $line =~ /^([0-9a-fA-F]+);\s+(C|S);\s+([0-9a-fA-F]+);/;

    my $cp = $1; my $cp_2 = $3;
    $cp =~ s/^0+//;
    $cp_2 =~ s/^0+//;

    # generate the UTF-8 bytes from the given codepoints
    my @b = reverse unpack("U0C*", pack ("U", hex("0x$cp_2")));
    my $b1 = '0x';
    $max = scalar @b if scalar @b > $max;
    for my $b2 (@b) { $b1 .= sprintf("%02x",$b2); }

    @b = reverse unpack("U0C*", pack ("U", hex("0x$cp")));
    my $b3 = '0x';
    $max = scalar @b if scalar @b > $max;
    for my $b2 (@b) { $b3 .= sprintf("%02x",$b2); }

    # print STDERR "'$b3' <=> '$b1'\n";
    push @lc, $b3, $b1;
    push @uc, $b1, $b3;
    }
  close $FH;

  print "# Maximum number of bytes seen (should be <= 4!): $max\n";
  \@lc, \@uc;
  }

sub _write_file
  {
  my ($file,$doc) = @_;

  print ("Writing to $file\n");
  open my $FH, ">$file" or die ("Cannot write $file: $!");
  print $FH $doc;
  close $FH;
  }

