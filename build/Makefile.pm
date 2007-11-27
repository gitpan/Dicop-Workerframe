# common routines for Makefile.PL, so they can be re-used

# We need Perl 5.6.2 or up
use 5.006002;
use ExtUtils::MakeMaker;
use Config;
use strict;
use warnings;
use File::Spec;

our $arch = lc($^O);					# mswin32, linux etc
our $is_win = ($arch =~ /mswin32/);
our $updir = File::Spec->updir();
our $libprefix = 'lib'; $libprefix = '' if $is_win;

# for GCC, let it warn on:
# anything (-pedantic) 
# and unused function parameters (-Wunused-parameter):
# but do not warn for use of "long long" and things like "%llf"
our $warnings = '-Wunused-parameter -pedantic -Wno-long-long';
# for VC++, hide the excessive "padded with xxx bytes" messages
$warnings = '/wd4820' if $is_win;

our $optimize = _optimize();

# check arguments to Makefile.PL
our $flags = ''; our $debug = 0;
for my $arg (@ARGV)
  {
  my $f = uc($arg);
  $f = '-D' . $f unless $f =~ /^-D/;

  die ("Unknown flag $arg") unless $arg =~ /^-D(DEBUG)\z/;

  $flags .= " $f";
  $debug = 1 if $f eq '-DDEBUG';
  }
$flags .= ' -D_FILE_OFFSET_BITS=64' unless $is_win;
$flags .= ' -DWIN32 /nologo' if $is_win;

1;

no warnings 'redefine';

sub _optimize
  {
  # generate the compiler flags for optimizing
  
  return ' ' if $is_win;

  # For all other systems, use the flags Perl was build as a baseline, then
  # modify them to suit our needs:

  my $optimize = $Config{ccflags} . ' ' . $Config{optimize};
  # to match on entry
  my $entry = qr/[\w-]+(=[\w-]+)?/;

  # remove -DSOMETHING, -Wall, -I/usr
  $optimize =~ s/(^|\s)-[DIW]$entry//g;

  # remove "-g" to prevent a debuggin perl to generate debugging info
  # XXX TODO: this might fail on other non-gcc compilers where "-g" means something else
  $optimize =~ s/(^|\s)-g// unless $is_win;

  $optimize =~ s/-O([12]|\s|\z)/-O3/;	# if we optimize, we might as well do it fully

  # remove double entries
  my %entries;
  $optimize =~ s/(^|\s)(-$entry)/$entries{$2} = 1;/eg;

  delete $entries{"-fno-strict-aliasing"};

  $optimize = join (" ", sort keys %entries);
  }

sub MY::makeaperl
  {
  '';
  }

#############################################################################
# In the subroutines below, the generated text in rc has to have TAB instead
# of 8 spaces, or the generated Makefile will not accepted by make!

sub MY::test
  {
  my $rc = <<EOF

test :: pure_all
	PERL_DL_NONLAZY=1 \$(FULLPERLRUN) -MExtUtils::Command::MM -e "test_harness(0)" t/*.t

EOF
;

  return $rc unless $is_win;

  # For win32, we need to build the list of test files explicitely:

  opendir my $dir, 't' or die ("Cannot read directory t: $!"); 
  my @files = readdir $dir;
  closedir $dir;

  my @t_files;
  for my $file (sort @files)
    {
    my $t = File::Spec->catfile('t',$file);
    next unless $t =~ /\.t\z/;		# only accept .t files
    next unless -f $t;

    push @t_files, $t;
    }

  # finally generate the target text
  $rc = <<EOF

test :: pure_all
	\$(FULLPERLRUN) -MExtUtils::Command::MM -e "test_harness(0)" @t_files

EOF
;

  $rc;
  }

sub MY::const_cccmd
  {
  # generate the section with the command for the c-compiler:
  'CCCMD = $(CC) -c $(INC) $(CCFLAGS) $(OPTIMIZE)';
  }

sub MY::c_o
  {
  # We need to insert the "-o $*.o" to create "src/main/main.o" opposed to
  # main.o. This ensures that the .o files are actually found during linking
  # the library.
  package MY;
  my $self = shift;

  my $c_o = $self->SUPER::c_o();

  my $r = ' -o $*$(OBJ_EXT)'; $r = ' -Fo"$*$(OBJ_EXT)"' if $is_win;
  $c_o =~ s/(\$\(CCCMD\) \$\(.*\$\*\.c)\n/$1$r\n/;

  # we do not need this, so remove it to shorten command lines
  $c_o =~ s/"-I\$\(PERL_INC\)" //g;

  # these do not get defined anywhere, so we simple remove them
  $c_o =~ s/\$\(PASTHRU(_INC|_DEFINE)?\) //g;

  return $c_o; 
  }

