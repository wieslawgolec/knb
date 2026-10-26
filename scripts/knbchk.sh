#!/bin/sh
#
# knbchk.sh
#
# This is a script suitable for use in a crontab.  It checks to make sure
# your bot is running.  YOU NEED A SEPARATE CRON JOB FOR EACH BOT.  If your
# bot isn't found, it'll try to start it back up.
#
# You'll need to edit this script for your bot.
#
# To check for your bot every 10 minutes, put the following line in your
# crontab:
#    */10 * * * *   /home/user/knb/knbchk.sh
# And if you don't want to get email from crontab when it checks you bot,
# put the following in your crontab:
#    */10 * * * *   /home/user/knb/knbchk.sh >/dev/null 2>&1
# or simply use cron.sh script to add knbchk.sh to cron
#

# change this to the directory you run your bot from
KNBDIR="$HOME/knb"

# change this to the nick of your bot
NICK="knb"

# change this to then name of your bot's pidfile
PIDFILE="pid.$NICK"

# change this to the name of your bot's config file
CONF="conf"

# change this to the name of your bot's binary file 
KNB="knb"

########## you probably don't need to change anything below here ##########

cd $KNBDIR

# is there a pid file?
if [ -r $PIDFILE ]; then
    # there is a pid file -- reading pid from it
    MYPID=`cat $PIDFILE | head -n 1 | awk '{ print $1 }'`
else
    # there is no pid file, starting bot then
    echo "-+- Starting Knb bot.."
    ./$KNB $CONF >/dev/null 2>&1
    exit 0
fi

# searching for pid readed from pid file in process list  
TMPPID=`ps ux -U $USER | grep $MYPID | head -n 1 | awk '{ print $2 }'` 

# is bot allready running?
if [ ! "$TMPPID" = "$MYPID" ]; then
    # no, so we start it now (and removing pid file) 
    echo "-+- Starting Knb bot.."
    rm -fr $PIDFILE
    ./$KNB $CONF >/dev/null 2>&1
    exit 0
fi
 
# bot is allready running, so nothing to do ;>  
echo "-+- Knb bot allready running.." 
