# Dist_Libs Module:

Dist_Libs provides external client sdk for OpcuaBusAbstraction and ImageStore along with the sample client files.

# Build and Run the Docker and Create dist_libs.

* Go to IEdgeInsights/docker_setup and run the below command.
```
sudo ./create_client_dist_package.sh
```
* Also, added create_client_dist_package.sh as part of docker_setup/deploy/setup_iei.py

 **Note**:
 * Append PYTHONPATH with "/opt/intel/iei/dist_libs/:/opt/intel/iei/dist_libs/ImageStore/protobuff/py:/opt/intel/iei/dist_libs/OpcuaBusAbstraction/py" in order to run the test scripts for the usage of dist_libs.
