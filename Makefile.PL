use ExtUtils::MakeMaker;

#############################################################################
# write META.yml

sub MY::metafile {
  package MY;
  my $self = shift;
  if (!eval { require YAML; 1 })
  {
    warn ("For developers: YAML not installed, 'make dist' will not override META.yml");
    return $self->SUPER::metafile_target(@_);
  };

  use YAML::Node;
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

WriteMakefile(
  'NAME'	 => 'Dicop-Workerframe',
  'VERSION_FROM' => 'src/include/dicop.h',
);
