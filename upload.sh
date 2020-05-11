#!/bin/bash

identity_file="LightsailDefaultKey-us-east-1.pem"

line=$(cat upload.txt | awk 'NR==1')
IPstr=${line#*=}
IPaddr=(${IPstr//,/ })  # () means convert to a list
for node in ${IPaddr[@]}; do
	echo $node
	# scp -i $identity_file *.* DHTConfig ubuntu@$node:~/Distributed-System
	# scp -i $identity_file *.* DHTConfig latency throughput ubuntu@$node:~/Distributed-System
	scp -i $identity_file upload.txt thread_pool.hpp client.cc server.cc hashlist.cc hashlist.h run.sh server_start.sh server.log DHTConfig latency throughput ubuntu@$node:~/Distributed-System
done

