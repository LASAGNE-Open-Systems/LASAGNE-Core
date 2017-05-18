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
$services = "Training.conf:reactive";

# Run the example.
print "Running Reactive Service for ".$duration." seconds\n";
print "\t-TAFProperties ".$properties."\n";
print "\t-TAFServices ".$services."\n";
print "\n";

$the_TAFServer = new PerlACE::Process ("$DAF_ROOT/bin/TAFServer", "-TAFProperties $properties -TAFServices $services");
$the_TAFServer->Spawn ();

sleep ($duration);

# Kill the application
$the_TAFServer->Kill ();
$the_TAFServer->TimedWait (1);

print "Finished Reactive Service...\n";

exit 0;