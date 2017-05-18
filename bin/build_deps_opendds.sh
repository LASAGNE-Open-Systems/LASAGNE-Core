#!/bin/bash


SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Check the Environment
source "$SCRIPT_DIR/build_check_env.sh"

if [[ $? -ne 0 ]]; then
    exit 1;
fi

#
# Test for ACE/TAO build
#
if [[ ! -e "$ACE_ROOT/lib/libACE.so" ]]; then
    echo "---"
    echo " OpenDDS ACE doesn't look like its been built"
    echo " build ACE/TAO first"
    exit 1;
fi



#
# Generate and Build OpenDDS
#
pushd $DDS_ROOT > /dev/null
    if [[ ! -e "$DDS_ROOT/GNUmakefile_daf_build" ]]; then
        echo "MPC generation..."
        $ACE_ROOT/bin/mwc.pl -type gnuace -name_modifier *_daf_build DDS_no_tests.mwc
    fi
    
    make -f GNUmakefile_daf_build
popd


