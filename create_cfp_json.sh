#!/bin/bash
#
# @brief  A simple script for writting a cFp.json file for a specific cFp_Zoo domain-role
#         The 1st argument is the domain-role
# @author DID, DCO
# @date Nov. 2020

echo "{" > cFp.json
echo "    \"version\": \"This cFp was created by cFCreate 0.8\"," >> cFp.json
echo "    \"cFpMOD\": \"FMKU60\"," >> cFp.json
echo "    \"cFpSRAtype\": \"Themisto\"," >> cFp.json
echo "    \"usedRoleDir\": \"\"," >> cFp.json
echo "    \"usedRoleDir2\": \"to-be-defined\"," >> cFp.json
echo "    \"roleName1\": \"$1\"," >> cFp.json
echo "    \"roleName2\": \"unused\"," >> cFp.json
echo "    \"srat-conf\": {">> cFp.json
echo "        \"version\": 0.3,">> cFp.json
echo "        \"roles\": [">> cFp.json
echo "        {">> cFp.json
echo "                \"name\": \"$1\",">> cFp.json
echo "                \"path\": \"\"">> cFp.json
echo "            },">> cFp.json
echo "            {">> cFp.json
echo "                \"name\": \"1st-role\",">> cFp.json
echo "                \"path\": \"$1/\"">> cFp.json
echo "            }">> cFp.json
echo "        ],">> cFp.json
echo "        \"active_role\": \"1st-role\"">> cFp.json
echo "    }">> cFp.json
echo "}" >> cFp.json
    
    
    
    
    
