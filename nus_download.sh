#!/bin/bash
# NUS downloader bash script
# for use with tmdparse/tikdecrypt

if [ $# -lt 1 ]; then
    echo "usage: ./nus_download.sh title_id [optional: version, or 'latest'] [optional: ticket path]"
    exit -1
fi

# set a version
VERSION=
if [ $# -gt 1 ]; then
    if [ $2 != "latest" ]; then
        VERSION=.$2
    fi
fi

DIR_BASE=titles
NUS_BASE=http://ccs.cdn.wup.shop.nintendo.net/ccs/download
TITLE_ID=$1
OUTPUT_DIR=$DIR_BASE/$1

TICKET_PATH=$OUTPUT_DIR/cetk
if [ $# -gt 2 ]; then
    TICKET_PATH="$3"
fi

[ ! -d $DIR_BASE ] && mkdir $DIR_BASE
# delete the directory if it exists already
[ -d $OUTPUT_DIR ] && rm -rf $OUTPUT_DIR
# create the output folder
mkdir $OUTPUT_DIR
# download the TMD and parse it
echo "Downloading TMD for $TITLE_ID$VERSION..."
HTTP_CODE=$(curl -s --write-out "%{http_code}" --user-agent "wii libnup/1.0" "$NUS_BASE/$TITLE_ID/tmd$VERSION" -o $OUTPUT_DIR/tmd)
if [[ ${HTTP_CODE} -ne 200 ]]; then
    echo "Failed to download TMD."
    exit 1
fi
# download a ticket if one wasn't specified
if [ $# -lt 3 ]; then
    echo "Downloading ticket..."
    HTTP_CODE=$(curl -s --write-out "%{http_code}" --user-agent "wii libnup/1.0" "$NUS_BASE/$TITLE_ID/cetk" -o $OUTPUT_DIR/cetk)
    if [[ ${HTTP_CODE} -ne 200 ]]; then
        echo "Failed to download ticket."
        exit 1
    fi
else
    echo "Using ticket at $TICKET_PATH"
fi
TITLE_VERSION=0
PREV_INDEX=0
PREV_CID=0
echo "Reading TMD..."
version_regex="Version: ([0-9]+)"
content_id_regex="ID: ([0-f]{8})"
content_idx_regex="Index: ([0-9]+)"
content_sha_regex="SHA1: ([0-f]{40})"
./tmdparse $OUTPUT_DIR/tmd | while TMD= read -r line; do
    if [[ $line =~ $version_regex ]]; then
        # get the version from the TMD and rename the downloaded TMD
        TITLE_VERSION=${BASH_REMATCH[1]}
        echo "Version: $TITLE_VERSION"
        # we can't move, since the file is in use
        cp $OUTPUT_DIR/tmd $OUTPUT_DIR/$TITLE_VERSION.tmd
    fi
    if [[ $line =~ $content_idx_regex ]]; then
        # take note of the content index
        PREV_INDEX=${BASH_REMATCH[1]}
    fi
    if [[ $line =~ $content_id_regex ]]; then
       # download and decrypt the content using the content ID
       PREV_CID=${BASH_REMATCH[1]}
       echo "Downloading content ${PREV_INDEX} (${PREV_CID})..."
       curl -s --user-agent "wii libnup/1.0" "$NUS_BASE/$TITLE_ID/$PREV_CID" -o $OUTPUT_DIR/$PREV_CID > /dev/null
       echo "Decrypting content $PREV_CID..."
       ./tikdecrypt $TICKET_PATH $OUTPUT_DIR/$PREV_CID $OUTPUT_DIR/tmd ${PREV_INDEX} --noHash
       # delete the encrypted content since... there isn't really a need for it
       rm $OUTPUT_DIR/$PREV_CID
    fi
    if [[ $line =~ $content_sha_regex ]]; then
       # Save the matched text and stop
       echo "${BASH_REMATCH[1]} *$OUTPUT_DIR/$PREV_CID.app" >> $OUTPUT_DIR/sha1
    fi
done
# delete the original TMD 
rm $OUTPUT_DIR/tmd
# validate the checksums
# note: due to padding issues, some will return failed despite decrypting correctly
# if at least 1 is OK then we're safe :)
echo "Validating SHA1 checksums..."
sha1sum -c $OUTPUT_DIR/sha1
rm $OUTPUT_DIR/sha1