#!/bin/bash
HELP="USAGE: ./webtrace.sh URL TIME"

if [ $# -ne 2 ]
then
    echo "$HELP"
    exit 1
fi

URL=$1
TIME=$2

SNAP_NAME=`echo "$URL" | sed -e "s|://|_|; s|[/?:=]|_|g"`
SNAP_OLD="/tmp/webtrace_snapshot_${SNAP_NAME}_old"
SNAP_CUR="/tmp/webtrace_snapshot_${SNAP_NAME}"

function cleanUp {
   # rm $SNAP_OLD
   # rm $SNAP_CUR
    exit
}
trap "cleanUp" SIGKILL SIGINT SIGTERM
while [ 1 ]; do

    if [ -a "$SNAP_CUR" ] 
    then
        cp -f "$SNAP_CUR" "$SNAP_OLD"
        curl -s -A "Mozilla/4.0" -L "$URL" > "$SNAP_CUR"
        if [ "$(md5sum $SNAP_CUR)" != "$(md5sum $SNAP_OLD)" ]
        #if [ "$(sha1sum "$SNAP_OLD" | awk '{ print $1 }' )" != "$(sha1sum "$SNAP_CUR" | awk '{ print $1}' )" ]
        then
          echo -ne "Site updated:\n$URL" |  gxmessage --buttons "" -timeout 5 \
              -nearmouse  -borderless -name "webtrace" -file - \
              &> /dev/null &
        fi
    else
        curl -s -A "Mozilla/4.0" -L "$URL" > "$SNAP_CUR"
    fi



    sleep $TIME
done

