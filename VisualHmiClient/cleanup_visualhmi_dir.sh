#!/bin/bash

if [ "$#" -lt 1 ]
then
    fileName=`basename "$0"`
    echo "Usage: sudo ./$fileName <full_path_to_visual_hmi_img_folder> Eg: sudo ./$fileName /root/saved_images\n"
    exit 1
fi

freq=300
while true
do
    sleep $freq # sleep for 5 mins
    echo "Deleting $folder every $freq secs....."
    rm -rf $1/*
done
