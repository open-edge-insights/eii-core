#!/bin/bash

source ./stop_eta_containers.sh

# docker rmi cmd removes the image
docker rmi ia_data_agent \
            ia_data_analytics \
            ia_nats_client \
            ia_video_ingestion