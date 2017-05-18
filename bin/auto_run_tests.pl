eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
    & eval 'exec perl -S $0 $argv:q'
    if 0;

# $Id: auto_run_tests.pl 97634 2014-02-25 09:31:17Z johnnyw $
# -*- perl -*-
# This file is for running the run_test.pl scripts listed in
# auto_run_tests.lst.

use lib "$ENV{ACE_ROOT}/bin";

use PerlACE::Run_Test;

use English;
use Getopt::Std;
use Cwd;


use Env qw(ACE_ROOT PATH TAO_ROOT DAF_ROOT TAF_ROOT);

if (!defined $TAF_ROOT && -d "$DAF_ROOT/TAF") {
    $TAF_ROOT = "$DAF_ROOT/TAF";
}


################################################################################

if (!getopts ('l:r:hz') || $opt_h) {
    print "auto_run_tests.pl [-a] [-h] [-s sandbox] [-o] [-t]\n";
    print "\n";
    print "Runs the tests listed in auto_run_tests.lst\n";
    print "\n";
    print "Options:\n";
    print "    -h             Display this help\n";
    print "    -d             Directory to run from\n";
    print "    -l list        Load the list file\n";
    print "    -r dir         Root directory for running the tests\n";
    print "    -z             Run debug mode, no tests executed\n";


    print "\n";
    $test_config_list = new PerlACE::ConfigList;    
    
    if (defined $DAF_ROOT) {
        $test_config_list->load($DAF_ROOT . "/bin/daf_test.lst");
        print "Test List: " . $test_config_list->list_configs() . "\n";
    }    
    exit (1);
}

my @file_list;



if ($opt_r) {
  $startdir = $opt_r;
}
else {
  $startdir = "$DAF_ROOT";
}

if ($opt_l) {
push (@file_list, "$opt_l");
}

if (scalar(@file_list) == 0) {
    print "Not List Catalog included \n";
    exit(1);
}

foreach my $test_lst (@file_list) {

    my $config_list = new PerlACE::ConfigList;
    
    if (-r "$startdir/$test_lst") {
      $config_list->load ("$startdir/$test_lst");
    }
    else {
      $config_list->load ($test_lst);
    }

    # Insures that we search for stuff in the current directory.
    $PATH .= $Config::Config{path_sep} . '.';

    foreach $test ($config_list->valid_entries ()) {
        my $directory = ".";
        my $program = ".";

        ## Remove intermediate '.' directories to allow the
        ## scoreboard matrix to read things correctly
        $test =~ s!/./!/!g;

        if ($test =~ /(.*)\/([^\/]*)$/) {
            $directory = $1;
            $program = $2;
        }
        else {
            $program = $test;
        }

        # this is to ensure that we dont print out the time for tests/run_test.pl
        # that test prints out the times for each of the ace tests individually
        my($orig_dir) = $directory;
        

        $status = undef;
        my @dirlist = ();
        # when $opt_r is set make sure to *first* check the explicitly
        # specified directory and only when nothing found there check
        # the default dirs
        if ($opt_r) {
          unshift (@dirlist, $startdir."/$directory");
          unshift (@dirlist, $startdir."/$orig_dir");
        }
        foreach my $path (@dirlist) {
          if (-d $path && ($status = chdir ($path))) {
            last;
          }
        }

        if (!$status) {
          if ($opt_r) {
            print STDERR "ERROR: Cannot chdir to $startdir/$directory\n";
          } else {
            print STDERR "ERROR: Cannot chdir to $directory\n";
          }
          next;
        }

        if ($program =~ /(.*?) (.*)/) {
            if (! -e $1) {
                print STDERR "ERROR: $directory.$1 does not exist\n";
                next;
              }
          }
        else {
            if (! -e $program) {
                print STDERR "ERROR: $directory.$program does not exist\n";
                next;
              }
          }

        ### Generate the -ExeSubDir and -Config options
        my $inherited_options = " -ExeSubDir $PerlACE::Process::ExeSubDir ";

        foreach my $config ($config_list->my_config_list ()) {
             $inherited_options .= " -Config $config ";
        }

        $cmd = '';
        if ($opt_s) {
            #The Win32 sandbox takes the program and options in quotes, but the
            #posix sandbox takes the program and options as separate args.
            my($q) = ($^O eq 'MSWin32') ? '"' : '';
            $cmd = "$opt_s ${q}perl $program $inherited_options${q}";
        }
        else {
            $cmd = "perl $program$inherited_options";
        }

        my $result = 0;

        if (defined $opt_z) {
            print "Running: $cmd\n";
        }
        else {
            $start_time = time();
            $result = system ($cmd);
            $time = time() - $start_time;

            # see note about tests/run_test.pl printing reports for ace tests individually            
            if ($result != 0) {
                print "Error: $test returned with status $result\n";
            }

            print "\nauto_run_tests_finished: $test Time:$time"."s Result:$result\n";
            print "==============================================================================\n";
        
        }
    }
}
