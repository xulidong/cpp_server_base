#!/bin/bash
#后台运行，将日志写入黑洞，即不写日志
nohup tar zcvf tst.tar.gz echo.sh > /dev/null &
