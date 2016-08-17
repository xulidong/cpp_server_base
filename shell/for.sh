#!/bin/bash
for i in 1 2 3 4 ; do
	echo $i
done

for ((i=1;i<=10;i++)) ; do
	echo $(expr $i \* 4);
done
