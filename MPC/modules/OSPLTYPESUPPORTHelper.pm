package OSPLTYPESUPPORTHelper;

use strict;
use CommandHelper;
use File::Basename;
our @ISA = qw(CommandHelper);

##print "OSPLHelper Loaded\n";

sub get_output {
  my($self, $file, $flags) = @_;

  my $dir = '';
  $flags = '' unless defined $flags;
  if ($flags =~ /-o +(\S+)/) {
    $dir = "$1/";
  }

  ## Not interested in Stand Alone 
  ##
  my @out = ();
  if ($flags =~ /-S/) {
    return \@out;
  }

  ## For CORBA DDS we need to include the *Dcps.idl get_output file
  ##
  my $tsidl = $file;
  $tsidl =~ s/\.idl$/Dcps.idl/;
  push(@out, $dir . basename($tsidl));

  return \@out;
}

sub get_outputexts {
  return ['Dcps\\.idl']; #these are regular expressions
}

1;
