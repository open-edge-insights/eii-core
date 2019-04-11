#!/bin/bash
if [ ! -n "$2" ]
then
  echo "Usage: sudo make push/pull DOCKER_REGISTRY=<Registry IP Address or Host Name>"
  echo "Please provide docker registry details"
  exit 1
fi

source ./init.sh
export IEI_DOCKER_REGISTRY="$2"

services+=('ia_provision')
services+=('ia_dist_libs')

if [ "$1" = "push" ]
then
  echo '** Pushing to registry ** ' 
  echo "Images: ${services[@]}"
  for image in "${services[@]}"
  	do
      echo "Tagging $image image..."
      docker tag $image:${IEI_VERSION} $IEI_DOCKER_REGISTRY/$image:${IEI_VERSION}
      echo "pushing $image image..."
      docker push $IEI_DOCKER_REGISTRY/$image:${IEI_VERSION}
  	done
else
  echo '** Pulling from registry ** ' 
  echo "Images: ${services[@]}"
  for image in "${services[@]}"
  	do
      echo "pulling $image image..."
      docker pull $IEI_DOCKER_REGISTRY/$image:${IEI_VERSION}
      echo "Tagging $image image..."
      docker tag $IEI_DOCKER_REGISTRY/$image:${IEI_VERSION} $image:${IEI_VERSION}
  	done
fi