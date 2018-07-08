#!/bin/bash

echo "Stopping ETA containers..."
# docker stop cmd stops the container
docker stop ia_data_agent \
            ia_data_analytics \
            ia_nats_client \
            ia_video_ingestion