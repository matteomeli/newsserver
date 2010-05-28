#!/bin/bash

n="ansa" 

COUNTER=0
while [  $COUNTER -lt $1 ]; do
	#echo The counter is $COUNTER
	k="${n}$COUNTER"
	#echo $k
	./provider $k sport 10 1 &
	let COUNTER=COUNTER+1 
done
