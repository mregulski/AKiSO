function help {
    echo "USAGE: /path/to/vcp.sh [-h] FILE TARGET
    cp wrapper for copying large files. Copies FILE to TARGET using cp.
    Shows progress and ETA.
    -h : show this message"
}

function size_format {
    numfmt --to=iec-i --suffix=B --format="%.2f" 
}

if [ $1 = "-h" -o $# -lt 2 ]
then 
    help
    exit
fi

INTERVAL=1
SOURCE=$1
TARGET=$2

# get user confirmation
read -p "copy $1 -> ${2}?[y/n]" -r -n 1
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]
then
    exit 1
fi

#echo "PID: $$"
cp "$SOURCE" "$TARGET" &
BGPID=$!
sleep 1  # wait for cp to start 

SOURCE_SIZE=$(stat -c %s "$SOURCE")
CUR_SIZE=1
AVG_SPEED=0
TIME=0
while kill -0 $BGPID &>/dev/null; do
    ## get the numbers
    OLD_SIZE=$CUR_SIZE
    CUR_SIZE=$(stat -c %s "$TARGET")
    PERCENTAGE=$(echo "scale=2;($CUR_SIZE/$SOURCE_SIZE)*100" | bc)
    DELTA=$(echo "$CUR_SIZE-$OLD_SIZE" | bc)
    
    CUR_SPEED=$(echo "$DELTA/$INTERVAL" | bc)
    AVG_SPEED=$(echo "($AVG_SPEED*$TIME+$CUR_SPEED)/($TIME+$INTERVAL)" | bc)
    
    # stderr>/dev/null because division by 0 can happen at very low speeds.
    TIME_LEFT=$(echo "($SOURCE_SIZE-$CUR_SIZE)/$AVG_SPEED" | bc 2>/dev/null)
    
    ## print {TARGET/SOURCE}% of {SOURCE} {SPEED}
    echo -ne "\r\e[K\e[38;2;1m${PERCENTAGE}% $(tput sgr0)  $(echo $CUR_SIZE | size_format)/$(echo $SOURCE_SIZE | size_format) @ $(echo $AVG_SPEED | size_format)/s ETA: ${TIME_LEFT}s"
    TIME=$(echo "$TIME+$INTERVAL" | bc)
    sleep $INTERVAL
done
if [ "$(stat -c  %s $TARGET)" -eq "$(stat -c %s $SOURCE)" ]
then
    echo -ne "\r\e[K\e[38;2;1m100% $(tput sgr0) $(stat -c %s $TARGET | size_format)/$(stat -c %s $SOURCE)"
    echo -e "\nFile copied successfully"
else
    echo -e "\nError copying file: target size not same as source size"
fi
