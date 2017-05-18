#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Check the Environment
source "$SCRIPT_DIR/build_check_env.sh"


ACE_CONFIG_DIR=$SCRIPT_DIR/build/ace

# config.h
if [[ ! -f "$ACE_ROOT/ace/config.h" ]]; then
    echo "----"
    echo "ACE has not been configured with a configuration file. "
    echo " copying one for you ..."
    cp -v "$ACE_CONFIG_DIR/config.h" "$ACE_ROOT/ace/config.h"        
fi

#platform_macros.GNU
if [[ -L "$ACE_ROOT/include/makeinclude/platform_macros.GNU" || ! -f "$ACE_ROOT/include/makeinclude/platform_macros.GNU" ]]; then
    echo "----"
    echo "ACE Looks like make has been configured with a symbolic link. "
    echo " Going to copy one for you.."
    
    # remove symbolic link
    if [[ -e "$ACE_ROOT/include/makeinclude/platform_macros.GNU" ]]; then
        rm -v "$ACE_ROOT/include/makeinclude/platform_macros.GNU"
    fi
    
    cp -v "$ACE_CONFIG_DIR/platform_macros.GNU" "$ACE_ROOT/include/makeinclude/platform_macros.GNU"
fi

# ARMv7 support
arch_test=`uname -m | grep armv7 | wc -c`
if [[ "$arch_test" -gt "0" ]]; then
echo "Detected ARMv7 Architecture checking for platform_linux_armv7l.GNU"

  if [[ ! -f "$ACE_ROOT/include/makeinclude/platform_linux_armv7l.GNU" ]]; then 
      echo "----"
      echo "Configuring ACE for ARMv7"
      echo " Going to copy one for you and configure platform_macros.GNU"
      
      cp -v "$ACE_CONFIG_DIR/platform_linux_armv7l.GNU" "$ACE_ROOT/include/makeinclude/platform_linux_armv7l.GNU"
      
      echo " You need to append platform_linux_arm7l.GNU to your platform_macros.GNU.. Attempting to do this automatically..."
      
      sed -i s/platform_linux.GNU/platform_linux_arm7l.GNU/  $ACE_ROOT/include/makeinclude/platform_macros.GNU

  fi

fi

# default features
if [[ ! -e "$ACE_ROOT/bin/MakeProjectCreator/config/default.features" ]]; then
    echo "----"
    echo "ACE Looks like MPC default.features is missing."
    echo " Going to copy one for you.."
    cp -v "$ACE_CONFIG_DIR/default.features.linux" "$ACE_ROOT/bin/MakeProjectCreator/config/default.features"
fi

#MPC base.cfg
MPC_ROOT=${MPC_ROOT:-$ACE_ROOT/MPC}
if [[ ! -e "$MPC_ROOT/config/base.cfg" ]]; then
    echo "----"
    echo " MPC has not be configured to use DAF."
    echo " Configuring '$MPC_ROOT/config/base.cfg'..."
    
    # Need a file to use readlink to get the full path
    # pwd -L doesn't resolve properly
    ABS_DAF_ROOT="$( dirname `readlink -f $DAF_ROOT/DAF.mwc` )"
    echo "$ABS_DAF_ROOT = \$DAF_ROOT/MPC/config" >> $MPC_ROOT/config/base.cfg

    # Add the entire filesystem to configure off DAF_ROOT/MPC/config
    echo "/ = \$DAF_ROOT/MPC/config" >> $MPC_ROOT/config/base.cfg
fi


