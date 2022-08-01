#!/bin/bash
# NUS downloader bash script

if [ $# -lt 1 ]; then
    echo "usage: ./nus_download.sh title_id"
    exit -1
fi

DIR_BASE=titles
NUS_BASE=http://ccs.cdn.wup.shop.nintendo.net/ccs/download
TITLE_ID=$1
OUTPUT_DIR=$DIR_BASE/$1

[ ! -d $DIR_BASE ] && mkdir $DIR_BASE
# delete the directory if it exists already
[ -d $OUTPUT_DIR ] && rm -rf $OUTPUT_DIR
# create the output folder
mkdir $OUTPUT_DIR
# download the TMD and parse it
echo "Downloading TMD for $TITLE_ID..."
curl -s --user-agent "wii libnup/1.0" "$NUS_BASE/$TITLE_ID/tmd" -o $OUTPUT_DIR/tmd > /dev/null
echo "Downloading ticket..."
curl -s --user-agent "wii libnup/1.0" "$NUS_BASE/$TITLE_ID/cetk" -o $OUTPUT_DIR/cetk > /dev/null
TITLE_VERSION=0
PREV_INDEX=0
echo "Reading TMD..."
version_regex="Version: ([0-9]+)"
content_id_regex="ID: ([0-f]{8})"
content_idx_regex="Index: ([0-9]+)"
./tmdparse $OUTPUT_DIR/tmd | while TMD= read -r line; do
    if [[ $line =~ $version_regex ]]; then
        TITLE_VERSION=${BASH_REMATCH[1]}
        echo "Version: $TITLE_VERSION"
    fi
    if [[ $line =~ $content_idx_regex ]]; then
        PREV_INDEX=${BASH_REMATCH[1]}
    fi
    if [[ $line =~ $content_id_regex ]]; then
       # Save the matched text and stop
       echo "Downloading content ${PREV_INDEX} (${BASH_REMATCH[1]})..."
       curl -s --user-agent "wii libnup/1.0" "$NUS_BASE/$TITLE_ID/${BASH_REMATCH[1]}" -o $OUTPUT_DIR/${BASH_REMATCH[1]} > /dev/null
       echo "Decrypting content ${BASH_REMATCH[1]}..."
       ./tikdecrypt $OUTPUT_DIR/cetk $OUTPUT_DIR/${BASH_REMATCH[1]} ${PREV_INDEX}
    fi
done