echo "Building dist_libs container..."
docker build -f ../dist_libs/Dockerfile -t ia/ia_dist_libs ../

echo "Running ia/ia_dist_libs image to create dist_libs..."
docker run -v /opt/intel/eta/dist_libs:/eta/dist_libs ia/ia_dist_libs


