#!/usr/bin/perl -w

# Generate a compressed table of the requested codepage(s) as C code, so that
# this table can be used to convert from/to Unicode to/from this codepage.

# Also generates case-folding (uppercase/lowercase) tables and a list
# of all supported encodings.

use Encode 2.23;
use strict;
use File::Spec;

if (@ARGV == 0)
  {
  print "Usage: perl gen_cp.pl ENCODING [ENCODING1 [ENCODING2 ... ]]\n\n";
  print "Example: perl gen_cp.pl cp437 cp1252 iso-8859-1\n\n";
  exit;
  }
 
require Test::More;
Test::More::plan (tests => scalar @ARGV);

_generate_encodings(@ARGV);

print ("All done.\n");

#############################################################################

sub _generate_encodings
  {
  my $encodings = [
  "ASCII",
  "ISO-8859-1 (Western European)",
  "ISO-8859-2 (Eastern European)",
  "ISO-8859-3 (Southern European)",
  "ISO-8859-4 (Baltic)",
  "ISO-8859-5 (Cyrillic)",
  "ISO-8859-6 (Arabic)",
  "ISO-8859-7 (Greek)",
  "ISO-8859-8 (Hebrew)",
  "ISO-8859-9 (Turkish)",
  "ISO-8859-10 (Nordic)",
  "ISO-8859-11 (Thai)",
  "ISO-8859-13 (Baltic Rim)",
  "ISO-8859-14 (Celtic)",
  "ISO-8859-15 (Western European)",
  "ISO-8859-16 (South-Eastern European)",
  "KOI8-R (Russian)",
  "KOI8-U (Ukrainian)",
  "CP437 (US)",
  "CP737 (Greek)",
  "CP775 (Baltic)",
  "CP850 (Western European)",
  "CP852 (Latin 2)",
  "CP855 (Cyrillic)",
  "CP857 (Turkish)",
  "CP858 (CP850 + Euro)",
  "CP860 (Portuguese)",
  "CP861 (Icelandic)",
  "CP862 (Hebrew)",
  "CP863 (French-Canadian)",
  "CP865 (Nordic)",
  "CP866 (Russian)",
  "CP869 (Greek)",
  "CP874 (Thai)",
  "CP1250 (Central European)",
  "CP1251 (Cyrillic)",
  "CP1252 (Western European)",
  "CP1253 (Greek)",
  "CP1254 (Turkish)",
  "CP1255 (Hebrew)",
  "CP1256 (Arabic)",
  "CP1257 (Baltic)",
  "CP1258 (Vietnam)",
  "MacRoman",
  "MacCentralEurRoman",
  "MacIcelandic",
  "MacCroatian",
  "MacRomanian",
  "MacRumanian",
  "MacCyrillic",
  "MacUkrainian",
  "MacGreek",
  "MacTurkish",
  "MacArabic",
  "MacFarsi",
  "MacSami",
  "MacHebrew",
  "UTF-8",
  "UTF-16",
  "UTF-16BE (big endian)",
  "UTF-16LE (little endian)",
  "UTF-32",
  "UTF-32BE (big endian)",
  "UTF-32LE (little endian)",
  "EUC-KISX0213",
  "Shift_JISX0213",
  "SHIFT_JIS",
  "HZ",
  "GBK",
  "GB18030",
  "CP950 (BIG5)",
  "BIG5-HKSCS (BIG5 Hongkong)",
  "invalid encoding",
  ];

  # XXX TODO - these are missing (unclear if we ever need them)

  # CP932 - MS extension to Shift_JIS
  # CP936 is an alias for "gbk"


  # Multi-Byte encodings with escape characters - ugh, hard to implement
  #"EUC-JP",
  #"EUC-CN",
  #"EUC-TW",
  #"EUC-KR",
  #"ISO-2022-JP",
  #"ISO-2022-JP-1",
  #"ISO-2022-JP-2",
  #"ISO-2022-JP-3",
  #"ISO-2022-CN",
  #"ISO-2022-CN-EXT",
  #"ISO-2022-KR",

  my @encs;
  for my $e (@$encodings)
    {
    push @encs, "NULL";
    }
  my $names_text = '';
  my $encoding_text = '';
  for my $cp (@_)
    {
    _generate($cp);
    my $cpname = lc($cp); $cpname =~ s/-/_/g;
    $names_text .= "#include \"encoding/$cpname.h\"\n";

    # CP850 => 850, ISO-8859-1 => 8859_1
    my $strip = lc($cp); $strip =~ s/-/_/g; $strip =~ s/^(CP|ISO-)//;
    my $idx = 0;
    for my $e (@$encodings)
      {
      my $cmp = lc($e); $cmp =~ s/-/_/g; $cmp =~ s/^(CP|ISO-)//;
      # CP111 (foo) => CP111
      $cmp =~ s/\s\(.*//;
      if ($cmp eq $strip)
        {
	print "# Replace $encs[$idx] ($idx) with $e\n";
        $encs[$idx] = $cmp; last;
        }
      $idx++;
      }
    }

  $names_text .= "\nconst char _encoding_names[INVALID_ENCODING + 1][40] = {\n";

  for my $e (@$encodings)
    {
    $names_text .= "  \"$e\",\n";
    }
  $names_text =~ s/,\n\z//;	# remove last ","
  $names_text .= "  };\n\n";

  $encoding_text .= <<EOF

/* define the known encodings as constants */
enum eEncodings {
EOF
;
  for my $e (@$encodings)
    {
    # iso_8859_1 => 'ISO_8859_1'
    my $entry = $e; $entry = uc($entry) unless $e =~ /^Mac/;
    $entry =~ s/\s\(.*//;
    $entry =~ s/[- ]/_/g;
    $entry .= ' = 0' if $entry eq 'ASCII';
    $encoding_text .= "  $entry,\n";
    }
  $encoding_text =~ s/,\n\z//;	# remove last ","
  $encoding_text .= "  };\n";

  $names_text .= _generate_mapping (<<EOF

/** Mapping Encoding to UTF-8 */
unsigned long* encoding_mappings[INVALID_ENCODING + 1] = {
EOF
 , 'enc', $encodings, \@encs);
  
  $names_text .= _generate_mapping (<<EOF

/** Mapping UTF-8 to Encoding */
unsigned long* encoding_tables[INVALID_ENCODING+1] = {
EOF
 , 'encoding', $encodings, \@encs);

  $names_text .= _generate_mapping (<<EOF

/** Mapping to Unicode codepoint */
unsigned long* codepoint_tables[INVALID_ENCODING+1] = {
EOF
 , 'cp', $encodings, \@encs);

  $names_text .= _generate_mapping (<<EOF

/** Mapping to UPPERCASE */
unsigned char* uppercase_tables[INVALID_ENCODING+1] = {
EOF
 , 'uc', $encodings, \@encs);

  $names_text .= _generate_mapping (<<EOF

/** Mapping to lowercase */
unsigned char* lowercase_tables[INVALID_ENCODING+1] = {
EOF
 , 'lc', $encodings, \@encs);

  my $tpl = _fix_template( _read_file('mappings.tpl') );
  $tpl =~ s/##MAPPINGS##/$names_text/;

  _write_file(File::Spec->catfile(
     File::Spec->updir(),'src','include','encoding', 'mappings.h' ), $tpl);
  
  ###########################################################################
  # encoding/encodings.h

  $tpl = _fix_template( _read_file('encodings.tpl') );

  $tpl =~ s/##ENCODINGS##/$encoding_text/;
  _write_file(File::Spec->catfile(
     File::Spec->updir(),'src','include','encoding', 'encodings.h' ), $tpl);
  }

sub _generate_mapping
  {
  my ($text,$prefix,$encodings, $encs) = @_;

  my $idx = 0;
  for my $e (@$encs)
    {
    # NULL => "  NULL,", "iso_8859_1" => "  enc_iso_8859_1,":
    if ($e ne 'NULL')
      {
      $text .= "  ${prefix}_$e,\n";
      }
    else
      {
      $text .= "  NULL,\t\t\t/* $encodings->[$idx] */\n";
      }
    $idx++;
    }
  #$text =~ s/,\n\z//;	# remove last ","
  $text .= "  };\n";

  $text;
  }

#############################################################################

sub _generate
  {
  my $cp = shift;

  # iso-8859-1 => ISO-8859-1, MacRoman => MacRoman
  my $name = $cp; $name = uc($name) unless $name =~ /^Mac/;
  my $safe_name = uc($name);
  $safe_name =~ s/[^0-9A-Z]/_/g;

  my $file = lc($cp);

  my (@code_points, $input, @bytes, @upper, @lower);
  my $max = 0;
  for (my $i = 0; $i <= 255; $i++)
    {
    my $in = '$input = pack("C", $i);';
    $in = eval $in;
    my $output = decode($cp,$in);
    die $@ if $@;					# wrong encoding?
    push @code_points, unpack("U0U*", $output);
    #print "cp => " , join(", ", unpack("U*",$output)),"\n";
    my @b = reverse unpack("U0C*", $output);
    # print "by => " , join(", ", reverse unpack("U0C*",$output)),"\n";
  
    my $b1 = '0x';
    # find maximum number of bytes this char is in UTF-8
    $max = scalar @b if scalar @b > $max;
    for my $b2 (@b)
      {
      $b1 .= sprintf("%02x",$b2);
      }
    # print "by => $b1\n\n";
    push @bytes, $b1;

    # UPPER/lower
    $in = decode($cp, pack("C", $i));
    push @upper, sprintf("0x%02x", ord(encode($cp,uc($in))));
  #  print "$in => ",encode('utf-8',uc($in)),"\n";
    push @lower, sprintf("0x%02x", ord(encode($cp,lc($in))));
  #  print "$in => ",encode('utf-8',lc($in)),"\n";
    }

  my $first = undef; my $cur = 0;
  my @result;
  my @org = @code_points;
  while (@code_points)
    {
    $first = shift @code_points; my $count = 1; my $last = $first;
    while (@code_points && (($code_points[0] - $last) == 1))
      {
      $count++; $last = shift @code_points;
      }
    push @result, $count, $first;
    }

  # rebuild the original code-point list and see if it matches
  my @test = @result;

  my @c;
  while (@test)
    {
    my $count = shift @test;
    $first = shift @test;
    for (0..$count-1) { push @c, $first + $_; } 
    }

  die ("Table doesn't match!") unless Test::More::is_deeply (\@c, \@org, 'match');

  my $table = '';

  my $count = scalar @result / 2;
  my $count2 = $count * 2 + 1;
  my $line = "[$count2] = {\n  $count,\n  ";
  while (@result)
    {
    $line .= shift @result;
    $line .= ', ' . sprintf( "0x%x, ", shift (@result));
    if (length($line) > 76)
      {
      $table .= $line . "\n"; $line = "  ";
      }
    }
  $table .= $line;
  $table =~ s/,\s+\z//;

  push @bytes, $max;
  my $table2 = _format_table(\@bytes);
  my $table_upper = _format_table(\@upper);
  my $table_lower = _format_table(\@lower);
  my $table_codepoints = _format_table(\@org);

  my $tpl = _read_file('conv.tpl');

  $tpl = _fix_template($tpl);

  $tpl =~ s/##TABLE##/$table/g;
  $tpl =~ s/##TABLE2##/$table2/g;
  $tpl =~ s/##TABLE_CP##/$table_codepoints/g;
  $tpl =~ s/##TABLE_UC##/$table_upper/g;
  $tpl =~ s/##TABLE_LC##/$table_lower/g;
  $tpl =~ s/##NAME##/$name/g;
  $tpl =~ s/##SAFENAME##/$safe_name/g;
  my $lc_safe_name = lc($safe_name);
  $tpl =~ s/##LCSAFENAME##/$lc_safe_name/g;
  
  $tpl =~ s/##VERSION##/Perl v$], Encode v$Encode::VERSION/g;

  _write_file(File::Spec->catfile(
     File::Spec->updir(),'src','include','encoding', $lc_safe_name.'.h' ), $tpl);
  }

sub _fix_template
  {
  # replace things like ##DATE##
  my $tpl = shift;

  # remove unused markers and code
  $tpl =~ s/##(START|STOP)##\n//g;
  $tpl =~ s/##UTF8##(.|\n)+##UTF8##\n//;
  $tpl =~ s/##DATE##/ scalar localtime() /eg;

  $tpl;
  }

#############################################################################

sub _format_table
  {
  my ($val) = @_;

  my $table = ''; my $line = '';
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

sub _write_file
  {
  my ($file,$doc) = @_;

  print ("Writing to $file\n");
  open my $FH, ">$file" or die ("Cannot write $file: $!");
  print $FH $doc;
  close $FH;
  }

