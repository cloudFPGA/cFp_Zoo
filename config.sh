#!/bin/bash
#
# @brief  A script for selecting the targeted cFp_Vitis domain and kernel
# @author DID
# @date Nov. 2020


die() {
    echo >&2 "$0 ERROR: $@"
    exit 1
}


dialog --output-fd 1 --title "Hello cF Developer" --msgbox 'Through this utility you can quickly 
configure a cFp_Vitis project by selecting a domain and a kernel of this domain.' 10 30 

####################################################################################################
# Select tcp/udp
option1=$(dialog --checklist --output-fd 1 "Select:" 0 0 2 \
  tcp "TCP ROLE" off \
  udp "UDP ROLE" on)
response=$?
case $response in
${DIALOG_CANCEL-1})  die "Aborting without selecting a domain";;
${DIALOG_ESC-255})   die "[ESC] key pressed.";;
${DIALOG_ERROR-255}) die "Dialog error";;
*) echo "Unknown error $retval"
esac

####################################################################################################
# Select Domain
option2=$(dialog --radiolist --output-fd 1 "Select domain" 0 0 0 \
  blas                  "Vitis BLAS Library" off \
  database              "Vitis Database Library" off \
  data_analytics        "Vitis Data Analytics Library" off \
  data_compression      "Vitis Data Compression Library" off \
  dsp                   "Vitis DSP Library" off \
  graph                 "Vitis Graph Library" off \
  quantitative_finance  "Vitis Quantitative Finance Library" on \
  security              "Vitis Security Library" off \
  solver                "Vitis Solver Library" off \
  sparse                "Vitis SPARSE Library" off \
  utils                 "Vitis Utility Library" off \
  vision                "Vitis Vision Library" off)
response=$?
case $response in
${DIALOG_CANCEL-1})  die "Aborting without selecting a domain";;
${DIALOG_ESC-255})   die "[ESC] key pressed.";;
${DIALOG_ERROR-255}) die "Dialog error";;
*) echo "Unknown error $retval"
esac

####################################################################################################
# Select Kernel
if [ $option2 = 'blas' ]; then
  option3=$(dialog --radiolist --output-fd 1 "Select BLAS kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'database' ]; then
  option3=$(dialog --radiolist --output-fd 1 "Select database kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'data_analytics' ]; then
  option3=$(dialog --radiolist --output-fd 1 "Select Data Analytics kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'data_compression' ]; then
  option3=$(dialog --radiolist --output-fd 1 "Select Data Compression kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'dsp' ]; then
  option3=$(dialog --radiolist --output-fd 1 "Select DSP kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'graph' ]; then
  option3=$(dialog --radiolist --output-fd 1 "Select Graph kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'quantitative_finance' ]; then
  option3=$(dialog --radiolist --output-fd 1 "Select Quantitative Finance kernel" 0 0 0 \
  MCEuropeanEngine "Monte-Carlo European Options Pricing Engine" on \
  N/A "N/A" off)
elif [ $option2 = 'security' ]; then
  option3=$(dialog --radiolist --output-fd 1 "Select Security kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'solver' ]; then
  option3=$(dialog --radiolist --output-fd 1 "Select Solver kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'sparse' ]; then
  option3=$(dialog --radiolist --output-fd 1 "Select Sparse kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'utils' ]; then
  option3=$(dialog --radiolist --output-fd 1 "Select Utils kernel" 0 0 0 \
  N/A "N/A" on \
  N/A "N/A" off)
elif [ $option2 = 'vision' ]; then
  option3=$(dialog --radiolist --output-fd 1 "Select Vision kernel" 0 0 0 \
  Harris "Harris Corner Detector" off \
  Gammacorrection "Gamma Correction Filter" on)
fi
response=$?
case $response in
${DIALOG_CANCEL-1})  die "Aborting without selecting a domain";;
${DIALOG_ESC-255})   die "[ESC] key pressed.";;
${DIALOG_ERROR-255}) die "Dialog error";;
*) echo "Unknown error $retval"
esac

####################################################################################################
# Confirmation
confirm=$(dialog --yesno --output-fd 1 "DO you want to continue?" 0 0 )
# Get exit status
# 0 means user hit [yes] button.
# 1 means user hit [no] button.
# 255 means user hit [Esc] key.
response=$?
case $response in
${DIALOG_OK-0}) bash create_cfp_json.sh $option2 && source env/setenv.sh &&\
python3 ./select_cfpvitis_kernel.py "$option1" $option2 $option3 &&\
echo -e "Succesfully configured cFp_Vitis with : option1:'$option1', option2:'$option2', option3:'$option3'.\n\n";;
${DIALOG_CANCEL-1})  die "Aborting without selecting a domain";;
${DIALOG_ESC-255})   die "[ESC] key pressed.";;
${DIALOG_ERROR-255}) die "Dialog error";;
*) echo "Unknown error $retval"
esac
