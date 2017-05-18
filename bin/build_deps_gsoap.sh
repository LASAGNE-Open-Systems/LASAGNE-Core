#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Check the Environment
source "$SCRIPT_DIR/build_check_env.sh"

if [[ ! -d "$GSOAP_HOME" ]]; then
    echo "Error GSOAP_HOME must be valid environment variable"
    exit 1;
fi

if [[ $? -ne 0 ]]; then
    exit 1;
fi

if [[ -z "$GSOAP_ARCH" ]]; then
  GSOAP_ARCH=linux
fi

pushd $GSOAP_HOME > /dev/null
    ./configure         
    make
popd > /dev/null


if [[ ! -d "$GSOAP_HOME/gsoap/bin/$GSOAP_ARCH" ]]; then
    mkdir -p "$GSOAP_HOME/gsoap/bin/$GSOAP_ARCH"
fi

pushd $GSOAP_HOME/gsoap/bin/$GSOAP_ARCH > /dev/null
    if [[ ! -e wsdl2h ]]; then
        echo "Creating Symbolic link for 'wsdl2h'"
        if [[ ! -e "$GSOAP_HOME/gsoap/wsdl/wsdl2h" ]]; then
            echo " Error 'wsdl2h' does not exist. "
            echo " Was there a compilation failure? "                   
        else
            ln -s -v $GSOAP_HOME/gsoap/wsdl/wsdl2h wsdl2h
        fi
    fi
    
    if [[ ! -e soapcpp2 ]]; then
        echo "Creating Symbolic link for 'soapcpp2'"
        if [[ ! -e "$GSOAP_HOME/gsoap/src/soapcpp2" ]]; then
            echo " Error 'soapcpp2' does not exist. "
            echo " Was there a compilation failure? "                   
        else
            ln -s -v $GSOAP_HOME/gsoap/src/soapcpp2 soapcpp2
        fi
    fi

popd > /dev/null