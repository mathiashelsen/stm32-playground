#!/bin/bash

set i = 0
echo "Press any key to start a measurement, Ctrl+C to stop"
while read -n 1 c
do
    ./software > BFW10_$i.txt
    ((i++))
done
