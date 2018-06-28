#!/bin/bash

source ./init.sh

echo "1. Starting DataAgent container..."
docker run -d --rm --net host --name ia_data_agent \
           -v $etaConfDir/DataAgent.conf:/ETA/DataAgent.conf:ro \
           -v $etaLogDir/DataAgent:/ETA/logDataAgent \
           ia/data_agent:1.0

echo "2. Starting DataAnalytics/Classifier container..."
docker run -d --rm --net host --name ia_data_analytics \
           -v $etaConfDir/kapacitor.conf:/etc/kapacitor/kapacitor.conf:ro \
           -v $etaDataDir/kapacitor:/var/lib/kapacitor \
           -v $etaConfDir/factory.json:/ETA/factory.json \
           -v $etaRootDir/$testFile:/ETA/yumei_trigger.avi \
           -v $etaDataDir/classifier_saved_images:/root/saved_images \
            ia/data_analytics:1.0
docker logs ia_data_analytics &> $etaLogDir/classifier.log

echo "2.1 Execing kapacitor daemon in Classifier container..."
docker exec -d ia_data_analytics kapacitor/kapacitord > $etaLogDir/kapacitord.log

echo "2.2 Sleeping for a while to bring up the kapacitor daemon..."
sleep 5

echo "2.3 Defining and enabling kapacitor task in Classifier container..."
docker exec ia_data_analytics enable_kapacitor_task.sh

echo "3. Starting NATS client container..."
docker run -d --rm --net host --name ia_nats_client \
           ia/nats_client:1.0

echo "4. Starting VideoIngestion container..."
docker run -d --rm --net host --name ia_video_ingestion \
           -v $etaRootDir/$testFile:/ETA/yumei_trigger.avi \
           -v $etaConfDir/factory.json:/ETA/factory.json \
           -v $etaLogDir/video_ingestion:/ETA/video_ingestion \
           ia/video_ingestion:1.0

docker logs ia_data_analytics &> $etaLogDir/classifier.log
docker logs ia_nats_client &> $etaLogDir/nats_client.log