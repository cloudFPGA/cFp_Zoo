#!/bin/bash
#
# *******************************************************************************
# * Copyright 2016 -- 2022 IBM Corporation
# *
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
# ********************************************************************************
#
# @brief  A script for selecting the targeted cFp_Zoo domain and kernel
# @author DID
# @date Nov. 2020


die() {
    echo >&2 "$0 ERROR: $@"
    exit 1
}


DIALOG="dialog --output-fd 1 "
#DIALOG=whiptail
#if [ -x /usr/bin/gdialog ] ;then DIALOG="gdialog --output-fd 1 " ; fi
#if [ -x /usr/bin/xdialog ] ;then DIALOG=xdialog ; fi

$DIALOG --title "Hello cF Developer" --msgbox 'Through this utility you can quickly configure a cFp_Zoo project by selecting a domain and a kernel of this domain.' 20 60 



####################################################################################################
# Cchecking if configuration file already exists
CONF_FILE=./etc/cFp_zoo.conf
if [ -f "$CONF_FILE" ]; then
    echo "$CONF_FILE exists."
    i=0

    confirm=$($DIALOG --yesno  "Detected $CONF_FILE. Do you want to:\n - load the configuration (select Yes) or\n - create a new one (select No) ?" 20 60 )
    # Get exit status
    # 0 means user hit [yes] button.
    # 1 means user hit [no] button.
    # 255 means user hit [Esc] key.
    response=$?
    case $response in
      ${DIALOG_OK-0}) load_conf=1;;
      ${DIALOG_CANCEL-1})  load_conf=0 && echo "Ignoring configuration file $CONF_FILE and starting creating a new one";;
      ${DIALOG_ESC-255})   die "[ESC] key pressed.";;
      ${DIALOG_ERROR-255}) die "Dialog error";;
      *) echo "Unknown error $retval"
    esac    

    if [ $load_conf = 1 ]; then
     while read line; do 
    
      case "$i" in
        0)
            option1=$line
            ;;
         
        1)
            option2=$line
            ;;
         
        2)
            option3=$line
            ;;
        3)
            option4=$line
            ;;
        4)
            option5=$line
            ;;
        5)
            option6=$line
            ;;         
        *)
            echo $"Wrong number of lines parsed in the configuration file}"
            exit 1
 
      esac
      let "i++"
     done < $CONF_FILE
     $DIALOG --title "Hello cF Developer" --msgbox "Successfully configured cFp_zoo from $CONF_FILE with :\nROLE   : $option1 \nDomain : $option2 \nKernel : $option3 \nMTU    : $option4 \nPort   : $option5\nDDR    : $option6 \n" 20 50
    fi # load_conf = 1
else
    echo "Configuration file $CONF_FILE does not exist."
    load_conf=0
fi

if [ $load_conf = 0 ]; then

####################################################################################################
# Select tcp/udp
option1=$($DIALOG --checklist "Select:" 0 0 2 \
  tcp "TCP ROLE" off \
  udp "UDP ROLE" on)
response=$?
case $response in
${DIALOG_OK-0})      echo "Selected net $option1";;
${DIALOG_CANCEL-1})  die "Aborting without selecting a domain";;
${DIALOG_ESC-255})   die "[ESC] key pressed.";;
${DIALOG_ERROR-255}) die "Dialog error";;
*) echo "Unknown error $retval"
esac

####################################################################################################
# Select Domain
option2=$($DIALOG --radiolist "Select domain" 0 0 0 \
  blas                  "Vitis BLAS Library" off \
  database              "Vitis Database Library" off \
  data_analytics        "Vitis Data Analytics Library" off \
  data_compression      "Vitis Data Compression Library" off \
  dsp                   "Vitis DSP Library" off \
  graph                 "Vitis Graph Library" off \
  quantitative_finance  "Vitis Quantitative Finance Library" off \
  security              "Vitis Security Library" off \
  solver                "Vitis Solver Library" off \
  sparse                "Vitis SPARSE Library" off \
  utils                 "Vitis Utility Library" off \
  vision                "Vitis Vision Library" on \
  custom                "IBMZRL Custom Library" off)

response=$?
case $response in
${DIALOG_OK-0})      echo "Selected domain $option2";;
${DIALOG_CANCEL-1})  die "Aborting without selecting a domain";;
${DIALOG_ESC-255})   die "[ESC] key pressed.";;
${DIALOG_ERROR-255}) die "Dialog error";;
*) echo "Unknown error $retval"
esac

####################################################################################################
# Select Kernel
if [ $option2 = 'blas' ]; then
  option3=$($DIALOG --radiolist "Select BLAS kernel" 0 0 0  \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'database' ]; then
  option3=$($DIALOG --radiolist "Select database kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'data_analytics' ]; then
  option3=$($DIALOG --radiolist "Select Data Analytics kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'data_compression' ]; then
  option3=$($DIALOG --radiolist "Select Data Compression kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'dsp' ]; then
  option3=$($DIALOG --radiolist "Select DSP kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'graph' ]; then
  option3=$($DIALOG --radiolist "Select Graph kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'quantitative_finance' ]; then
  option3=$($DIALOG --radiolist "Select Quantitative Finance kernel" 0 0 0 \
  MCEuropeanEngine "Monte-Carlo European Options Pricing Engine" on \
  N/A "N/A" off)
elif [ $option2 = 'security' ]; then
  option3=$($DIALOG --radiolist "Select Security kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'solver' ]; then
  option3=$($DIALOG --radiolist "Select Solver kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'sparse' ]; then
  option3=$($DIALOG --radiolist "Select Sparse kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'utils' ]; then
  option3=$($DIALOG --radiolist "Select Utils kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'vision' ]; then
  option3=$($DIALOG --radiolist "Select Vision kernel" 0 0 0 \
  Harris "Harris Corner Detector" on \
  Median_Blur "Median Blur Filter" off \
  Sobel "Sobel Filter" off \
  Gammacorrection "Gamma Correction Filter" off \
  Warp_Transform "Warp Transform" off)
elif [ $option2 = 'custom' ]; then
  option3=$($DIALOG --radiolist "Select Custom kernel" 0 0 0 \
  Uppercase "Select Uppercase kernel example" off \
  Memtest "Select Memtest kernel example" on \
  N/A "N/A" off)
else
  echo "Unknown domain."
  exit 1
fi
response=$?
case $response in
${DIALOG_OK-0})      echo "Selected kernel $option3";;
${DIALOG_CANCEL-1})  die "Aborting without selecting a domain";;
${DIALOG_ESC-255})   die "[ESC] key pressed.";;
${DIALOG_ERROR-255}) die "Dialog error";;
*) echo "Unknown error $retval"
esac



####################################################################################################
# Select MTU
#option4=$(dialog --title "MTU" --backtitle "2718" --inputbox "Enter port " 8 60)
option4=$($DIALOG --title "Select MTU" --rangebox "Please set the MTU (choose a value multiple of 8)" 0 60 8 1500 1024)
response=$?
case $response in
${DIALOG_OK-0})      echo "Selected MTU $option4";;
${DIALOG_CANCEL-1})  die "Aborting without selecting a domain";;
${DIALOG_ESC-255})   die "[ESC] key pressed.";;
${DIALOG_ERROR-255}) die "Dialog error";;
*) echo "Unknown error $retval"
esac

####################################################################################################
# Select Port
option5=$($DIALOG --title "Select port" --rangebox "Please set the port (both TCP and UDP)" 0 60 2700 2800 2718)
response=$?
case $response in
${DIALOG_OK-0})      echo "Selected port $option4";;
${DIALOG_CANCEL-1})  die "Aborting without selecting a domain";;
${DIALOG_ESC-255})   die "[ESC] key pressed.";;
${DIALOG_ERROR-255}) die "Dialog error";;
*) echo "Unknown error $retval"
esac


####################################################################################################
# Select DDR
option6=$($DIALOG --radiolist "Select DDR ROLE I/F" 0 0 0 \
  ddr_enabled           "Enable DDR" off \
  ddr_disabled          "Disable DDR" on )
response=$?
case $response in
${DIALOG_OK-0})      echo "Selected DDR $option6";;
${DIALOG_CANCEL-1})  die "Aborting without selecting DDR";;
${DIALOG_ESC-255})   die "[ESC] key pressed.";;
${DIALOG_ERROR-255}) die "Dialog error";;
*) echo "Unknown error $retval"
esac

fi # load_conf = 0


####################################################################################################
# Confirmation
confirm=$($DIALOG --yesno "Do you want to continue?" 0 0 )
# Get exit status
# 0 means user hit [yes] button.
# 1 means user hit [no] button.
# 255 means user hit [Esc] key.
response=$?
case $response in
${DIALOG_OK-0}) bash create_cfp_json.sh $option2 && source env/setenv.sh &&\
python3 ./select_cfpzoo_kernel.py "$option1" $option2 $option3 $option4 $option5 $option6 &&\
echo -e "Succesfully configured cFp_Zoo with : option1:'$option1', option2:'$option2', option3:'$option3', option4:'$option4', option5:'$option5', option6:'$option6'." &&\
echo -e "$option1\n$option2\n$option3\n$option4\n$option5\n$option6" > $CONF_FILE && echo -e "Configuration saved in $CONF_FILE\n\n";;
${DIALOG_CANCEL-1})  die "Aborting upon user selection";;
${DIALOG_ESC-255})   die "[ESC] key pressed.";;
${DIALOG_ERROR-255}) die "Dialog error";;
*) echo "Unknown error $retval"
esac
