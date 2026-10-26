#!/bin/sh
#
# cron.sh

# change this to name of your bot's knbchk file
SCRIPT="knbchk.sh"

########## you probably don't need to change anything below here ##########

if [ ! "$HOME" = "" ]; then
    DEFKNBDIR="$HOME/knb/scripts/$SCRIPT"
fi

OK=false

while [ $OK = false ]; do
    if [ ! "$DEFKNBDIR" = "" ]; then  
	echo -n "-+- Where are you keeping $SCRIPT? [$DEFKNBDIR]: "
	read KNBDIR
	if [ "$KNBDIR" = "" ]; then
	    KNBDIR=$DEFKNBDIR
	fi
    else
	echo -n "-+- Where are you keeping $SCRIPT? "
	read KNBDIR
    fi
    if [ ! -f $KNBDIR ]; then
	echo "-+- File $KNBDIR doesn't exist!"
    else
	OK=true
    fi
done

echo -n "-+- Installing crontab.. "
echo "*/10 * * * * $KNBDIR >/dev/null 2>&1" > ~/cron-knb-tmp
crontab ~/cron-knb-tmp 2>/dev/null
rm -fr ~/cron-knb-tmp
echo " done."
