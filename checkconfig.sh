#!/bin/bash
input=$1
input2=$2
while IFS= read -r line
do
	if [[ $line != "" ]] && [[ ! "$line" =~ "^#" ]];then
		grep "$line" $input2 >/dev/null
		if [[ $? -ne 0 ]];then
			echo "$line"
			echo "> missing!"
		fi
	fi
done < "$input"
