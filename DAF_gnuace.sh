#!/bin/bash
#
# Generate the MPC artifacts for gnuace. 
#

# A variation on checking for definedness and validity. 
#: ${ACE_ROOT:?"ACE_ROOT is not defined please check your setup"}
#: ${DAF_ROOT:?"DAF_ROOT is not defined please check your setup"}

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
# It might be useful to add the following: 
# - to define a development variable for the Feature File and do some default behaviour. Something like:
# : ${MPC_FEATURE?"$DAF_ROOT/MPC/config/TAF.features"} # to setup a default on the MPC_FEATURE env var.
#

# Using pushd to negate calling from a different PWD. 
pushd "$DAF_ROOT" > /dev/null
  perl $ACE_ROOT/bin/mwc.pl -type gnuace DAF.mwc 
popd