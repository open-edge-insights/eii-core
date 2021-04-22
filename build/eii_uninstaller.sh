#!/bin/bash -e

# Copyright (c) 2021 Intel Corporation.

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

# Fetching all the necessary variables from build/.env file
composeProjectName=$(cat .env | grep COMPOSE_PROJECT_NAME= | cut -d '=' -f2 | cat)
eiiInstallPath=$(cat .env | grep EII_INSTALL_PATH= | cut -d '=' -f2 | cat)

#--------------------------------------------------------------------
# removeEIIContainers
#
# Description:
#        Finds all the containers in docker then stop and removes them
# Args:
#        None
# Return:
#       None
# Usage:
#        removeEIIContainers
#--------------------------------------------------------------------
removeEIIContainers()
{

    x="docker ps -a --filter name=^(ia_|${composeProjectName,,}) --format {{.Names}}"
    $x > temp

    if [ ! -s ./temp ]
    then
        echo "There are no containers instance of EII"
        rm temp
        return
    fi

    containersId=$(while read line; do echo $line; done < temp)
    rm temp

    echo "==============================================="
    echo "Stopping and Removing all the EII containers..."
    echo "==============================================="
    docker stop $containersId && docker rm -f $containersId
}

#------------------------------------------------------------------
# removeVolumes
#
# Description:
#        Finds all the docker volumes then removes it
#        from the docker
# Args:
#        None
# Return:
#       None
# Usage:
#        removeVolumes
#------------------------------------------------------------------
removeVolumes()
{
    echo "==============================="
    echo "Removing EII Docker Volumes ..."
    echo "==============================="

    volumes=$(docker volume ls | awk {'print $2'} | grep "^\(${composeProjectName,,}\)" | cat)

    if [[ ! $volumes ]]
    then
        echo "There are no EII Volumes"
        return
    else
        docker volume rm -f $volumes
    fi
}


#------------------------------------------------------------------
# removeImages
#
# Description:
#        Finds all the docker Images then removes it
#        from the docker
# Args:
#        string : tag name
# Return:
#       None
# Usage:
#        removeImages "eii tag"
#------------------------------------------------------------------
removeImages()
{
    echo "==============================="
    echo "Removing the EII docker Images ..."
    echo "==============================="

    imageId=$(docker images --filter "reference=ia_*:2.4.1" --format '{{.Repository}}:{{.Tag}}')

    if [[ ! $imageId ]]
    then
        echo "There are no docker images for EII version $1"
        return
    else
        docker rmi -f $imageId
    fi
}


#------------------------------------------------------------------
# removeInstallDir
#
# Description:
#        Removes EII Install Directory
# Args:
#        None
# Return:
#       None
# Usage:
#        removeInstallDir
#------------------------------------------------------------------
removeInstallDir()
{
    echo "==============================="
    echo "Removing the EII Install Path"
    echo "==============================="

    if [[ -d $eiiInstallPath ]]
    then
        sudo rm -rf $eiiInstallPath
    fi
}


# -----------------------------------
# Init Function
#------------------------------------

usage="Usage: ./$(basename "$0") [-h] [-d]
 This script uninstalls previous EII version.
 Where:
    -h show the help
    -d triggers the deletion of docker images (by default it will not trigger)

 Example: 
  1) Deleting only EII Containers and Volumes
    $ ./eii_uninstaller.sh

  2) Deleting EII Containers, Volumes and Images
    $ export EII_VERSION=2.4
    $ ./eii_uninstaller.sh
    above example will delete EII containers, volumes and all the docker images having 2.4 version.
"

deleteImages=false
options=':hd'
while getopts $options option
do
    case "${option}" in
        h) echo "$usage";exit;;
        d) deleteImages=true;;
        :) printf "missing argument for -%s\n" "$OPTARG" >&2; echo "$usage" >&2; exit 1;;
        \?) printf "illegal option: -%s\n" "$OPTARG" >&2; echo "$usage" >&2; exit 1;;
    esac
done




echo "=========================================="
echo "Uninstalling EII Docker configurations"
echo "=========================================="


# 1. stop all running containers and remove them
removeEIIContainers

# 2. remove the docker volumes
removeVolumes

# 3. remove all the docker images
if [[ $deleteImages == true ]]
then
    if [[ ! "$EII_VERSION" ]]
    then
        echo "------------------------------------------------------------------------------"
        echo " Error: Unable to fetch \$EII_VERSION. User need to export EII_VERSION."
        echo "------------------------------------------------------------------------------"
        echo "$usage" >&2; exit 1
    else
        removeImages $EII_VERSION
    fi
fi

# 4. remove eii install directory
removeInstallDir
