#!/bin/bash

# *****************************************************************************
# *                            cloudFPGA
# *                Copyright 2016 -- 2022 IBM Corporation
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *     http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *----------------------------------------------------------------------------
#
#  *    =============================================
#  *     Created: Aug 2021
#  *     Authors: FAB, WEI, NGL, POL, DID
#  *
#  *     Description:
#  *        Main cFp_Vitis regressions script.
#  *
#  *     Details:
#  *       - This script is typically called the Jenkins server.
#  *       - It expects to be executed from the cFp_Vitis root directory.
#  *       - The '$cFpVitisRootDir' variable must be set externally. 
#  *       - All environment variables must be sourced beforehand.
#  *

# @brief A function to check if previous step passed.
# @param[in] $1 the return value of the previous command.
function exit_on_error {
    if [ $1 -ne 0 ]; then
        echo "EXIT ON ERROR: Regression '$0' FAILED."
        echo "  Last return value was $1."
        exit $1
    fi
}

# STEP-0: We need to set the right environment
export rootDir="$cFpRootDir/"  #the / is IMPORTANT
export cFpIpDir="$rootDir/ip/"
export cFpMOD="FMKU60"
export usedRoleDir="$rootDir/ROLE/vision/"
export usedRole2Dir="$rootDir/ROLE/2/"
export cFpSRAtype="Themisto"
export cFpXprDir="$rootDir/xpr/"
export cFpDcpDir="$rootDir/dcps/"
export roleName1="R1"
export roleName2="R2"

#also, we need a license:
export XILINXD_LICENSE_FILE=2100@pokwinlic1.pok.ibm.com:2100@pokwinlic2.pok.ibm.com:2100@pokwinlic3.pok.ibm.com


echo "Set cFp environment."
retval=1

echo "================================================================"
echo "===   START OF REGRESSION:" 
echo "===     $0"
echo "================================================================"

echo "================================================================"
echo "===   REGRESSION - START OF BUILD: 'monolithic' "
echo "===     $0"
echo "================================================================"
cd $cFpRootDir 
#make testError
make full_clean #just to be sure...
make monolithic
exit_on_error $? 
echo "================================================================"
echo "===   REGRESSION - END OF BUILD  : 'monolithic' "
echo "===     $0"
echo "================================================================"


echo "================================================================"
echo "===   REGRESSION - START OF BUILD: 'pr_full' "
echo "===     $0"
echo "================================================================"
cd $cFpRootDir 
make clean 
make pr_full
exit_on_error $? 
echo "================================================================"
echo "===   REGRESSION - END OF BUILD  : 'pr_full' "
echo "===     $0"
echo "================================================================"

# we don't need this here
#echo "================================================================"
#echo "===   REGRESSION - START OF COSIM "
#echo "===     $0"
#echo "================================================================"
#export cFdkRootDir=$cFpFlashRootDir/cFDK
#cd $cFdkRootDir 
#sh $cFdkRootDir/REG/run_cosim_reg.sh
#exit_on_error $? 
#echo "================================================================"
#echo "===   REGRESSION - END OF COSIM "
#echo "===     $0"
#echo "================================================================"

exit 0

