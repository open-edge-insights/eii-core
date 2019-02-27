#!/bin/bash
source ./setenv.sh
mkdir -p $PWD/deploy/docker_images
cd $PWD/deploy/docker_images
curDir=`pwd`

function SaveImage()
{
  text="ia/$1:$2.."
  (( len=40-${#text} ))

  echo -n $text
  for i in $(seq 1 $len); do
    echo -n '.'
  done


  docker save -o "$1-$2.tar" "ia/$1:$2"

  echo "Done"
}


echo "Saving all the docker images to $curDir folder..."

# saving all docker images of iei along with its dependencies

SaveImage logrotate              $IEI_VERSION
SaveImage gobase                 $IEI_VERSION
SaveImage pybase                 $IEI_VERSION
SaveImage gopybase               $IEI_VERSION
SaveImage data_agent             $IEI_VERSION
SaveImage data_analytics         $IEI_VERSION
SaveImage video_ingestion        $IEI_VERSION
SaveImage telegraf               $IEI_VERSION
SaveImage factoryctrl_app        $IEI_VERSION
SaveImage provision              $IEI_VERSION
SaveImage imagestore             $IEI_VERSION


