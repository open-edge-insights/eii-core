#!/usr/bin/python3

"""
Copyright (c) 2020 Intel Corporation.
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import ruamel.yaml
import argparse
import os
import json
from jsonmerge import merge
from common.util.log import configure_logging, LOG_LEVELS

logger = configure_logging('INFO', __name__, True)
yaml = ruamel.yaml.YAML()

docker_compose_path = './build/docker-compose.yml'


def json_parser(file):
    """Generate etcd config by parsing through
       individual app configs
    """
    # Fetching GlobalEnv config
    with open('./eis_config.json', "rb") as infile:
        data = {}
        head = json.load(infile)
        data['/GlobalEnv/'] = head
        config_json = data

    # Fetching docker-compose.yml to retrieve required App dirs
    with open(file, 'r') as fp:
        data = yaml.load(fp)

    app_list = []
    # Replacing $PWD with relative path to EIS dir
    for key in data['services']:
        s = data['services'][key]['build']['context'].replace('$PWD/..', '.')
        app_list.append(s)

    # Removing duplicates & unwanted dirs from app list
    app_list = list(dict.fromkeys(app_list))
    app_list.remove('./common')

    # Fetching and merging individual App configs
    logger.info("Constructing consolidated config json...")
    for x in app_list:
        with open(x + '/config.json', "rb") as infile:
            data = {}
            head = json.load(infile)
            x = x.replace('.', '')
            data[x+'/config'] = head
            config_json = merge(config_json, data)

    # Writing consolidated json into desired location
    f = open("./build/provision/config/eis_config.json", "w")
    f.write(json.dumps(config_json, sort_keys=True, indent=4))


def yaml_parser():
    """Yaml parser method.
    """

    # Fetching list of subdirectories
    logger.info("Parsing through directory to fetch required services...")
    dir_list = [f.name for f in os.scandir('.') if f.is_dir()]

    # Adding openvino manually since it's not a direct sub-directory
    if os.path.isdir('common/openvino'):
        dir_list.append('common/openvino')

    app_list = []
    for dirs in dir_list:
        # Append to app_list if dir has both docker-compose.yml and config.json
        if os.path.isfile(dirs + '/docker-compose.yml') and \
             os.path.isfile(dirs + '/config.json'):
            app_list.append(dirs)

    # Load the common docker-compose.yml
    yaml_files_dict = []
    with open('common-docker-compose.yml', 'r') as fp:
        data = yaml.load(fp)
        yaml_files_dict.append(data)

    # Load the required yaml files
    for k in app_list:
        with open('./' + k + '/docker-compose.yml', 'r') as fp:
            data = yaml.load(fp)
            yaml_files_dict.append(data)

    logger.info("Constructing consolidated yaml file...")
    x = yaml_files_dict[0]
    for v in yaml_files_dict[1:]:
        for k in v:
            # Update the values from current compose
            # file to previous compose file
            for i in v[k]:
                if(k == "version"):
                    pass
                else:
                    x[k].update({i: v[k][i]})

    with open(docker_compose_path, 'w') as fp:
        yaml.dump(x, fp)

    # Starting json parser
    json_parser(docker_compose_path)


if __name__ == '__main__':

    # Parse command line arguments
    yaml_parser()
