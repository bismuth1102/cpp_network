#!/bin/bash

identity_file="/Users/apple/Downloads/LightsailDefaultKey-us-east-1.pem"

line=$(cat upload.txt | awk 'NR==1')
IPstr=${line#*=}
IPaddr=(${IPstr//,/ })  # () means convert to a list
for node in ${IPaddr[@]}; do
	echo $node
	ssh -i $identity_file ubuntu@$node "cd Distributed-System; cat latency; cat throughput;"
done

