#!/bin/bash

source ./stop_eta_containers.sh

# docker rm cmd removes the container
docker rm ia_data_agent \
            ia_data_analytics \
            ia_nats_client \
            ia_video_ingestion