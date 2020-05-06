#!/bin/bash -e
# Script to run to create K8 ymls with env variables substituted to actual values.
# This script on running, substitutes the environment variables present in ymls in k8s folder
# and creates yml files with actual values in deploy_yml folder.
 set -a
 source ../.env
 source ../provision/.env
 set +a

 sudo rm -rf deploy_yml
 mkdir deploy_yml

 envsubst < etcd_devmode.yml > deploy_yml/etcd_devmode.yml
 envsubst < etcd_prodmode.yml > deploy_yml/etcd_prodmode.yml
 envsubst < sample_publisher_subscriber.yml > deploy_yml/sample_publisher_subscriber.yml
 envsubst < ia_video_ingestion.yml > deploy_yml/ia_video_ingestion.yml
 envsubst < ia_video_analytics.yml > deploy_yml/ia_video_analytics.yml
 envsubst < ia_visualizer.yml > deploy_yml/ia_visualizer.yml
 envsubst < ia_web_visualizer.yml > deploy_yml/ia_web_visualizer.yml
