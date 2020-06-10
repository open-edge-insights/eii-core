#!/usr/bin/python3

# Copyright (c) 2020 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import ruamel.yaml
import argparse
import os
import json
import subprocess
import sys
from jsonmerge import merge
import distutils.util as util


DOCKER_COMPOSE_PATH = './docker-compose.yml'
SCAN_DIR = ".."


def source_env(file):
    """Method to source an env file

    :param file: Path of env file
    :type file: str
    """
    try:
        with open(file, 'r') as fp:
            for line in fp.readlines():
                # Checking if line has = in env
                if "=" in line:
                    # Emulating sourcing an env
                    key, value = line.strip().split("=")
                    os.environ[key] = value
    except Exception as e:
        print("Exception occured {}".format(e))
        return


def json_parser(file):
    """Generate etcd config by parsing through
       individual app configs

    :param file: Path of json file
    :type file: str
    """
    # Fetching GlobalEnv config
    with open('./common_config.json', "rb") as infile:
        data = {}
        head = json.load(infile)
        data['/GlobalEnv/'] = head
        config_json = data

    # Fetching docker-compose.yml to retrieve required App dirs
    with open(file, 'r') as fp:
        data = ruamel.yaml.round_trip_load(fp, preserve_quotes=True)

    app_list = []
    # Replacing $PWD with relative path to EIS dir
    for key in data['services']:
        s = data['services'][key]['build']['context'].replace('$PWD/..', '..')
        app_list.append(s)

    # Removing duplicates & unwanted dirs from app list
    app_list = list(dict.fromkeys(app_list))
    app_list.remove(SCAN_DIR + '/common')

    eis_config_path = "./provision/config/eis_config.json"
    # Fetching and merging individual App configs
    for x in app_list:
        with open(x + '/config.json', "rb") as infile:
            data = {}
            head = json.load(infile)
            x = x.replace('.', '')
            # remove trailing '/'
            x = x.rstrip('/')
            data[x+'/config'] = head
            config_json = merge(config_json, data)

    # Writing consolidated json into desired location
    f = open(eis_config_path, "w")
    f.write(json.dumps(config_json, sort_keys=True, indent=4))
    print("Successfully created consolidated config json at {}".format(
        eis_config_path))


def csl_parser(app_list):
    """Generate CSL AppSpec by parsing through
       individual app specific AppSpecs

    :param app_list: List of services
    :type app_list: list
    """
    # Fetching the template json
    with open('./csl/csl_template.json', "rb") as infile:
        csl_template = json.load(infile)

    # json to store all publisher endpoints
    publisher_list = {
        "VideoAnalytics": {},
        "InfluxDBConnector": {}
    }

    # Updating publisher apps endpoints
    for app in app_list:
        with open(app + '/app_spec.json', "rb") as infile:
            jsonFile = json.load(infile)
            head = jsonFile["Module"]
            if head["Name"] == list(publisher_list.keys())[0] or\
               head["Name"] == list(publisher_list.keys())[1]:
                publisher_list[head["Name"]] = head["Endpoints"]

    # Creating deploy AppSpec from individual AppSpecs
    for app in app_list:
        with open(app + '/app_spec.json', "rb") as infile:
            jsonFile = json.load(infile)
            head = jsonFile["Module"]
            csl_template["Modules"].append(head)
            # Variable to handle no publishers
            publisher_present = False

            # Fetch links from all AppSpecs & update deploy AppSpec
            for endpoint in head["Endpoints"]:
                if "Link" in endpoint.keys():
                    csl_template["Links"].append({"Name": endpoint["Link"]})
                else:
                    # Iterate through available publisher list
                    for publisher in publisher_list.keys():
                        # Verify publiser doesn't try to fetch keys of itself
                        if publisher != head["Name"]:
                            if publisher_list[publisher] != "{}":
                                pub_list = publisher_list[publisher]
                                # Iterate through publisher's multiple
                                #  endpoints
                                for ep in pub_list:
                                    # Add Link if Publisher endpoint is of type
                                    # server
                                    if ep["Endtype"] == "server":
                                        endpoint["Link"] = ep["Link"]
                                        publisher_present = True
                            # Remove link if no publishers are available
                            if publisher_present is False:
                                head["Endpoints"].remove(endpoint)

            # Removing all duplicate links
            csl_template["Links"] = [dict(t) for t in {tuple(d.items())
                                     for d in csl_template["Links"]}]

            # Fetch links from all AppSpecs & update deploy AppSpec
            if "Ingress" in jsonFile.keys():
                csl_template["Ingresses"].append(jsonFile["Ingress"])

    # Creating consolidated json
    f = open('./csl/tmp_csl_app_spec.json', "w")
    f.write(json.dumps(csl_template, sort_keys=False, indent=4))

    # Sourcing required env from .env & provision/.env
    source_env("./.env")
    source_env("./provision/.env")

    # Generating module specs for all apps
    for app in app_list:
        app_name = app.rsplit("/", 1)[1]
        module_spec_path = "./csl/" + app_name + "_module_spec.json"
        app_path = app + "/module_spec.json"
        # Substituting sourced env in module specs
        cmnd = "envsubst < " + app_path + " > " + module_spec_path
        try:
            ret = subprocess.check_output(cmnd, shell=True)
        except subprocess.CalledProcessError as e:
            print("Subprocess error: {}, {}".format(e.returncode, e.output))
            sys.exit(1)

    # Substituting sourced env in AppSpec
    csl_config_path = "./csl/csl_app_spec.json"
    cmnd = "envsubst < ./csl/tmp_csl_app_spec.json > " + csl_config_path
    try:
        ret = subprocess.check_output(cmnd, shell=True)
    except subprocess.CalledProcessError as e:
        print("Subprocess error: {}, {}".format(e.returncode, e.output))
        sys.exit(1)

    # Removing generated temporary file
    os.remove("./csl/tmp_csl_app_spec.json")

    print("Successfully created consolidated AppSpec json at "
          "{}".format(csl_config_path))


def yaml_parser():
    """Yaml parser method.
    """

    # Fetching list of subdirectories
    print("Parsing through directory to fetch required services...")
    root_dir = os.getcwd() + '/../'
    dir_list = [f.name for f in os.scandir(root_dir) if
                (f.is_dir() or os.path.islink(f))]
    dir_list = sorted(dir_list)

    # Adding video folder manually since it's not a direct sub-directory
    if os.path.isdir(root_dir + 'common/video'):
        dir_list.insert(0, 'common/video')

    app_list = []
    csl_app_list = []
    for dir in dir_list:
        prefix_path = root_dir + dir
        # Append to app_list if dir has both docker-compose.yml and config.json
        if os.path.isfile(prefix_path + '/docker-compose.yml') and \
           os.path.isfile(prefix_path + '/config.json'):
            app_list.append(prefix_path)
        # Append to csl_app_list if dir has both app_spec.json and
        # module_spec.json
        if os.path.isfile(prefix_path + '/app_spec.json') and \
           os.path.isfile(prefix_path + '/module_spec.json'):
            csl_app_list.append(prefix_path)

    # Load the common docker-compose.yml
    yaml_files_dict = []
    with open('common-docker-compose.yml', 'r') as fp:
        data = ruamel.yaml.round_trip_load(fp, preserve_quotes=True)
        yaml_files_dict.append(data)

    # Load the required yaml files
    for k in app_list:
        with open(k + '/docker-compose.yml', 'r') as fp:
            data = ruamel.yaml.round_trip_load(fp, preserve_quotes=True)
            yaml_files_dict.append(data)

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

    # Fetching DEV_MODE from .env
    with open(".env") as f:
        for line in f:
            if line.startswith('DEV_MODE'):
                dev_mode = line.strip().split('=')[1]
                dev_mode = util.strtobool(dev_mode)
                break

    if dev_mode:
        for k, v in x.items():
            # Deleting the main secrets section
            if(k == "secrets"):
                del x[k]
            # Deleting secrets section for individual services
            elif(k == "services"):
                for _, service_dict in v.items():
                    for service_keys, _ in service_dict.items():
                        if(service_keys == "secrets"):
                            del service_dict[service_keys]

    with open(DOCKER_COMPOSE_PATH, 'w') as fp:
        ruamel.yaml.round_trip_dump(x, fp)

    str = "PROD"
    if dev_mode:
        str = "DEV"
    print("Successfully created docker-compose.yml"
          " file for {} mode".format(str))

    # Starting json parser
    json_parser(DOCKER_COMPOSE_PATH)

    # Starting csl json parser
    csl_parser(csl_app_list)


if __name__ == '__main__':

    # Parse command line arguments
    yaml_parser()
