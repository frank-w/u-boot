#!/bin/bash
if [ "$#" -lt 1 ]; then echo "min 1 arg"; exit;fi;
if [[ -z "$2" ]]; then
	OLDBR=$(git rev-parse --abbrev-ref HEAD);
	NEWBR=$1;
	git branch -m $NEWBR
else
	OLDBR=$1;
	NEWBR=$2;
	git branch -m $OLDBR $NEWBR
fi
#echo "$OLDBR => $NEWBR"
git push origin :$OLDBR $NEWBR
git push origin -u $NEWBR
