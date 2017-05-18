#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Check the Environment
source "$SCRIPT_DIR/build_check_env.sh"

if [[ ! -d "$SSL_ROOT" ]]; then 
    echo "Error SSL_ROOT must be a valid environment variable"
    exit 1;
fi


pushd "$SSL_ROOT" > /dev/null

./config --prefix=$SSL_ROOT/install shared 

make
make test
make install

# symbolic links for MPC configuration
if [[ ! -d bin ]]; then
  ln -s install/bin bin
fi

if [[ ! -d lib ]]; then
  ln -s install/lib lib
fi

popd > /dev/null