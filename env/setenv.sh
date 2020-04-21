#!/bin/bash
#  *
#  *                       cloudFPGA
#  *     Copyright IBM Research, All Rights Reserved
#  *    =============================================
#  *     Created: Feb 2020
#  *     Authors: FAB, WEI, NGL
#  *
#  *     Description:
#  *        Bash wrapper for parsing the cFp and/or sourcing the environment of this machine
#  *


if [[ $0 == $BASH_SOURCE ]]; then
  echo "THIS script should be called with 'source /path/to/script.sh'"
  #exit 1
fi

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
echo=$DIR

# cFBuild also requires python3...so it should be there
# will guarantee an up to date env file
# on success, load env
$DIR/gen_env.py && source $DIR/this_machine_env.sh


