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

        }        
      ```
    ***Note***: Please validate the json.
    Also Please ensure that you have updated the DOCKER_REGISTRY in [docker_setup/.env](../.env) file

3. Follow the below commands for Telit bundle Generation.
    ```
    sudo python3.6 generate_tc_bundle.py
    ```
4. Telit Bundle **eis_installer.tar.gz** will be generated under the same directory.

5. Follow the TurtleCreek Repo on How to deploy this bundle with Telit Portal

**Note** :

***TBD***

    Currently this patch is an intialVersion to achieve the functionality, More optimization on Arguments Parsing and Log Handling will be pushed in future patch.