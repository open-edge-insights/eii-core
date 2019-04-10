echo "copying dockerignore file.."
cp dockerignores/.dockerignore.dist_libs ../.dockerignore

echo "Building dist_libs container..."
docker build --build-arg UBUNTU_IMAGE_VERSION=18.04 \
             -f ../dist_libs/Dockerfile \
             -t ia/ia_dist_libs \
             ../

echo "Running ia/ia_dist_libs image to create dist_libs..."
docker run --rm \
            -v /opt/intel/iei/dist_libs:/iei/dist_libs \
            ia/ia_dist_libs


