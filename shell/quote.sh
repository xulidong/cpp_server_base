#!/bin/bash
#双引号引号会解析其中的变量
echo "shell name: $SHELL"
#加上-e可以解析\n, 在shell中直接执行才可以
#加上-n,是回车不换行
echo "shell name:\n $SHELL"
echo -e "shell name:\n $SHELL"
#单引号直接输出
echo 'shell name: $SHELL'
#单反号执行命令
echo `ls -l`
