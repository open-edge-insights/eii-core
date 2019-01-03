# Dist_Libs Module:

Dist_Libs provides external client sdk for DataBusAbstraction and ImageStore along with the sample client files.

# Build and Run the Docker and Create dist_libs.

* Go to ElephantTrunkArch/docker_setup and run the below command.
```
sudo ./create_client_dist_package.sh
```
* Also, added create_client_dist_package.sh as part of docker_setup/deploy/setup_eta.py

 **Note**:
 * Append PYTHONPATH with "/opt/intel/eta/dist_libs/:/opt/intel/eta/dist_libs/ImageStore/protobuff/py:/opt/intel/eta/dist_libs/DataBusAbstraction/py" in order to run the test scripts for the usage of dist_libs.
