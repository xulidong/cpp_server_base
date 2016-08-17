#!/bin/bash
read key
case $key in
	[a-z]|[A-Z])
		echo "$key is letter"
		;;
	[0-9])
		echo "$key is number"
		;;
	*)
		echo "$key is digit"
		;;
esac
