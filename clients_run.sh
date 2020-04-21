#!/bin/bash

identity_file="/Users/apple/Downloads/LightsailDefaultKey-us-east-1.pem"
counter=0

line=$(cat upload.txt | awk 'NR==1')
IPstr=${line#*=}
IPaddr=(${IPstr//,/ })  # () means convert to a list
for node in ${IPaddr[@]}; do
    ssh -i $identity_file ubuntu@$node "cd Distributed-System; ./client -c 2 -l 2"
    counter=$((counter+1))
done
echo "Run on all nodes"
