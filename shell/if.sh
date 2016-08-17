#!/bin/bash
# eq/e:equal, n:no, l:less, g:greater
#-eq等于 -ne不等于 -lt小于 -gt大于 -le小于等于 -ge大于等于
a=$1
b=$2
if [ $a -lt $b ] ; then
	echo "$a < $b"
else
	echo "$a > $b"
fi
