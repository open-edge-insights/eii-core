#!/bin/bash
source .env
DIST_LIBS_PATH=$IEI_INSTALL_PATH/dist_libs
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
    echo "Removing old $DIST_LIBS_PATH..."
    rm -rf $DIST_LIBS_PATH
    echo "Creating and setting permissions for $DIST_LIBS_PATH..."
    mkdir -p $DIST_LIBS_PATH
    chown -R $IEI_USER_NAME:$IEI_USER_NAME $DIST_LIBS_PATH
    echo "Copying dockerignore file.."
    cp dockerignores/.dockerignore.dist_libs ../.dockerignore
    echo "Building dist_libs container..."
    docker build --build-arg UBUNTU_IMAGE_VERSION=18.04 \
             -f ../dist_libs/Dockerfile \
             -t ia_dist_libs:${IEI_VERSION} \
             ../
fi

echo "Running ia/ia_dist_libs image to create dist_libs..."
docker run --rm \
            -v $DIST_LIBS_PATH:/iei/dist_libs \
            ia_dist_libs:${IEI_VERSION}

echo "Recursively setting permissions to $DIST_LIBS_PATH..."
chmod -R 777 $DIST_LIBS_PATH
