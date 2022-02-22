#!/bin/bash
# Note important difference between /bin/sh and /bin/bash
# https://askubuntu.com/q/141928/295286

# define functions (the function definition must be placed before any calls to the function)
# helper function
HELPER_FUNCTION(){
        echo -e "*****************************************************************"
        echo -e "# This script reads the power values from IPMI."
        echo -e "# It writes the readings into power_TODAYS-DATE.log file for further evaluation."
        echo -e "# To test if the power value change, increase the power of the machine by using the stress utility:"
        echo -e "# sudo apt install stress"
        echo -e "# sudo stress --cpu 32 --timeout 120   # this uses 32 workers (cores) and runs 120 second"
        echo -e " "
        echo -e "HELP on how to use this script:"
        echo -e " "
        echo -e "./read_power.sh {-h} {NUMBER_OF_ROUNDS SLEEP_TIME}"
        echo -e " "
        echo -e "Arguments:"
        echo -e "-h prints this HELP message"
        echo -e "NUMBER_OF_ROUNDS SLEEP_TIME"
        echo -e "Example with 10 rounds and sleeptime of 1s in between the rounds: ./read_power.sh 10 1"
        echo -e " "
        echo -e "*****************************************************************"
        echo -e " "
        exit 1
}

if [ $1 = "-h" ]; then
        HELPER_FUNCTION
fi

# STOP_HERE   # uncomment to stop execution here function
STOP_HERE(){
        # Used for testing up to here and then stop
        echo "Press any key to continue"
        while [ true ] ; do
                read -t 3 -n 1
                if [ $? = 0 ] ; then
                        exit
                else
                        echo -ne "waiting for the keypress\r"
                fi
        done
}

NUMBER_OF_ROUNDS=$1
SLEEP_TIME=$2

now=$(date +"%Y-%m-%d_%H-%M-%S_")
today=$(date +"%Y-%m-%d")

for ((N = 1; N <= $NUMBER_OF_ROUNDS; N++)) ;do (clear ; echo "Round Nr. "$N "of "$NUMBER_OF_ROUNDS ;sudo ipmitool dcmi power reading ; sleep $SLEEP_TIME) ;done | tee -a power_${HOSTNAME}_${today}.log