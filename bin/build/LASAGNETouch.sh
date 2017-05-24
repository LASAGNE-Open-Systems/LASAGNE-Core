#!/bin/bash

if [[ -z "$DAF_ROOT" || ! -d "$DAF_ROOT" ]]; then
  echo "*****************************************************************************"
  echo "DAF_ROOT is not defined or valid please check your development environment"
  echo "DAF_ROOT has not been 'touched'"
  echo "*****************************************************************************"
else
  find $DAF_ROOT -print0 | xargs -0 touch 
fi 

