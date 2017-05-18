#!/bin/bash


SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Check the Environment
source "$SCRIPT_DIR/config_deps_ace_mpc.sh"

if [[ $? -ne 0 ]]; then
    exit 1;
fi

pushd $TAO_ROOT > /dev/null
    if [[ ! -e "$TAO_ROOT/GNUmakefile_daf_build" ]]; then
        echo "MPC generation..."
        $ACE_ROOT/bin/mwc.pl -type gnuace -name_modifier *_daf_build TAO_ACE.mwc    
    fi
    
    make -f GNUmakefile_daf_build
popd > /dev/null