#!/bin/bash

# Copyright (c) 2022 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

#
#  Starts/Restarts all eii containers
#
# Usage: ./eii_start.sh [timeout-in-seconds]
#
# Accept timeout (in seconds) as parameter for config manager agent to come up, 
# defaults to 60 seconds

RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
MAGENTA=$(tput setaf 5)
NC=$(tput sgr0)
BOLD=$(tput bold)
INFO=$(tput setaf 3)   # YELLOW (used for informative messages)

# Default timeout value (in seconds)
default_timeout=60
timeout="$default_timeout"
echo
if [ "$1" == "" ];then
    echo "Using default timeout value of $default_timeout seconds${NC}..."
else
    # Validate the user provided timeout value
    case $1 in
        ''|*[!0-9]*) echo "${INFO}WARNING: User provided timeout value is INVALID! Using default timeout value of $default_timeout seconds${NC}..." ;;
        *) timeout="$1" ;;
    esac
    if [ "$timeout" -lt "$default_timeout" ];then
        echo "${INFO}WARNING: User provided timeout value is too low! Using default timeout value of $default_timeout seconds${NC}..."
        timeout="$default_timeout"
    fi
fi
echo
echo "======================================"
echo "Bringing ${RED}down${NC} the eii containers..."
echo "======================================"

running_conts=$(docker ps -qf name=ia_*)
if ! [ "$running_conts" == "" ];then
    echo "${RED}Stopping${NC} containers..."
    docker stop $(docker ps -qf name=ia_*)
    if [ "$?" -ne 0 ];then
        echo "${RED}Some errors occured while stopping containers!${NC}"
    fi
fi
all_conts=$(docker ps -qaf name=ia_*)
if ! [ "$all_conts" == "" ];then
    echo "${RED}Removing${NC} containers..."
    docker rm $(docker ps -qaf name=ia_*)
    if [ "$?" -ne 0 ];then
        echo "${RED}Some errors occured while removing containers!${NC}"
    fi
fi
# Remove all dangling/unused docker networks
docker network prune -f
echo
echo "======================================"
echo "Bringing ${GREEN}up${NC} Config Manager Agent..."
echo "======================================"
docker-compose up -d ia_configmgr_agent
if [ "$?" -ne 0 ];then
    echo "${RED}Some errors occured while bringing up ia_configmgr_agent!${NC}"
fi
echo
echo "=================================================="
echo "${INFO}Waiting${NC} for Config Manager Agent to initialize..."
echo "=================================================="
c=0
while [ "$c" -lt "$timeout" ];do
    c=$((c + 1))
    # Check if Config Manager container is up
    docker ps | grep ia_configmgr_agent > /dev/null 2>&1
    if [ "$?" -ne 0 ];then
        sleep 1
        echo -ne "."
        continue
    fi
    # Look for the string "Provisioning is Done" in container logs
    docker logs ia_configmgr_agent 2>&1 | grep 'Provisioning is Done' > /dev/null 2>&1
    if [ "$?" -eq 0 ];then
        echo " "
        echo "=================================================="
        echo "${GREEN}Config Manager Agent has successfully initialized!${NC}"
        echo "Bringing ${GREEN}up${NC} the eii containers..."
        echo "=================================================="
        docker-compose up -d
        if [ "$?" -ne 0 ];then
            echo "${RED}Some errors occured while bringing up containers!${NC}"
        fi
        # Return success
        exit 0
    fi
    sleep 1
    echo -ne "."
done
# Return failure
echo
docker logs ia_configmgr_agent
echo "${RED}=========================================================${NC}"
echo "${RED}Config Manager Agent initialization has FAILED/timed out${NC}"
echo "${RED}=========================================================${NC}"
exit 1

