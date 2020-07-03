 set -a
 source ../.env
 source ../provision/.env
 set +a
kubectl delete -f video_analytics_deployment.yml -f video_ingestion_deployment.yml -f visualizer_deployment.yml -f webvisualizer_deployment.yml -f etcd_ui_deployment.yml
./create_deploy_yml.sh

sleep 10
kubectl apply -f deploy_yml/video_ingestion_deployment.yml
kubectl apply -f deploy_yml/video_analytics_deployment.yml
kubectl apply -f deploy_yml/visualizer_deployment.yml
kubectl apply -f deploy_yml/webvisualizer_deployment.yml
kubectl apply -f deploy_yml/etcd_ui_deployment.yml
