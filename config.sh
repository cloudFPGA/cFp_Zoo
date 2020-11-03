#!/bin/bash
#
# @brief  A simple script for selecting the targeted cFp_Vitis kernel
# @author DID
# @date Nov. 2020

####################################################################################################
# Select tcp/udp
option1=$(dialog --checklist --output-fd 1 "Select:" 0 0 2 \
  tcp "TCP ROLE" off \
  udp "UDP ROLE" on)

####################################################################################################
# Select Domain
option2=$(dialog --radiolist --output-fd 1 "Select domain" 0 0 0 \
  vision "Vision" off \
  quant  "Quantitative Finance" on)

####################################################################################################
# Select Kernel
if [ $option2 = 'vision' ]; then
  option3=$(dialog --radiolist --output-fd 1 "Select Kernel" 0 0 0 \
  Harris "Harris Corner Detector" off \
  Gammacorrection "Gamma Correction Filter" on)
elif [ $option2 = 'quant' ]; then
  option3=$(dialog --radiolist --output-fd 1 "Select Kernel" 0 0 0 \
  MCEuropeanEngine "Monte-Carlo European Options Pricing Engine" on \
  2 "N/A" off)
fi

####################################################################################################
# Confirmation
confirm=$(dialog --yesno --output-fd 1 "DO you want to continue?" 0 0 )
# Get exit status
# 0 means user hit [yes] button.
# 1 means user hit [no] button.
# 255 means user hit [Esc] key.
response=$?
case $response in
${DIALOG_OK-0}) echo "option1:'$option1', option2:'$option2', option3:'$option3'." &&\
python3 ./select_cfpvitis_kernel.py $option1 $option2 $option3 ;;
${DIALOG_CANCEL-1}) echo "Aborting without selecting the kernel.";;
${DIALOG_ESC-255}) echo "[ESC] key pressed.";;
${DIALOG_ERROR-255}) echo "Dialog error";;
*) echo "Unknown error $retval"
esac
