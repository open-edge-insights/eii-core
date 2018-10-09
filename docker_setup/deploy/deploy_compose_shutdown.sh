#!/bin/bash

# This scripts brings down the previous containers
# using docker-compose.yml

echo "Shutting down & Removing eta & dependent containers"
docker-compose down
