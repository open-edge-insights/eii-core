#!/bin/bash
source .env
if [ "$1" = "registry" ]
then
    if [ ! -n "$2" ]
    then
        echo "Usage: sudo make dist_libs_registry DOCKER_REGISTRY=<Registry IP Address or Host Name>"
        echo "Please provide docker registry details"
        exit 1
    fi
    export IEI_DOCKER_REGISTRY="$2" 
    echo "Pulling dist_libs image..."
    docker pull $IEI_DOCKER_REGISTRY/ia_dist_libs:${IEI_VERSION}
    docker tag $IEI_DOCKER_REGISTRY/ia_dist_libs:${IEI_VERSION} ia_dist_libs:${IEI_VERSION}
else
    echo "copying dockerignore file.."
    cp dockerignores/.dockerignore.dist_libs ../.dockerignore
    echo "Building dist_libs container..."
    docker build --build-arg UBUNTU_IMAGE_VERSION=18.04 \
             -f ../dist_libs/Dockerfile \
             -t ia_dist_libs:${IEI_VERSION} \
             ../
fi

echo "Running ia/ia_dist_libs image to create dist_libs..."
docker run --rm \
            -v /opt/intel/iei/dist_libs:/iei/dist_libs \
            ia_dist_libs:${IEI_VERSION}
