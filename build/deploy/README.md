# EII provisioning and deployment via bundle generation

Perform the below steps to achieve provisioning & deployment through bundle generation

[Step 1 Pre-requisite](#step-1-pre-requisite)

[Step 2 Set up Docker Registry URL then Build and Push Images](#step-2-set-up-docker-registry-url-then-build-and-push-images)

[Step 3 Choosing the EII services to deploy](#step-3-choosing-the-eii-services-to-deploy)

[Step 4 Creating eii bundle for deployment](#step-4-creating-eii-bundle-for-deployment)


# Step 1 Pre-requisite

> **Pre-requisite**:
> Please follow the EII Pre-requisites before Provisioning.
> [EII Pre-requisites](../../README.md#eii-pre-requisites)

# Step 2 Set up Docker Registry URL then Build and Push Images

EII Deployment through bundle generation must be done using a docker registry.

Follow below steps:

* Please update docker registry url in DOCKER_REGISTRY variable in  [build/.env](../.env). Please use full registry URL with a traliling /

* Building EII images and pushing the same to docker registry.

      ```sh
      docker-compose -f docker-compose-build.yml build
      docker-compose -f docker-compose-push.yml push
      ```

# Step 3 Choosing the EII services to deploy

>Note: This deployment bundle generated can be used to provision and also to deploy EII stack

1. This software works only on python3+ version.
2. Please update the [config.json](./config.json) file

      ```
        {
            "docker_compose_file_version": "<docker_compose_file_version which is compose file version supported by deploying docker engine>",

            "exclude_services": [list of services which you want exclude for your deployement]
            "include_services": [list of services needed in final deployment docker-compose.yml]

        }
      ```

    ***Note***:
    >
    > 1. Please validate the json.

    > 2. Please ensure that you have updated the DOCKER_REGISTRY in [build/.env](../.env) file

# Step 4 Creating eii bundle for deployment

> **NOTE**: Before proceeding this step, please make sure, you have followed steps 1-3.

```
    # commands to be executed for bundle creation:
    $ cd build/deploy
    $ sudo python3 generate_eii_bundle.py -gb

    This will generate the .tar.gz which has all the required artifacts
```

***Note***:

    1. Default Values of Bundle Name is "eii_bundle.tar.gz" and the tag name is "eii_bundle"

    2. Using *-t* options you can give your custom tag name which will be used as same name for bundle generation.

For more help:

    $sudo python3 generate_eii_bundle.py


    usage: generate_eii_bundle.py [-h] [-f COMPOSE_FILE_PATH] [-t BUNDLE_TAG_NAME]

    EII Bundle Generator: This Utility helps to Generate the bundle to deploy EII.

    optional arguments:
        -h, --help            show this help message and exit
        -t BUNDLE_TAG_NAME    Tag Name used for Bundle Generation (default:eii_bundle)

Now this bundle can be used to deploy an eii on the instance. This bundle has all the required artifacts to start the eii
services.

```
