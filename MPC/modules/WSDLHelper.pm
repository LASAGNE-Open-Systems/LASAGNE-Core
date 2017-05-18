package WSDLHelper;

use strict;
use CommandHelper;
use File::Basename;
our @ISA = qw(CommandHelper);

sub get_output {
  my($self, $file, $flags) = @_;

  #print "WSDLHelper Output $file\n";
  my $dir = '';
  $flags = '' unless defined $flags;
  if ($flags =~ /-d +(\S+)/) {
    $dir = "$1/";
    ## TODO check on directory existing
  }

  my @out = ();
  my $wsdhfile = $file;
  $wsdhfile =~ s/(.*).wsdl$/\1.wsdh/;
  #print "Out WSDH: $wsdhfile\n";
  push(@out, $wsdhfile);

  ##
  ## Need to find the PortBinding Entries of the original WSDL
  ##
  #print "Inspecting : '$file'\n";
  if ( open (WSDLFILE, "$file") ) {
    # Grep the file for <binding name=... > 
    my @match = grep { /<binding\sname=/ }(<WSDLFILE>);
    my @bindings;
    my $mat;
    for $mat(@match) {
        # extract the Name only
        $mat =~ s/.*name="(.*?)".*/\1/;
        #print "WSDL:Binding Match $mat";
        push(@bindings, $mat);
    }
    close(WSDLFILE);
    
    ##
    ## Output the Bindings to our temporary file
    ##
    my $bindfile = $file;
    $bindfile =~ s/(.*).wsdl/\1.bind/;
    
    if ( open (BINDING, ">$bindfile") ) {
        my $bind;
        foreach $bind(@bindings) {
            #print "WSDL:Bind:$bind\n";
            print BINDING "$bind";
        }
        close(BINDING);
        ## DON'T push out the binding file... it stuffs MPC
        ##push(@out, $bindfile);
    } else {
      print "WSDLHelper : Could not open file '$bindfile' for writing\n";
    }
    
    
  } else {
    print "WSDLHelper : Could not find input file '$file'\n";
  }
   
  return \@out;
}

#sub get_outputexts {
#  return ['\\.idl']; #these are regular expressions
#}

1;
