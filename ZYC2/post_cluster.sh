#!/bin/bash

curl -X POST --header 'Content-Type: application/json' --header 'Accept: application/json' -d '[    {  "image_id": "33ffdea4-4462-48ac-9b32-77768ae95135",   "node_id": 1    },    {      "image_id": "NON_FPGA",      "node_ip":  "'"$1"'",      "node_id": 0    }  ]' 'http://10.12.0.132:8080/clusters?username=did&password=IamnewatZYC2&project_name=cf_Test_2'

#curl -X POST --header 'Content-Type: application/json' --header 'Accept: application/json' -d '[ \ 
#{ \ 
#"image_id": "33ffdea4-4462-48ac-9b32-77768ae95135", \ 
#"node_id": 1 \ 
#}, \ 
#{ \ 
#"image_id": "NON_FPGA", \ 
#"node_ip":  "10.12.2.100", \ 
#"node_id": 0 \ 
#} \ 
#]' 'http://10.12.0.132:8080/clusters?username=did&password=IamnewatZYC2&project_name=cf_Test_2'
 
 
 