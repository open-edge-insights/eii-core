# EIS - TutleCreek Deployment

Edge Insights Software (EIS) Deployment Bundle Generation for TurtleCreek Agent which will be deployed via Telit portal.This Utility helps you to generate deployment bundle for EIS Software deployment via Telit Portal

## Telit - TurtleCreek:

EIS Deployment will be done only TurtleCreek Installed & Provisioned Device via Telit portal. 

1. For TurtleCreek Agent installtion & provisioning and for Telit User Registration in Development portal by referring below Repo

    
     [https://gitlab.devtools.intel.com/Indu/IEdgeInsights/turtlecreek](https://gitlab.devtools.intel.com/Indu/IEdgeInsights/turtlecreek).


## Deployement Bundle Generation

1. This software works only on python3+ version.
2. Please update the **config.json** file

      ```
        { 
            "docker_compose_file_version": "<docker_compose_file_version which is compose file version supported by deploying docker engine>",
            
            "exclude_services": [list of services which you want exclude for your deployement]
            "include_services": [list of services needed in final deployment docker-compose.yml]

        }        
      ```
    ***Note***: Please validate the json.
    Also Please ensure that you have updated the DOCKER_REGISTRY in [docker_setup/.env](../.env) file

3. Follow the below commands for EIS bundle Generation.
    ```
    sudo python3.6 generate_eis_bundle.py
    ```
4. EIS Bundle **eis_installer.tar.gz** will be generated under the same directory.

5. Follow the TurtleCreek Repo on How to deploy this bundle with Telit Portal

**Note** :

    Default Values of Bundle Name is "eis_installer.tar.gz" and the tag name is "eis_installer"

    Using *-t* options you can give your custom tag name which will be used as same name for bundle generation.

    Default values for docker-compose yml path is "../docker-compose.yml"

    using "-f" you can give any docker-compose yml file path. But make sure that you have provisioned that already.


For more help:
        ```
            sudo python3.6 generate_eis_bundle.py
        ```


    usage: generate_eis_bundle.py [-h] [-f COMPOSE_FILE_PATH] [-t BUNDLE_TAG_NAME]

    EIS Bundle Generator: This Utility helps to Generate the bundle to deploy EIS.

    optional arguments:
        -h, --help            show this help message and exit
        -f COMPOSE_FILE_PATH  Docker Compose File path for EIS Deployment (default: ../docker-compose.yml)
        -t BUNDLE_TAG_NAME    Tag Name used for Bundle Generation (default:eis_installer)

    While deploying EIS Software Stack via Telit-TurtleCreek. Visualizer UI will not pop up because of display not attached to docker container of Visualizer.

    To Overcome. goto IEdgeInsights/docker_setup 
    -->  Stop the running ia_visualizer container with it's name or container id using "docker ps" command.
         ```sh
         docker rm -f ia_visualizer
         ```

    -->  Start the ia_visualizer again. 
        ```sh
        docker-compose up ia_visualizer
        ```
        