#!/bin/bash
# /*******************************************************************************
#  * Copyright 2016 -- 2021 IBM Corporation
#  *
#  * Licensed under the Apache License, Version 2.0 (the "License");
#  * you may not use this file except in compliance with the License.
#  * You may obtain a copy of the License at
#  *
#  *     http://www.apache.org/licenses/LICENSE-2.0
#  *
#  * Unless required by applicable law or agreed to in writing, software
#  * distributed under the License is distributed on an "AS IS" BASIS,
#  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  * See the License for the specific language governing permissions and
#  * limitations under the License.
# *******************************************************************************/

#  *
#  *                       cloudFPGA
#  *    =============================================
#  *     Created: Feb 2020
#  *     Authors: FAB, WEI, NGL, DID
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
#echo $DIR

export cFsysPy3_cmd_hint_0=$(hash -d python3.8 2>>/dev/null; which python3.8 2>>/dev/null || echo "failed")
export cFsysPy3_cmd_hint_1=$(hash -d python3 2>>/dev/null; which python3 2>>/dev/null || echo "failed")

# cFCreate also requires python3...so it should be there
# will guarantee an up to date env file
# on success, load env
$DIR/gen_env.py && source $DIR/this_machine_env.sh


