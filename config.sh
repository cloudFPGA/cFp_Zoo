#!/bin/bash
#
# @brief  A script for selecting the targeted cFp_Vitis domain and kernel
# @author DID
# @date Nov. 2020


dialog --output-fd 1 --title "Hello cF Developer" --msgbox 'Through this utility you can quickly 
configure a cFp_Vitis project by selecting a domain and a kernel of this domain.' 10 30 

####################################################################################################
# Select tcp/udp
option1=$(dialog --checklist --output-fd 1 "Select:" 0 0 2 \
  tcp "TCP ROLE" off \
  udp "UDP ROLE" on)

####################################################################################################
# Select Domain
option2=$(dialog --radiolist --output-fd 1 "Select domain" 0 0 0 \
  vision                "Vision" off \
  quantitative_finance  "Quantitative Finance" on)
response=$?
case $response in
${DIALOG_CANCEL-1})  echo "Aborting without selecting a domain";;
${DIALOG_ESC-255})   echo "[ESC] key pressed.";;
${DIALOG_ERROR-255}) echo "Dialog error";;
*) echo "Unknown error $retval"
esac

####################################################################################################
# Select Kernel
if [ $option2 = 'vision' ]; then
  option3=$(dialog --radiolist --output-fd 1 "Select Kernel" 0 0 0 \
  Harris "Harris Corner Detector" off \
  Gammacorrection "Gamma Correction Filter" on)
elif [ $option2 = 'quantitative_finance' ]; then
  option3=$(dialog --radiolist --output-fd 1 "Select Kernel" 0 0 0 \
  MCEuropeanEngine "Monte-Carlo European Options Pricing Engine" on \
  N/A "N/A" off)
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
bash create_cfp_json.sh $option2 && source env/setenv.sh &&\
python3 ./select_cfpvitis_kernel.py "$option1" $option2 $option3 ;;
${DIALOG_CANCEL-1}) echo "Aborting without selecting the kernel.";;
${DIALOG_ESC-255})   echo "[ESC] key pressed.";;
${DIALOG_ERROR-255}) echo "Dialog error";;
*) echo "Unknown error $retval"
esac
