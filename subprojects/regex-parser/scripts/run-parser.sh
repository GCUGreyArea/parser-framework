#!/bin/bash

cmd="${PWD}/build/parser -r rules -l logs"
val=0
while [ true ]
do
    val=val+1
    echo "Doing ${val}"
    eval ${cmd}
done