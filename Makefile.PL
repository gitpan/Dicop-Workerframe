use ExtUtils::MakeMaker;

use strict;
use warnings;

# Creates the Makefile that is used during the build stage "make":

# User changable parts:

# For each worker, list all the nec. .c files inside "src/workername":
my $workers = { 
  test		=> [ 'test.c' ],
  test_par	=> [ 'test_par.c' ],
  pwdtest	=> [ 'pwdtest.c' ],
  extract	=> [ 'extract.c' ],
  };

# C files for libdicop, from src/main/:
my $c_files = [ qw/
  main
  dicop
  hexdump
  pwd_img
  pwd_par
  pwd_util
  pwdcfg
  pwddict
  pwdgen
  pwdread
  pwdsort
  / ];

#############################################################################
#############################################################################
#
# No changes neccessary below!
#

use File::Spec;

my $arch = lc($^O);					# mswin32, linux etc
my $is_win = ($arch =~ /mswin32/);
my $updir = File::Spec->updir();

# lib/libdicop$(LIB_EXT)
my $mylib = File::Spec->catfile( 'lib', 'libdicop$(LIB_EXT)');

my $src = File::Spec->catdir('src','main');
my $o_files = '';
for my $c_file (@$c_files)
  {
  # main => src/main/main.o
  $o_files .= File::Spec->catfile($src, $c_file . '$(OBJ_EXT)') . ' ';
  # modify entry in-place:
  # main => src/main/main.c
  $c_file = File::Spec->catfile($src, $c_file . '.c');
  }
my $o_files_lib = $o_files;

my $files =
	File::Spec->catfile($src, '*.o') . ' ' .
	File::Spec->catdir('t','test-worker','test.img') . ' ' .
	File::Spec->catdir('src','blib') . ' ';

for my $worker (sort keys %$workers)
  {
  # worker => src/worker/worker.o
  my $obj = File::Spec->catfile('src', $worker, $worker . '$(OBJ_EXT)') . ' ';
  $o_files .= $obj . ' ';
  my $exe = File::Spec->catfile('t', 'test-worker', $worker). ' ';
  # clean these up, too
  $files .= "$obj $exe ";
  for my $c_file (@{$workers->{$worker}})
    {
    # test.c => src/worker/test.c
    push @$c_files,  File::Spec->catfile('src', $worker, $c_file);
    }
  }

# Now generate the makefile, using subroutines defined below,
# and values generated above:

my $warnings = '-pedantic';
$warnings = '' if $is_win;

# XXX TODO: create compiler/version dependend flags here

my $optimize = '-O3 -march=i686 -fmessage-length=0 -pipe';
$optimize = ' ' if $is_win;

# check arguments to Makefile.PL
my $flags = ''; my $debug = 0;
for my $arg (@ARGV)
  {
  my $f = uc($arg);
  $f = '-D' . $f unless $f =~ /^-D/;

  die ("Unknown flag $arg") unless $arg =~ /^-D(DEBUG)\z/;

  $flags .= " $f";
  $debug = 1 if $f eq '-DDEBUG';
  }
$flags .= ' -D_FILE_OFFSET_BITS=64' unless $is_win;
$flags .= ' -DWIN32' if $is_win;

WriteMakefile(
  NAME		 => 'Dicop-Workerframe',
  'VERSION_FROM' => 'src/include/patch.h',

  OBJECT	 => $o_files,
  LIB		 => $mylib,
  C		 => $c_files,
  INC		 => "-I src/include",
  CCFLAGS	 => "-Wall $warnings$flags",

  # This package should not be installed, nor has it XS, so some sections
  # can be skipped to not clutter the Makefile:

  SKIP		 => [ 'install', 'dynamic', 'dynamic_lib', 'xs_c', 'xs_o',
		      'tool_xsubpp', 'manifypods', 'dynamic_lib',
		      'tool_autosplit', 'perldepend' ],

  LDFROM	 => $o_files_lib,
  LINKTYPE	 => 'static',

  OPTIMIZE	 => $optimize,

  # "make realclean" will remove these files:
  clean		 => { FILES => $files  },
);

# Do not print out this notice when we are run from the top makefile:
if (!$ENV{DICOP_SOURCE})
  {
  my $make = 'make'; $make = 'nmake' if $is_win;
 
  print "\nYour makefile has been successfully build. You should now run:\n\n";
  print "        $make\n";
  print "        $make test\n";

  print "\nAfterwards you can build the documentation in doc/html with _ONE_ of\n";
  print "the following commands:\n\n";
  print "        $make doc			# English documentation\n";
  print "        $make doc-de			# German documentation\n\n";

  };

1;

#############################################################################
# modify/customize different targets in the Makefile:

no warnings 'redefine';

sub MY::constants
  {
  # create the proper target name for the library file we build
  package MY;
  my $self = shift;

  $self->{INST_STATIC} = $mylib;

  return $self->SUPER::constants();
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

sub MY::subdirs
  {
  #############################################################################
  # Generate the sub_dirs section - this builds the various testworkers.

  my $rc = '';

  my $subdirs_rule = "subdirs :: \$(FIRST_MAKEFILE) ";

  for my $worker (sort keys %$workers)
    {
    my $outdir = File::Spec->catdir('src', $worker);
    my $out = File::Spec->catfile($outdir, 'Makefile.PL');
 
    my $W = uc($worker); 
    my $WO = '$(' . $W . '_O)'; 
    my $result = File::Spec->catfile('t', 'test-worker', $worker);
    my $object = File::Spec->catfile('src', $worker, $worker.'$(OBJ_EXT)');
    my $main = File::Spec->catfile('src', 'main', 'main'.'$(OBJ_EXT)');

    $subdirs_rule .= "$result ";

    my $LD = 'LD_RUN_PATH="$(LD_RUN_PATH)" $(LD) -dynamic $(' . $W . '_O) $(OTHERLDFLAGS) -o $@ $(INSTALLPRIVLIB) $(EXPORT_LIST)';
    $LD = '$(LD) /OUT:"$@$(EXE_EXT)" $(' . $W . '_O) /SUBSYSTEM:CONSOLE $(OTHERLDFLAGS) $(INSTALLPRIVLIB)' if $is_win;

    my $r = "$result: $WO \$(LIB)\n";
    $r = "$result: $WO\n" if $is_win;

    $rc .=  "${W}_O=$object $main\n\n" .
	    $r .					# result: ...
	   "\t" . '$(RM_F) $@$(EXE_EXT)' . "\n" .
	   "\t" . $LD . "\n" .
	   "\t" . '$(CHMOD) $(PERM_RWX) $@$(EXE_EXT)' . "\n\n";

    }

  $subdirs_rule . "\n\t\$(NOECHO) \$(NOOP)\n\n" . $rc;
  }

sub MY::static_lib
  {
  # build libdicop.a from the files in LDFROM opposed to OBJECT

  my $rc = '

$(INST_STATIC): $(LDFROM) $(MYEXTLIB) $(INST_ARCHAUTODIR)$(DIRFILESEP).exists
	$(RM_RF) $@
	$(FULL_AR) $(AR_STATIC_ARGS) $@ $(LDFROM) && $(RANLIB) $@
	$(CHMOD) $(PERM_RWX) $@
	$(NOECHO) $(ECHO) "$(EXTRALIBS)" > $(INST_ARCHAUTODIR)/extralibs.ld

$(INST_ARCHAUTODIR)/.exists ::
	$(NOECHO) $(MKPATH) $(INST_ARCHAUTODIR)
	$(NOECHO) $(EQUALIZE_TIMESTAMP) Makefile $(INST_ARCHAUTODIR)/.exists

	-$(NOECHO) $(CHMOD) $(PERM_RWX) $(INST_ARCHAUTODIR)
';

  # WIN32
  $rc = '

$(INST_STATIC): $(LDFROM) $(INST_ARCHAUTODIR)$(DIRFILESEP).exists
        $(RM_RF) $@
        lib /OUT:"$@" $(LDFROM)

$(INST_ARCHAUTODIR)/.exists ::
        $(NOECHO) $(MKPATH) $(INST_ARCHAUTODIR)
        $(NOECHO) $(EQUALIZE_TIMESTAMP) Makefile $(INST_ARCHAUTODIR)/.exists

        -$(NOECHO) $(CHMOD) $(PERM_RWX) $(INST_ARCHAUTODIR)
' if $is_win;

  $rc;
  }

sub MY::top_targets
  {
  my $rc = '
all :: static subdirs
	$(NOECHO) $(NOOP)

config ::
	$(NOECHO) $(NOOP)

doc ::  
	doxygen build/Doxyfile
	$(NOECHO) $(NOOP)

doc-de ::  
	doxygen build/Doxyfile-de
	$(NOECHO) $(NOOP)

pure_all :: static config pm_to_blib subdirs linkext
	$(NOECHO) $(NOOP)

';

  return $rc;
  }

sub MY::const_cccmd
  {
  # generate the section with the command for the c-compiler:
  'CCCMD = $(CC) -c $(PASTHRU_INC) $(INC) $(CCFLAGS) $(OPTIMIZE)';
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
  $c_o =~ s/"-I\$\(PERL_INC\)"//g;

  return $c_o; 
  }

#############################################################################
# Write META.yml. This routine will create a META.yml file during a
#  make metyfile
# and is mainly used to build a distribution package.

sub MY::metafile
  {
  package MY;
  my $self = shift;
  if (!eval { require YAML; 1 })
    {
    warn ("For developers: YAML not installed, 'make dist' will not override META.yml");
    return $self->SUPER::metafile_target(@_);
    };

  require YAML::Node;
  my $node = YAML::Node->new({});

  $node->{name}                 = $self->{DISTNAME};
  $node->{version}              = $self->{VERSION};
  $node->{version_from}         = $self->{VERSION_FROM};
  $node->{license}              = 'bsd';
  $node->{distribution_type}    = 'module';
  $node->{generated_by}         = "$self->{DISTNAME} version $self->{VERSION}";
  $node->{installdirs}          = 'none';
  $node->{requires}             = $self->{PREREQ_PM};

  my $dump = YAML::Dump( $node );

  $dump =~ s/^(.*)$/\t\$(NOECHO) \$(ECHO) "$1" >>META.yml/gm;
  $dump =~ s/>>META\.yml/>META.yml/;

  return "metafile:\n$dump";
  }

