#!/bin/bash

# This scripts brings down the previous containers
# using docker-compose.yml

#source ./init.sh

echo "Shuting down & Removing eta & dependent containers"
docker-compose down
