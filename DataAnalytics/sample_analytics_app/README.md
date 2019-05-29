# Development Mode Operations and Custom Analytics Algo Creation Workflow:

This README will explain about development mode of operations and how to create a sample analytics Application using IEI internal libraries and its  workflow life cycle.

The **DEVELOPMENT MODE** is a mode of operation in IEI wherein user can create, modify and verify its custom written analytics application without provisioning any security certificates or without enabling container for the custom application. Moreover it enables user to experiment the video preprocessing algorithm that is hosted as part of trigger algorithms.

In order to enable development user need to set **DEV_MODE** variable to "true" and restart the container. Important point to note here that for running the IEI in Development mode user needn't run provisioning step. Additionally one needn't alter any security specific parameter as
it won't be utilized in development Mode.

The videoAnalytics container supports plugging new analytic algorithm. In case the user requirement doesn't comply with video_analytics way of functioning then user can follow this approach.


# App WorkFlow Introduction:

The following steps will describe about the process of developing a bare metal custom analytics appalication and eventually altering it to be part of IEI container cluster with enablement of security.

Typically a user will start developing the algorithm in a bare metal way without having any security setting and fine tune the application algorithm through multiple testing phases. In this phase it uses only IEI libraries to access data for inferencing.

Once the algoritm gets stabilized through various steps, the application need to run in IEI framework as an inherent container with all security policy imposed on it.

Here application is expected to run using OpenVino framework though it is not a mandatory requirement. For using any other third party frame work user is expected to install and configure the same.  

## Creating & Running APP in Development Mode(Bare Metal)

1. Follow Build & Installation process from [IEI's README](../../docker_setup/README.md)

2. Install OpenVino:
    Download the full package for OpenVINO toolkit for Linux version "2019 R1.0.1" from the official website (https://software.intel.com/en-us/openvino-toolkit/choose-download/free-download-linux) and extract it inside IEdgeInsights/DataAnalytics/VideoAnalytics. Post this step a directory named `l_openvino_toolkit_xxxxx/` will be present inside VideoAnalytics directory.

    > **NOTE**: Make sure there is always one `l_openvino_toolkit_xxxxx/` folder under IEdgeInsights/DataAnalytics/
    > VideoAnalytics folder as we are adding `l_openvino_toolkit_*` into Dockerfile which could result in
    > build failure of VideoAnalytics container if there are multiple openvino sdk's in there especially the old ones

    Run the following script:
    ```
    cd DataAnalytics/VideoAnalytics/
    sudo ./install_openvino.sh
    ```
    **This step can be skipped if other third party software is intended to be used for developing the custom algorithm.**

3. Install python dependencies and set environment variable:
    ```
    sudo ./install.sh <systemp_ip>
    source ./setenv.sh <systemp_ip>
    ```

    This completes seting up environment for creating Bare metal Analytics APP.

    ### Libraries For accessing IEI Meta-data :
    Below mentioned library call will enable user to access influx DB stored streams via influxDB subscription without using any certificates.
    ```
    from StreamSubLib.StreamSubLib import StreamSubLib
    self.sampleSubscriber = StreamSubLib()
    self.sampleSubscriber.init(dev_mode=true)
    ```
    The keyword parameter **dev_mode** is a **Boolean variable** which when set to **true** library wont use any certificates internally for communicating to IEI. In case IEI running with provisioned certificates, then **dev_mode** need to be set to **false** for library to work. For other streamSubLib APIs kindly refer IEI User Guide. Additionally theese library calls work as expected if the user is having super user previlege specfically when the dev_mode is set to TRUE.

    ### Libraries for accesing Image from ImageStore :
    The Image store client should be running in secured or non-secured mode,is decided during client creation itself. Following all other APIs are remains same as before. Hence the API looks as below:

    ```
    From ImageStore.internalClient.py.client import GrpcImageStoreInternalClient
    self.img_store = GrpcImageStoreInternalClient(dev_mode=True)
    ```
    User can pass **True** or **False** for the keyword argument ** dev_mode** while creating the client object.

4. Writting custom Bare-metal APP :

    This step is a complete user defined step wherein USer writes its Bare metal application using above said libraries of streamSubLib() and ImageStore(). A sample Application is developed to showcase the usage of above said library and parsing of the IEI message format. For e.g. one can execute the following comand to make the sample APP run in Development Mode or non-Development Mode by passing relevant **dev_mode** value.

    ```
    # For running the APP in non-secured(development) Mode
    python3.6 sample_analytics.py --dev-mode True

    # For running the app in secured Mode
    python3.6 sample_analytics.py --dev-mode False
    ```
    
    > **NOTE**: Pre-requisite when dev mode is set to true, run the following command :
    ```
    cp -rf /opt/intel/iei/grpc_int_ssl_secrets /etc/ssl/
    cp -rf ../../cert-tool/Certificates/ca /etc/ssl/
    cp -rf ../../cert-tool/Certificates/streamsublib /etc/ssl/
    ```
    
    
## Converting Bare-Metal APP to IEI framework based container :

    These steps help user to convert Bare Metal APP to a full fledged security based IEI container.  These steps can be executed once the user has tested its algorithm thoroughly.

1. Create a **Dockerfile** for the APP with all relevant packages and file additions based on the custom algo code. The entrypoint can be same bare-metal command which is used in Step-4.

2. Add a service entry in the [docker-compose.yml](../../docker_setup/docker-compose.yml) similar to ia_video_analytics container. The [docker_setup/README](../../docker_setup/README.md) explains it in detail how to add a new custom service. Note user must exclude any entry unrelated to custom algo he/she has written.

3. Restart the IEI service or **"sudo make build run"** to make YML file's chnage to take effect.
