#!/bin/bash

#
# Generate the MPC artifacts for gnuace. 
#

if [[ -z "$ACE_ROOT" || ! -d "$ACE_ROOT" ]]; then
  echo "*****************************************************************************"
  echo "ACE_ROOT is not defined or valid please check your development environment"  
  echo "*****************************************************************************"
  exit  1
fi

if [[ -z "$DAF_ROOT" || ! -d "$DAF_ROOT" ]]; then
  echo "*****************************************************************************"
  echo "DAF_ROOT is not defined or valid please check your development environment"
  echo "*****************************************************************************"
  exit  1
fi 


#
# ${TAF_ROOT:-"$DAF_ROOT/TAF"} -> if TAF_ROOT is defined
# then use that, if not make a guess with DAF_ROOT which 
# we know is valid from above. ie Setting a default value.
#
pushd ${TAF_ROOT:-"$DAF_ROOT/TAF"} > /dev/null
  perl $ACE_ROOT/bin/mwc.pl -type gnuace TAF.mwc 
popd > /dev/null
