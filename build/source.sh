#!/bin/bash -e

hostIP=$(ip route get 1 | awk '{print $7}'|head -1)
sed -i '/HOST_IP/d' .env && echo "HOST_IP=$hostIP" >> .env
hostTimezone=`timedatectl | grep "zone" | awk '{print $3}'`
hostTimezone=`echo $hostTimezone`

# This will remove the HOST_TIME_ZONE entry if it exists and adds a new one with the right timezone
sed -i '/HOST_TIME_ZONE/d' .env && echo "HOST_TIME_ZONE=$hostTimezone" >> .env
