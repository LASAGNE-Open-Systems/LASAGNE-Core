package WSDHHelper;

use strict;
use CommandHelper;
use File::Basename;
our @ISA = qw(CommandHelper);

sub get_output {
  my($self, $file, $flags) = @_;

  #print "WSDHHelper Output $file\n";
  my $dir = '';
  $flags = '' unless defined $flags;
  if ($flags =~ /-d +(\S+)/) {
    $dir = "$1/";
    ## TODO check on directory existing
  }

  my @out = ();
  
  my $prefix = 'soap';
  if ($flags =~ /-p +(\S+)/) {
    $prefix = "$1";
  }
  
  
  my $client = 'Client';
  my $server = 'Server';
  my $ext = '.cpp';
  my $core = $file;
  $core =~ s/(.*).wsdh$/\1/;
  
  
  if ($flags =~ /(-c)/ ) {
    $ext = '.c';
  }
  
  ##
  
  if ($flags =~ /(-j)|(-i)/) {
    $client = 'Proxy';
    $server = 'Service';
  } else {
    $core = '';
  }
    
  ##
  ## Need to find the PortBinding Entries of the original WSDL. 
  ## This cooperates with WSDLHelper.pm and the intermediate
  ## .bind file
  ##
  my $bindfile = $file;
  $bindfile =~ s/(.*).wsdh/\1.bind/;
  
  
  ## Read in the Binding Names
  # print "WSDH:Inspecting : '$bindfile'\n";
  if ( open (BINDING, "<$bindfile") ) {
    my @bindfile = <BINDING>;
    my @bindings;
    close(BINDING);
    my $bind;
    my $str;
    for $str(@bindfile) {
        #Trim whitespace
        $str =~ s/^\s+|\s+$//g;
        #print "WSDH:Binding Match '$str'\n";
        push(@bindings, $str);
    }
        
        
        ## NOT Server ONLY
        if ( !($flags =~ /(-S)/) ) {
            foreach $bind(@bindings) {
                my $proxy = $prefix . $bind .  $client;
            
            
                #print "Proxy File: $proxy\n";
                push(@out, $dir . basename($proxy . ".h"));
                push(@out, $dir . basename($proxy . $ext));
            }
        }

        ## NOT Client ONLY
        if (!($flags =~ /(-C)/)) {
            foreach $bind(@bindings) {
                my $service = $prefix . $bind . $server;
            
                #print "Service File: $service\n";
                push(@out, $dir . basename($service . ".h"));
                push(@out, $dir . basename($service . $ext));
            }
        }
  } else {
     print "WSDHHelper : Count not find input file '$bindfile'\n";
  }
   
  #Tester
  if ( $flags =~ /(-T)/ ) {
     my $tester = $prefix . 'Tester';

    #print "Tester File: $tester\n";
     push(@out, $dir . basename($tester . $ext));
  }
  
  # Lib Generation
  if ( !($flags =~ /(-L)|(-j)|(-i)/) ) {
    my $clientlib = $prefix . 'ClientLib';
    my $serverlib = $prefix . 'ServerLib';
     
        
    #print "ClientLib File: $clientlib\n";
    #print "ServerLib File: $serverlib\n";        
    push(@out, $dir . basename($clientlib . $ext));
    push(@out, $dir . basename($serverlib . $ext));
  }
    
  

  ## Base gSOAP output files
  #
  my $cfile = $prefix . "C" . $ext;
  #print "Cfile : $cfile\n";
  push(@out, $dir . basename($cfile));
  
  my $hfile = $prefix . "H.h";
  my $stubfile = $prefix . "Stub.h";
  push(@out, $dir . basename($hfile));
  push(@out, $dir . basename($stubfile));

  
  return \@out;
}

#sub get_outputexts {
#  return ['\\.idl']; #these are regular expressions
#}

1;
