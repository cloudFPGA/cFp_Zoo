#!/bin/bash
#
# @brief  A simple script for writting a cFp.json file for a specific cFp_Vitis domain-role
#         The 1st argument is the domain-role
# @author DID
# @date Nov. 2020

echo "{" > cFp.json
echo "    \"version\": \"This cFp was created by cFBuild 0.6\"," >> cFp.json
echo "    \"cFpMOD\": \"FMKU60\"," >> cFp.json
echo "    \"cFpSRAtype\": \"Themisto\"," >> cFp.json
echo "    \"usedRoleDir\": \"$1/\"," >> cFp.json
echo "    \"usedRoleDir2\": \"2/\"," >> cFp.json
echo "    \"roleName1\": \"$1\"," >> cFp.json
echo "    \"roleName2\": \"2\"" >> cFp.json
echo "}" >> cFp.json



    
    
    
    
    
