#!/bin/bash



BASE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"



#

# Default Configuration Sections.

#

: ${TAF_CONFIG:="ODDS-TDF,ODDS-RADAR"}

: ${TAF_SERVICES:="$BASE_DIR/DSTO.conf:$TAF_CONFIG"}



if [[ ! -x "$BASE_DIR/TAFServer" ]]; then

  echo "*********************************************************"

  echo " TAFServer does NOT exist. Have you built TAF?"

  echo "*********************************************************"

  exit 1

fi



echo "TAFServer Configuration $TAF_CONFIG"



$BASE_DIR/TAFServer -TAFServices $TAF_SERVICES -DCPSConfigFile $BASE_DIR/dds.ini -ORBListenEndpoints iiop://:8989 
$*
