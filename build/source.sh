#!/bin/bash

hostIP=$(ip route get 1 | awk '{print $7}'|head -1)
grep -Fxq "HOST_IP=" .env
if [ $? -eq 0 ] ; then
   sed -i "s/HOST_IP=/HOST_IP=$hostIP/g" .env
fi

grep -Fxq "ETCD_HOST=" .env
if [ $? -eq 0 ] ; then
   sed -i "s/ETCD_HOST=/ETCD_HOST=$hostIP/g" .env
fi

hostTimezone=`timedatectl | grep "zone" | awk '{print $3}'`
hostTimezone=`echo $hostTimezone`

# This will remove the HOST_TIME_ZONE entry if it exists and adds a new one with the right timezone
sed -i '/HOST_TIME_ZONE/d' .env && echo "HOST_TIME_ZONE=$hostTimezone" >> .env
