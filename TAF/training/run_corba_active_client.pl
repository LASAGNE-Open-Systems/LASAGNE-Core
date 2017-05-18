eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
    & eval 'exec perl -S $0 $argv:q'
    if 0;

# $Id: run_test.pl 2115 2012-01-13 13:57:53Z dfeiock $
# -*- perl -*-

use lib "$ENV{'ACE_ROOT'}/bin";
use PerlACE::Run_Test;

$DAF_ROOT = "$ENV{'DAF_ROOT'}";
$duration = 30;



# Run the client
print "Running CORBA Active Service Clients for ".$duration." seconds\n";

# For training we often put up a single naming server just to avoid issues
# $server = new PerlACE::Process("$DAF_ROOT/bin/TAFServer", "-TAFProperties LASAGNEProperties.conf -ORBListenEndpoints iiop://1.2@:8989,1.2@:8986");
# $server->Spawn ();

# Establish clients for each endpoint address we wish to use
$SERVANT_IP = "127.0.0.1";
$SERVANT_PORT = "8989";
$SERVANT_NAME = "Local";
$SERVANT_OID = "LTM_CORBActiveService";

print "$SERVANT_IP:$SERVANT_PORT - $SERVANT_NAME\n";
$client0 = new PerlACE::Process("$DAF_ROOT/TAF/training/CORBActiveService/CORBActiveClient", "-ORBInitRef LTM_CORBActiveService=corbaloc:iiop:$SERVANT_IP:$SERVANT_PORT/$SERVANT_OID -ORBObjRefStyle URL");
$client0->Spawn ();

# Adjust parameters and then copy-paste to load additional test clients

sleep ($duration);

# Kill the clients - make sure to close any additional test clients spawned down here
$client0->Kill ();
$client0->TimedWait (1);

print "Finished CORBA Active Service Clients...\n";


exit 0;