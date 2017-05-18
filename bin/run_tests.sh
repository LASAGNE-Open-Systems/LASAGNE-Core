#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

perl $SCRIPT_DIR/auto_run_tests.pl -r $DAF_ROOT -l $SCRIPT_DIR/daf_tests.lst
