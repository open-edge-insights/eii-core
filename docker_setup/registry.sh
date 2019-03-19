
if [ ! -n "$2" ]
then
  echo "Usage: sudo make push/pull DOCKER_REGISTRY=<Registry IP Address or Host Name>"
  echo "Please provide docker registry details"
  exit 1
fi

export IEI_DOCKER_REGISTRY="$2" 
images=(logrotate data_agent imagestore data_analytics factoryctrl_app video_ingestion telegraf)
 
if [ "$1" = "push" ]
then
  echo '** Pushing to registry ** ' 
  echo "Images: ${images[@]}"
  for image in "${images[@]}"
  	do
      echo "Tagging $image image..."
      docker tag ia/$image:1.0 $IEI_DOCKER_REGISTRY/ia/$image:1.0
      echo "pushing $image image..."
      docker push $IEI_DOCKER_REGISTRY/ia/$image:1.0
  	done
else
  echo '** Pulling from registry ** ' 
  echo "Images: ${images[@]}"
  for image in "${images[@]}"
  	do
      echo "pulling $image image..."
      docker pull $IEI_DOCKER_REGISTRY/ia/$image:1.0
      echo "Tagging $image image..."
      docker tag $IEI_DOCKER_REGISTRY/ia/$image:1.0 ia/$image:1.0 
  	done
fi