import yaml
import json
import sys
import os

SERVICES = 'services'
VIDEO_INGESTION = 'ia_video_ingestion'
VIDEO_ANALYTICS = 'ia_video_analytics'
CONTAINER_NAME = 'container_name'
HOSTNAME = 'hostname'
ENVIRONMENT = 'environment'
APPNAME = 'AppName'
PUBTOPICS = 'PubTopics'
SUBTOPICS = 'SubTopics'
ZMQ_IPC = 'zmq_ipc,${SOCKET_DIR}/'
CAMERA_STREAM = 'camera1_stream_cfg'
CAMERA_STREAM_RESULT = 'camera1_stream_results_cfg'
VA_CONFIG = {
    "name": "pcb_classifier",
    "queue_size": 10,
    "max_workers": 1,
    "ref_img": "./VideoAnalytics/classifiers/ref_pcbdemo/ref.png",
    "ref_config_roi": "./VideoAnalytics/classifiers/ref_pcbdemo/roi_2.json",
    "model_xml": "./VideoAnalytics/classifiers/ref_pcbdemo/model_2.xml",
    "model_bin": "./VideoAnalytics/classifiers/ref_pcbdemo/model_2.bin",
    "device": "CPU"
    }
VI_INGESTOR_CONFIG = {
            "ingestor": {
                            "video_src": "./test_videos/pcb_d2000.avi",
                            "encoding": {
                                        "type": "jpg",
                                        "level": 100
                            },
                            "loop_video": "true",
                            "poll_interval": 0.2
                        },
            "filter": {
                            "name": "pcb_filter",
                            "queue_size": 10,
                            "max_workers": 1,
                            "training_mode": "false",
                            "n_total_px": 300000,
                            "n_left_px": 1000,
                            "n_right_px": 1000
                       }}


class generate_multiple_instance():
    def __init__(self, path, instances):
        self.docker_setup = path
        self.no_of_instance = int(instances)
        self.docker_compose_file = path + "/docker_setup/docker-compose.yml"
        self.sample_docker_compose_file = "sample_vi_va_docker-compose.yml"
        self.etcd_pre_load_file = path + \
            "/docker_setup/provision/config/etcd_pre_load.json"
        self.etcd_pre_load_sample_file = "etcd_pre_load.json"
        self.fps_config = path + "/tools/FpsCalculator/config.json"
        self.fps_config_sample = "fps_config.json"

    def generate_multiple_vi_va(self):
        '''Function to generate multiple instance of the VI
           and VA in the docker-compose file
        '''
        with open(self.docker_compose_file) as docker_compose:
            yaml_config = yaml.safe_load(docker_compose)

        vi_template = yaml_config[SERVICES][VIDEO_INGESTION]
        va_template = yaml_config[SERVICES][VIDEO_ANALYTICS]
        for instance in range(2, self.no_of_instance+1):
            vi_service_name = VIDEO_INGESTION+str(instance)
            va_service_name = VIDEO_ANALYTICS+str(instance)
            yaml_config[SERVICES][vi_service_name] = vi_template
            yaml_config[SERVICES][va_service_name] = va_template

        yaml.Dumper.ignore_aliases = lambda *args: True

        with open(self.sample_docker_compose_file, 'w') \
                as docker_compose_sample:
            yaml.dump(yaml_config, docker_compose_sample,
                      default_flow_style=False)

    def modify_multiple_vi_va(self):
        '''Function to modify the required fields in the multiple VI
           and VA instance created in generate_multiple_instance function
        '''
        with open(self.sample_docker_compose_file) as docker_compose_sample:
            yaml_config = yaml.safe_load(docker_compose_sample)

        for instance in range(2, self.no_of_instance+1):
            vi_service_name = VIDEO_INGESTION+str(instance)
            va_service_name = VIDEO_ANALYTICS+str(instance)

            yaml_config[SERVICES][vi_service_name][CONTAINER_NAME] = \
                vi_service_name
            yaml_config[SERVICES][vi_service_name][HOSTNAME] = vi_service_name
            yaml_config[SERVICES][vi_service_name][ENVIRONMENT][APPNAME] = \
                'VideoIngestion' + str(instance)
            yaml_config[SERVICES][vi_service_name][ENVIRONMENT][PUBTOPICS] = \
                'camera' + str(instance) + '_stream'
            del(yaml_config[SERVICES][vi_service_name]
                [ENVIRONMENT][CAMERA_STREAM])
            yaml_config[SERVICES][vi_service_name][ENVIRONMENT][
                'camera' + str(instance) + '_stream_cfg'] = ZMQ_IPC

            yaml_config[SERVICES][va_service_name][CONTAINER_NAME] = \
                va_service_name
            yaml_config[SERVICES][va_service_name][HOSTNAME] = va_service_name
            yaml_config[SERVICES][va_service_name][ENVIRONMENT][APPNAME] = \
                'VideoAnalytics' + str(instance)
            yaml_config[SERVICES][va_service_name][ENVIRONMENT][SUBTOPICS] = \
                'VideoIngestion/camera' + str(instance)+'_stream'
            yaml_config[SERVICES][va_service_name][ENVIRONMENT][PUBTOPICS] = \
                'camera' + str(instance)+'_stream'
            del(yaml_config[SERVICES][va_service_name]
                [ENVIRONMENT][CAMERA_STREAM])
            del(yaml_config[SERVICES][va_service_name]
                [ENVIRONMENT][CAMERA_STREAM_RESULT])
            yaml_config[SERVICES][va_service_name][ENVIRONMENT][
                'camera' + str(instance) + '_stream_cfg'] = ZMQ_IPC
            yaml_config[SERVICES][va_service_name][ENVIRONMENT][
                'camera' + str(instance) + '_stream_results_cfg'] = ZMQ_IPC

        with open(self.sample_docker_compose_file, 'w') \
                as docker_compose_sample:
            yaml.dump(yaml_config, docker_compose_sample,
                      default_flow_style=False)

    def modify_etcd_config(self):
        '''Function to modify the etcd pre load config based on the
           sample docker-compose file
        '''
        with open(self.etcd_pre_load_file) as etcd_file:
            etcd_data = json.load(etcd_file)

        for instance in range(2, self.no_of_instance+1):
            VI_key_name = '/VideoIngestion' + str(instance) + '/config'
            VA_key_name = '/VideoAnalytics' + str(instance) + '/config'
            etcd_data[VI_key_name] = VI_INGESTOR_CONFIG
            etcd_data[VA_key_name] = VA_CONFIG

        with open(self.etcd_pre_load_sample_file, 'w') as etcd_sample_file:
            json.dump(etcd_data, etcd_sample_file, indent=4)

    def modify_fps_config(self):
        '''Function to modify the fps config based on the
           sample docker-compose file
        '''
        with open(self.fps_config) as fps_file:
            fps_config = json.load(fps_file)

        fps_config['camera1_stream_results_cfg'] = 'zmq_ipc,/opt/intel/eis/sockets'
        for instance in range(2, self.no_of_instance+1):
            fps_element = 'VideoAnalytics' + \
                str(instance) + '/camera'+str(instance) + '_stream_results'
            key_name = 'camera' + str(instance) + '_stream_results_cfg'
            fps_config[SUBTOPICS].append(fps_element)
            fps_config[key_name] = 'zmq_ipc,/opt/intel/eis/sockets'

        with open(self.fps_config_sample, 'w') as fps_sample_config:
            json.dump(fps_config, fps_sample_config, indent=4)


def clean():
    '''Function to delete the generated files
    '''
    if os.path.exists('sample_vi_va_docker-compose.yml'):
        os.remove('sample_vi_va_docker-compose.yml')

    if os.path.exists('etcd_pre_load.json'):
        os.remove('etcd_pre_load.json')

    if os.path.exists('fps_config.json'):
        os.remove('fps_config.json')


if __name__ == '__main__':
    with open('config.json') as json_file:
        data = json.load(json_file)

    try:
        if sys.argv[1] == 'clean':
            clean()
        else:
            gmi = generate_multiple_instance(data['eis_path'], sys.argv[1])
            gmi.generate_multiple_vi_va()
            gmi.modify_multiple_vi_va()
            gmi.modify_etcd_config()
            gmi.modify_fps_config()
    except IndexError:
        print("Please provide the number of copy as argument")
