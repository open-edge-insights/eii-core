#!/bin/bash

source ./setenv.sh
mkdir -p $PWD/deploy/docker_images
cd $PWD/deploy/docker_images
curDir=`pwd`


function LoadImage()
{
  text="Loading $1.."
  (( len=40-${#text} ))

  echo -n $text
  for i in $(seq 1 $len); do
    echo -n '.'
  done

  docker load -qi "$1-$2.tar"

}




echo "Loading all the docker images from $curDir folder..."

# loading all docker images of iei along with its dependencies
LoadImage logrotate          $IEI_VERSION
LoadImage gobase             $IEI_VERSION
LoadImage pybase             $IEI_VERSION
LoadImage gopybase           $IEI_VERSION
LoadImage data_agent         $IEI_VERSION
LoadImage data_analytics     $IEI_VERSION
LoadImage video_ingestion    $IEI_VERSION
LoadImage telegraf           $IEI_VERSION
LoadImage factoryctrl_app    $IEI_VERSION
LoadImage provision          $IEI_VERSION
LoadImage imagestore         $IEI_VERSION

