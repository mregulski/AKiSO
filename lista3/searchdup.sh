#!/bin/bash
function help {
    echo "USAGE: /path/to/searchdup.sh sourcedir
    Lists duplicate (same size & md5) files in sourcedir"
}

if [ $# -eq 1 ]; then
    DIR_SOURCE="$1"
else 
    help
    exit
fi

find "$DIR_SOURCE" -not -empty -type f -printf "%-30s'\t\"%h/%f\"\n" | sort -rn -t$'\t' | uniq -w30 -D | cut -f 2 -d $'\t' | xargs md5sum  | uniq -w32 --all-repeated=separate 

