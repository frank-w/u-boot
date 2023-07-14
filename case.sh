#!/bin/bash
case $1 in
	"bpi-r64")echo "r64";;
	"bpi-r3"|"bpi-r4")echo "r3/4";;
esac
