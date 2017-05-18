eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
    & eval 'exec perl -S $0 $argv:q'
    if 0;

# $Id: run_test.pl 2115 2012-01-13 13:57:53Z dfeiock $
# -*- perl -*-

use lib "$ENV{'ACE_ROOT'}/bin";
use PerlACE::Run_Test;

$DAF_ROOT = "$ENV{'DAF_ROOT'}";
$duration = 20;
$properties = "$DAF_ROOT/bin/LASAGNEProperties.conf:training";
$services = "Training.conf:corba";

# Run the example.
print "Running CORBA Active Service for ".$duration." seconds\n";
print "\t-TAFProperties ".$properties."\n";
print "\t-TAFServices ".$services."\n";
print "\n";


# Ensure we have a Naming Service available
print "Running CORBA Naming Service for ".$duration." seconds\n";
# Run the example.
print "Running CORBA Active Service for ".$duration." seconds\n";

$the_TAFServer = new PerlACE::Process ("$DAF_ROOT/bin/TAFServer", "-TAFProperties $properties -TAFServices $services -DCPSConfigFile Training_DDS.ini -ORBListenEndpoints iiop://:8989");
$the_TAFServer->Spawn ();

# Allow the server a small amount of time to start
sleep (2);

# Run the client
print "Running CORBA Active Service Client for ".$duration." seconds\n";

$the_client = new PerlACE::Process("$DAF_ROOT/TAF/training/CORBActiveService/CORBActiveClient", "-ORBInitRef LTM_CORBActiveService=corbaloc:iiop:localhost:8989/LTM_CORBActiveService -ORBObjRefStyle URL");
$the_client->Spawn ();

sleep ($duration);

# Kill the client
$the_client->Kill ();
$the_client->TimedWait (1);

print "Finished CORBA Active Service Client...\n";

# Kill the application server
$the_TAFServer->Kill ();
$the_TAFServer->TimedWait (1);

print "Finished CORBA Active Service...\n";
print "Finished CORBA Naming Service...\n";

exit 0;