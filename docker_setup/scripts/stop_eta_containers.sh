#!/bin/bash

# docker stop cmd stops and removes the container as "--rm" switch was used
# to run the container
docker stop ia_data_agent \
            ia_data_analytics \
            ia_nats_client \
            ia_video_ingestion