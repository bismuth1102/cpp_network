#!/bin/bash

identity_file="/Users/apple/Downloads/LightsailDefaultKey-us-east-1.pem"

line=$(cat upload.txt | awk 'NR==1')
IPstr=${line#*=}
IPaddr=(${IPstr//,/ })  # () means convert to a list
for node in ${IPaddr[@]}; do
	echo $node
	# scp -i $identity_file *.* DHTConfig ubuntu@$node:~/Distributed-System
	# scp -i $identity_file *.* DHTConfig latency throughput ubuntu@$node:~/Distributed-System
	scp -i $identity_file server.log latency throughput ubuntu@$node:~/Distributed-System
done

