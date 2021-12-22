#!/bin/bash
# /*******************************************************************************
#  * Copyright 2016 -- 2022 IBM Corporation
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
#  *     Created: Nov 2021
#  *     Authors: FAB, WEI, NGL, DID
#  *
#  *     Description:
#  *       Bash script to build and manage cloudFPGA project
#  *
#  *

# check for bash alias
if [ "$0" == "./sra" ]; then
  export SraToolShowHint="True"
else
  export SraToolShowHint="False"
fi

# 0. get current folder
cur_dir=$(pwd)
# so, some kind of bootstrapping
# 1. source the cFp environment
source $cur_dir/env/setenv.sh
# 2. now, since the path are there:
source $cFenv_path/bin/activate
# 3. finally, invoke the script
python3 $cur_dir/env/cf_sratool.py "$@"

