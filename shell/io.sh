#!/bin/bash

#read 若只指定了一个变量，会将所有的值赋给变量直到遇到遇到文件EOF或者\n
# 若指定多个变量，shell将用空格作为分隔符，分别赋值给变量
echo  "First name:"
read firstname

echo "Last Name:"
read lastname1 lastname2

echo "${firstname} ${lastname1} ${lastname2}"
