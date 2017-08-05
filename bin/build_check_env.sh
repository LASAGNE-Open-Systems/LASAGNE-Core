#!/bin/bash


#
# This script will check your environment to ensure 
# you can build correctly 
#

STATE=1

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"



MPC_FILE=${MPC_FILE:-$SCRIPT_DIR/../MPC/config/TAF.features}

echo "MPC_FILE:$MPC_FILE"

#
# Check DAF/TAF/DSTO
# - Check the installation folder is the current env
# - Offer to Setup the environment to installation
features_check() {
    #echo $1
    
    #echo "feature MPC_FILE:$MPC_FILE"
    local FEATURE_ON=`cat $MPC_FILE | grep -m 1 -E "^$1\s" | cut -d '=' -f2 | tr -d ' ' | sed -e 's/\s*$//'`
    #echo "FEATURE $1=$FEATURE_ON"
    echo "$FEATURE_ON"    
}

library_check() {
    IFS=:
    
    for p in ${LD_LIBRARY_PATH}; do
      if [ -e ${p}/lib$1.so ]; then
        echo "${p}/lib$1.so"
      fi    
    done
    echo "0"
}


#
# ACE/TAO Environment
#

if [[ -z "$ACE_ROOT" || ! -d "$ACE_ROOT" ]]; then
  echo "*****************************************************************************"
  echo "ACE_ROOT is not defined or valid please check your development environment"  
  echo "*****************************************************************************"
  STATE=0
else 
    #
    # Checks on ACE/TAO version
    #
    export ACE_MAJOR_VERSION=`cat $ACE_ROOT/ace/Version.h | grep MAJOR_VERSION | cut -d' ' -f3`
    export ACE_MINOR_VERSION=`cat $ACE_ROOT/ace/Version.h | grep MINOR_VERSION | cut -d' ' -f3`
    #echo "ACE_MAJOR_VERSION=$ACE_MAJOR_VERSION"
    #echo "ACE_MINOR_VERSION=$ACE_MINOR_VERSION"
    
    #if [ "$ACE_MAJOR_VERSION" -ge "6" && "$ACE_MINOR_VERSION" -ge "3" ]; then
    #    echo "ACE $ACE_MAJOR_VERSION.$ACE_MINOR_VERSION"
    #else 
    #    echo "WARNING: ACE Version $ACE_MAJOR_VERSION.$ACE_MINOR_VERSION is not supported. Please upgrade #to 6.3.x or higher"
    #fi
fi

if [[ -z "$TAO_ROOT" || ! -d "$TAO_ROOT" ]]; then
  echo "*****************************************************************************"
  echo "TAO_ROOT is not defined or valid please check your development environment"  
  echo "*****************************************************************************"
  STATE=0
fi

#
# LD_LIBRARY_PATH Checks. Can't detect whether. A. LD_LIBRARY_PATH is incorrect. B. It hasn't been built.
#
if [[ $(library_check TAO_IDL_FE) == "0" ]]; then
  echo "WARNING: LD_LIBRARY_PATH - TAO libraries missing from ld path"
  export LD_LIBRARY_PATH=$ACE_ROOT/lib:$LD_LIBRARY_PATH  
fi




if [[ -z "$DAF_ROOT" || ! -d "$DAF_ROOT" ]]; then
  echo "*****************************************************************************"
  echo "DAF_ROOT missing : configuring...."  
  echo "*****************************************************************************"
  export DAF_ROOT="$SCRIPT_DIR/.."
fi

if [[ -z "$TAF_ROOT" || ! -d "$TAF_ROOT" ]]; then
  echo "*****************************************************************************"
  echo "TAF_ROOT missing : configuring...."  
  echo "*****************************************************************************"
  export TAF_ROOT="$DAF_ROOT/TAF"
fi

if [[ $(library_check DAF) == "0" ]]; then
    echo "WARNING: LD_LIBRARY_PATH - DAF libraries missing"
    export LD_LIBRARY_PATH=$DAF_ROOT/lib:$LD_LIBRARY_PATH
fi


#
# OpenDDS Environment
# Check TAF.features
#echo "Checking OpenDDS... '$(features_check opendds)'"

if [[ $(features_check opendds) == "1" ]]; then
    #echo "Checking OpenDDS..."
    if [[ -z "$DDS_ROOT" || ! -d "$DDS_ROOT" ]]; then
        echo "*****************************************************************************"
        echo "DDS_ROOT is not defined or valid please check your development environment"  
        echo "*****************************************************************************"
        STATE=0
    fi
        
    if [[ $(features_check java) == "1" ]]; then
        if [[ -z "$JAVA_HOME" || -z "$JAVA_PLATFORM" || ! -d "$JAVA_HOME" ]]; then
            echo "*****************************************************************************"
            echo "JAVA_HOME or JAVA_PLATFORM has not been set for OpenDDS"
            echo "please check your development environment"  
            echo "*****************************************************************************"
            STATE=0
        fi        
    fi
    
    # TODO: Do some checks on LD_LIBRARY_PATH for opendds_idl
    if [[ $(library_check OpenDDS_Dcps) == "0" ]]; then
        echo "WARNING: LD_LIBRARY_PATH - OpenDDS libraries missing"
        export LD_LIBRARY_PATH=$DDS_ROOT/lib:$LD_LIBRARY_PATH
    fi
fi


#
# RTI NDDS Environment
# Check TAF.features
# Check CCK
if [[ "$(features_check ndds)" == "1" ]]; then
    #echo "Checking RTI Connext NDDS..."
    if [[ -z "$NDDSHOME" || ! -d "$NDDSHOME" ]]; then
        echo "*****************************************************************************"
        echo "NDDSHOME is not defined or valid please check your development environment"  
        echo "*****************************************************************************"
        STATE=0
    fi
    
    if [[ -z "$NDDSARCHITECTURE" || ! -d "$NDDSHOME/lib/$NDDSARCHITECTURE" ]]; then
        echo "*****************************************************************************"
        echo "NDDSARCHITECTURE is not defined or valid check your development environment"
        echo "See RTI documentation for more information"  
        echo "*****************************************************************************"
        STATE=0
    fi    
    
    if [[ $(features_check nddscorba) == "1" ]]; then
        echo "checks for nddscorba - NOT YET IMPLEMENTED"
    fi
    
    # TODO: Do some checks on LD_LIBRARY_PATH for ndds libs.
    if [[ $(library_check nddscore) == "0" ]]; then
        echo "WARNING: LD_LIBRARY_PATH missing NDDS libraries"
        export LD_LIBRARY_PATH=$NDDSHOME/lib/$NDDSARCHITECTURE:$LD_LIBRARY_PATH
    fi
fi





#
# CoreDX Environment
# Check TAF.features
# Check CCK
if [[ $(features_check coredx) == "1" ]]; then
    #echo "Checking TwinOaks CoreDX..."
    
    # Check for COREDX_TOP
    if [[ -z "$COREDX_TOP" || ! -d "$COREDX_TOP" ]]; then
        echo "*****************************************************************************"
        echo "COREDX_TOP is not defined or valid please check your development environment"  
        echo "*****************************************************************************"
        STATE=0
    #else 
        # Disabling the cdxenv script - it requies user input when you have multiple architectures/targets
        #source "$COREDX_TOP/scripts/cdxenv.sh"
    fi
    
    # Check for COREDX_ARCHITECTURE
    if [[ -z "$COREDX_HOST" || -z "$COREDX_TARGET" ]]; then
        echo "*****************************************************************************"
        echo "COREDX_HOST or COREDX_TARGET look empty."
        echo 'Consider Running the script $COREDX_TOP/scripts/cdxenv.sh'
        echo "*****************************************************************************"
        STATE=0
    fi
    

    # Check for TWINOAKS_LICENSE_FILE
    if [[ -z "$TWINOAKS_LICENSE_FILE" || ! -e "$TWINOAKS_LICENSE_FILE" ]]; then
        echo "*****************************************************************************"
        echo "TWINOAKS_LICENSE_FILE has not been set or is not valid please check your development environment"  
        echo "*****************************************************************************"
        STATE=0    
    fi
    
    if [[ $(features_check coredxcorba) == "1" ]]; then
        echo "Checks for CoreDX Corba - NOT YET IMPLEMENTED"    
    fi
    
    # TODO: Do some checks on LD_LIBRARY_PATH for coredx libs
fi

#
# OpenSSL Build
#
if [[ $(features_check openssl) == "1" ]]; then
    echo "Checking OpenSSL..."
    
    if [[ -z "$SSL_ROOT" || ! -d "$SSL_ROOT" ]]; then
        echo "*****************************************************************************"
        echo "SSL_ROOT is not defined or valid OpenSSL is going to have problems"  
        echo "*****************************************************************************"
        STATE=0    
    fi
fi

#
# gSOAP Environment
# Check TAF.features
if [[ $(features_check gsoap) == "1" ]]; then
    #echo "Checking gSOAP..."
    
    if [[ -z "$GSOAP_HOME" || ! -d "$GSOAP_HOME" ]]; then
        echo "*****************************************************************************"
        echo "GSOAP_HOME is not defined or valid please check your development environment"  
        echo "*****************************************************************************"
        STATE=0    
    fi    
    
    # TODO: Do some checks on PATH for wsdl2h soapcpp2 etc.
fi

#
# OpenSplice Environment
#
if [[ $(features_check opensplice) == "1" ]]; then
    
    if [[ -z "$OSPL_HOME" || ! -d "$OSPL_HOME" ]]; then
       echo "*****************************************************************************"
        echo "OpenSplice - OSPL_HOME is not defined or valid please check your development environment" 
        echo 'Consider Running "source $OSPL_HOME/release.com" '
        echo "*****************************************************************************"
        STATE=0    
    fi
    
    if [[ -z "$OSPL_URI" ]]; then
       echo "*****************************************************************************"
        echo "OpenSplice - OSPL_URI is not defined you may have trouble with executing OpenSplice binaries"
        echo "*****************************************************************************"
        # Not a Deal Breaker... 
        #STATE=0  
    fi
    
    if [[ $(library_check ddskernel) == "0" ]]; then
        echo "WARNING: LD_LIBRARY_PATH - OpenSplice libraries missing"
        export LD_LIBRARY_PATH=$OSPL_HOME/lib:$LD_LIBRARY_PATH
    fi
    

    if [[ $(features_check osplcorba) == "1" ]]; then                
        if [[ -z "$SPLICE_ORB" ]]; then
        echo "*****************************************************************************"
            echo "OpenSplice - SPLICE_ORB is not defined you may have problems with OpenSplice + CORBA compatibility."
            echo "*****************************************************************************"            
            STATE=0  
        fi
    fi
fi

if [ $STATE -ne 1 ]; then
    echo "Errors were found in your configuration. You can either"
    echo "A. Set the Environment Variables identified "
    echo "B. Disable MPC features in $MPC_FILE "
    exit 1;
fi
