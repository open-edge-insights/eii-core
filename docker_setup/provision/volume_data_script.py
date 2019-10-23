
import yaml
import sys
import subprocess


def _execute_cmd(cmd):
    cmd_output = subprocess.check_output(cmd, shell=True)
    return cmd_output

def get_volume_info(filepath):
    with open(filepath) as f:
            docs = yaml.load_all(f, Loader=yaml.FullLoader)
            for doc in docs:
                for key, value in doc.items():
                    if key == "volumes":
                        list_of_volumes = list(value.keys())

    for vol in list_of_volumes:
        cmd = 'docker volume ls | grep ' + 'docker_setup_' + vol + ' | wc -l'
        if int(_execute_cmd(cmd)) == 1 :
            print("Cleaning volume:",vol)
            _execute_cmd('docker volume rm docker_setup_' + vol)

if __name__=="__main__":
    get_volume_info("../docker-compose.yml")