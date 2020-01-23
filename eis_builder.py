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
import xml.etree.ElementTree as ET
from jsonmerge import merge
from common.util.log import configure_logging, LOG_LEVELS

logger = configure_logging('INFO', __name__, True)
yaml = ruamel.yaml.YAML()

docker_compose_path = './build/docker-compose.yml'

def parse_args():
    """Parse command line arguments.
    """
    ap = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    ap.add_argument('-m', '--manifest', default="../.repo/manifests/default.xml",
                    help='Available usecases(Video, TimeSeries, DiscoveryCreek). Default is set to all')

    return ap.parse_args()

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
            x = x.replace('.','')
            data[x+'/config'] = head
            config_json = merge(config_json, data)

    # Writing consolidated json into desired location
    f = open("./build/provision/config/eis_config.json", "w")
    f.write(json.dumps(config_json, sort_keys=True, indent=4))


def yaml_parser(manifest):
    """Yaml parser method.
    """

    # Parsing the XML file to fetch required services
    logger.info("Parsing XML file to fetch required services...")
    tree = ET.parse(manifest)
    root = tree.getroot()
    app_list = []
    for child in root:
        if child.tag == 'project':
            if child.attrib['path'] == 'IEdgeInsights':
                pass
            else:
                s = child.attrib['path']
                AppName = s.split("IEdgeInsights/",1)[1]
                app_list.append(AppName)

    # Load the common docker-compose.yml
    yaml_files_dict = []
    with open('common-docker-compose.yml', 'r') as fp:
        data = yaml.load(fp)
        yaml_files_dict.append(data)

    #Load the required yaml files
    for k in app_list:
        with open('./' + k + '/docker-compose.yml', 'r') as fp:
            data = yaml.load(fp)
            yaml_files_dict.append(data)

    logger.info("Constructing consolidated yaml file...")
    x = yaml_files_dict[0]
    for v in yaml_files_dict[1:]:
        for k in v:
            # Update the values from current compose file to previous compose file
            for i in v[k]:
                if(k == "version"):
                    pass
                else:
                    x[k].update({i:v[k][i]})

    with open(docker_compose_path, 'w') as fp:
        yaml.dump(x, fp)
    
    # Starting json parser
    json_parser(docker_compose_path)

def main(args):
    """Main method.
    """
    logger.info("Manifest provided is {}".format(args.manifest))
    yaml_parser(args.manifest)

if __name__ == '__main__':

    # Parse command line arguments
    args = parse_args()
    main(args)
