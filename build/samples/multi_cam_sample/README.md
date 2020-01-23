# The sample config files can be referred to work with multiple cameras.

1. [docker-compose-multi-cam.yml](./docker-compose-multi-cam.yml) file can be referred to launch multiple VI and VA containers. In  the sample docker-compose.yml file 6 VI and VA containers are launched using IPC mode.

2. [etcd_preload_multicam.json](./etcd_preload_multicam.json) file can be referred to change the config for different VI and VA containers. In case multiple physical RTSP cameras are used then update the `RTSP_CAMERA_IP` env variable with the IP addresses of all the RTSP cameras as comma separated values in [build/.env](../../.env) with all the camera IPs.

* [VideoIngestion/README.md](../../../VideoIngestion/README.md) can be referred for various camara source ingestor configurations.

* [VideoAnalytics/README.md](../../../VideoAnalytics/README.md) can be referred for classifier configurations.