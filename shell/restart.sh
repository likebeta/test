#!/bin/bash

# yisilong 2014/1/3

if [ $# -lt 1 ]; then
	cnt=`ls -l gamesvrd* | grep -vc "\."`
else
	cnt=$1
fi

for (( i=1;i<${cnt};i++ ))
do
	rm gamesvrd${i}
done

chmod +x gamesvrd

echo "kill all gamesvrd"
ps aux | grep 'gamesvrd[1-9]*$' | awk '{print "kill -9 " $2}' | sh

sleep 1

for (( i=1;i<${cnt};i++ ))
do
	cp -a gamesvrd gamesvrd${i}
	echo "start gamesvrd${i}"
	./gamesvrd${i} &
done

