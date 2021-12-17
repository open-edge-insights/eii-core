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
"""script to generate consolidated docker-compose.yml, eii_config.json
   and AppSpec json
"""
import argparse
import os
import json
import subprocess
import sys
import re
import copy
import distutils.util as util
from jsonmerge import merge
from jsonschema import validate
import ruamel.yaml
import io
from ruamel.yaml.comments import CommentedMap
import shutil

DOCKER_COMPOSE_PATH = './docker-compose.yml'
DOCKER_COMPOSE_BUILD_PATH = './docker-compose-build.yml'
DOCKER_COMPOSE_PUSH_PATH = './docker-compose-push.yml'
SCAN_DIR = ".."
dev_mode = False
# List to store list of AppNames
appname_list = []
# Initializing multi instance related variables
num_multi_instances = 0
used_ports_dict = {"send_ports": [], "recv_ports": [], "srvc_ports": []}
override_apps_list, override_helm_apps_list = \
    ([] for _ in range(2))
dev_override_list, app_list, helm_app_list = \
    ([] for _ in range(3))
used_ports_dict = {"send_ports":[], "recv_ports":[], "srvc_ports":[] }

def source_env(file):
    """Method to source an env file

    :param file: Path of env file
    :type file: str
    """
    try:
        with open(file, 'r') as env_path:
            for line in env_path.readlines():
                # Checking if line has = in env
                if "=" in line:
                    # Emulating sourcing an env
                    key, value = line.strip().split("=", 1)
                    os.environ[key] = value
    except Exception as err:
        print("Exception occured {}".format(err))
        return


def env_subst(input_file, output_file, input_type=None):
    """Method to apply the env variable values to the passing file

    :param input_file: Path of input_file for apply env file or cmd
    :type input_file: str
    :param output_file: Path of output_file to store the applied env values.
    :type output_file: str
    :param input_type: cmd value instead of input file path
    :type input_type: str
    """
    try:
        if input_type == 'cmd':
            cmd = input_file
        else:
            cmd = subprocess.run(["cat", input_file], stdout=subprocess.PIPE,
                                 check=False)

        with open(output_file, "w") as outfile:
            subprocess.run(["envsubst"], input=cmd.stdout,
                           stdout=outfile, check=False)
    except subprocess.CalledProcessError as err:
        print("Subprocess error: {}, {}".format(err.returncode, err.output))
        sys.exit(1)


def generate_valid_filename(path):
    """Method to generate valid file name

    :param path: file name
    :type path: str
    :return: valid file path
    :rtype: str
    """
    # List of special characters
    escape_list = ['[', '@', '!', '#', '$', '%', '^', '&', '*', '(',
                   ')', '<', '>', '?', '|', '}', '{', '~', ':', ']', ' ']
    for k in escape_list:
        path = str(path).strip().replace(k, '\\' + k)
    return path


def increment_rtsp_port(appname, config, i):
    """Method to increment rtsp port number in
       gstreamer pipeline if required

    :param appname: app name
    :type appname: str
    :param config: app config
    :type config: dict
    :param i: multi instance value
    :type i: int
    :return: updated app config
    :rtype: dict
    """
    # Increment port number for RTSP stream pipelines if
    # increment_rtsp_port is set to true
    if appname == 'VideoIngestion' and \
       builder_cfg['increment_rtsp_port'] is True:
        if 'rtspsrc' in config['config']['ingestor']['pipeline']:
            port = config['config']['ingestor']['pipeline'].\
                split(":", 2)[2].split("/")[0]
            port = re.findall(r'\d+', port)
            port = port[-1]
            new_port = str(int(port) + i)
            config['config']['ingestor']['pipeline'] =\
                config['config']['ingestor']['pipeline'].\
                replace(port, new_port)

    return config


def create_multi_instance_interfaces(config, default_config, i, bm_apps_list):
    """Generate multi instance interfaces for a given config

    :param config: app config of the multi_instance subdirectory file config.json
    :type config: json
    :param default_config: app config of the parent directory file config.json
    :type config: json
    :param i: number of instance
    :type i: int
    :param bm_apps_list: multi instance required apps
    :type bm_apps_list: list
    :return: updated app config
    :rtype: dict
    """
    # Iterating through interfaces key, value pairs
    for k, v in config["interfaces"].items():
        for x in config["interfaces"][k]:
            # Updating Name section of all interfaces
            y = default_config["interfaces"][k][0]
            if "Name" in x.keys():
                if (y["Name"] == x["Name"]):
                    x["Name"] = x["Name"] + str(i+1)
            # Updating Topics section of all interfaces
            if "Topics" in x.keys():
                if (y["Topics"] == x["Topics"]):
                    for index, topic in enumerate(x["Topics"]):
                        if bool(re.search(r'\d', x["Topics"][index])):
                           x["Topics"][index] = re.sub(r'\d+', str(i+1),
                                                    x["Topics"][index])
                        else:
                            x["Topics"][index] = x["Topics"][index] +\
                                                str(i+1)
            # Updating AllowedClients section of all interfaces
            if "AllowedClients" in x.keys():
                if (x["AllowedClients"] == y["AllowedClients"]):
                    for index, topic in enumerate(x["AllowedClients"]):
                        if x["AllowedClients"][index] in bm_apps_list:
                            # Update AllowedClients AppName if they are not
                            # in subscriber list
                            if x["AllowedClients"][index] not in subscriber_list:
                                if bool(re.search(r'\d', x["AllowedClients"
                                                       ][index])):
                                    x["AllowedClients"][index] = \
                                        re.sub(r'\d+', str(i+1),
                                        x["AllowedClients"][index])
                                else:
                                    x["AllowedClients"][index] = \
                                        x["AllowedClients"][index] + str(i+1)
            # Updating PublisherAppName section of all interfaces
            if "PublisherAppName" in x.keys():
                if x["PublisherAppName"] in bm_apps_list:
                    if (x["PublisherAppName"] == y["PublisherAppName"]):
                        if bool(re.search(r'\d', x["PublisherAppName"])):
                            x["PublisherAppName"] = re.sub(r'\d+', str(i+1),
                                                       x["PublisherAppName"])
                        else:
                            x["PublisherAppName"] = x["PublisherAppName"] +\
                                                      str(i+1)
            # Updating ServerAppName section of all interfaces
            if "ServerAppName" in x.keys():
                if x["ServerAppName"] in bm_apps_list:
                    if (x["ServerAppName"] == y["ServerAppName"]):
                        if bool(re.search(r'\d', x["ServerAppName"])):
                            x["ServerAppName"] = re.sub(r'\d+', str(i+1),
                                                    x["ServerAppName"])
                        else:
                            x["ServerAppName"] = x["ServerAppName"] +\
                                                      str(i+1)
            # Updating Type section of all interfaces
            if "Type" in x.keys():
                if "tcp" in x["Type"]:
                    if (x["EndPoint"] == y["EndPoint"]):
                        # If tcp mode, increment port numbers
                        port = x["EndPoint"].split(':')[1]
                        new_port = str(int(port) + (i))
                        x["EndPoint"] = x["EndPoint"].replace(port, new_port)
                        # Update IP if it is not localhost/127.0.0.1
                        ip = x["EndPoint"].split(":")[0]
                        new_ip = None
                        if ip != "localhost" and ip != "127.0.0.1" and not re.search(r'\d', ip):
                            new_ip = ip + str(i+1)
                        if new_ip is not None:
                            x["EndPoint"] = x["EndPoint"].replace(ip, new_ip)
    return config


def create_multi_subscribe_interface(head, temp, client_type):
    """Generate multi subscribe config for Subscribers
       or Clients if their appname is mentioned in
       subscriber_list

    :param head: app config
    :type head: dict
    :param temp: copy of app config
    :type temp: dict
    :param client_type: Subscribers/Clients
    :type client_type: str
    :return: updated app config
    :rtype: dict
    """
    # Loop over all available subscribers/clients
    for j in range(len(head["interfaces"][client_type])):
        client = head["interfaces"][client_type][j]
        # For every subscriber/client, create multiple instances
        # & update required parameters
        for i in range(num_multi_instances):
            cli_instance = copy.deepcopy(client)
            # Updating name of subscriber/client instance
            cli_instance["Name"] = client["Name"] + str(i+1)
            # Updating PublisherAppName of subscriber instance
            if "PublisherAppName" in client.keys():
                cli_instance["PublisherAppName"] = \
                    client["PublisherAppName"] + str(i+1)
            # Updating ServerAppName of client instance
            if "ServerAppName" in client.keys():
                cli_instance["ServerAppName"] = \
                    client["ServerAppName"] + str(i+1)
            # Updating Topics of subscriber instance
            if "Topics" in client.keys():
                if bool(re.search(r'\d', client["Topics"][0])):
                    cli_instance["Topics"][0] = re.sub(r'\d+', str(i+1),
                                                       client["Topics"][0])
                else:
                    cli_instance["Topics"][0] = client["Topics"][0] + str(i+1)
            # Updating EndPoint if mode is tcp
            if ":" in client["EndPoint"]:
                ip = client["EndPoint"].split(":")[0]
                new_ip = None
                if ip != "localhost" and ip != "127.0.0.1" and not re.search(r'\d', ip):
                    new_ip = ip + str(i+1)
                port = client["EndPoint"].split(":")[1]
                new_port = str(int(port)+i)
                cli_instance["EndPoint"] = \
                    client["EndPoint"].replace(port, new_port)
                if new_ip is not None:
                    cli_instance["EndPoint"] = \
                        cli_instance["EndPoint"].replace(ip, new_ip)
            # Appending client multi instances
            temp["interfaces"][client_type].append(cli_instance)
        # Remove existing instances
        temp["interfaces"][client_type].remove(client)
    return temp


def fetch_appname(app_path):
    """Fetches AppName from docker-compose.yml file
       in the directory specified in app_path

    :param app_path: path of the app
    :type app_path: str
    :return: AppName of service if found, None for common/video
             empty string otherwise
    :rtype: str
    """
    if os.path.isdir(app_path):
        with open(app_path+'/docker-compose.yml', 'rb') as infile:
            data = ruamel.yaml.round_trip_load(infile,
                                               preserve_quotes=True)
            # Iterate through the yaml file and
            # return AppName if found
            for x in data:
                if x == 'services':
                    for y in data[x]:
                        if "environment" in data[x][y]:
                            # If environment sections exists,
                            # raise Exception if AppName not found
                            if "AppName" not in data[x][y]["environment"]:
                                raise Exception("AppName not found")
                            return data[x][y]["environment"]["AppName"]
                        # For common/video case
                        return None
    return ""


def json_parser(app_list, args):
    """Generate etcd config by parsing through
       individual app configs

    :param app_list: list of apps
    :type app_list: list
    :param args: cli args
    :type args: argparse
    """

    # Fetching GlobalEnv config
    with open('./common_config.json', "rb") as infile:
        data = {}
        head = json.load(infile)
        data['/GlobalEnv/'] = head
        config_json = data

    # Removing duplicates from app list
    app_list = list(dict.fromkeys(app_list))

    # Creating benchmarking apps list to edit interfaces accordingly
    for x in override_apps_list:
        appname = x.split("/")[-2]
        # Updating app_list to exlude services with override directory
        for y in app_list:
            if ("/" + appname) in y:
                app_list.remove(y)
                app_list.append(x)

    # Fetching service names for creating multi instance
    bm_apps_list = []
    for x in app_list:
        bm_appname = x.split("/")[-1]
        if args.override_directory is not None:
            if args.override_directory in x:
                bm_appname = x.split("/")[-2]
        bm_apps_list.append(bm_appname)

    eii_config_path = "eii_config.json"
    for app_path in app_list:
        data = {}
        # Creating multi instance config if num_multi_instances > 1
        if num_multi_instances > 1:
            dirname = app_path.split("/")[-1]
            # Fetching AppName for all services
            app_name = fetch_appname(app_path)
            if args.override_directory is not None:
                if args.override_directory in app_path:
                    dirname = app_path.split("/")[-2]
            # Ignoring EtcdUI & common/video service to not create multi instance
            # TODO: Support AzureBridge multi instance creation if applicable
            if (app_name not in subscriber_list and
                dirname != "video" and
                dirname != "EtcdUI" and
                "AzureBridge" and "ConfigMgrAgent" not in app_path):
                for i in range(num_multi_instances):
                    # path to the multi_instance subdirectory
                    path = os.path.join(multi_inst_path , dirname + str(i+1))
                    # Open the config.json within the parent directory
                    with open(app_path + '/config.json' , "rb") as defile:
                        default_config = json.load(defile)  
                    # Open the config.json within the multi_instance subdirectory    
                    with open(path + '/config.json', "rb") as infile:
                        head = json.load(infile)
                        # Increments rtsp port number if required
                        head = increment_rtsp_port(app_name, head, i)
                        # Create multi instance interfaces
                        head = \
                            create_multi_instance_interfaces(head, default_config,
                                                             i,
                                                             bm_apps_list)
                        # merge config of multi instance config
                        if 'config' in head.keys():
                            data['/' + app_name + str(i+1) + '/config'] = \
                                head['config']
                        # merge interfaces of multi instance config
                        if 'interfaces' in head.keys():
                            data['/' + app_name + str(i+1) +
                                '/interfaces'] = \
                                head['interfaces']
                        # # merge multi instance generated json to eii config
                        config_json = merge(config_json, data)
                        # Writing the changes to the config.json file in the multi_instance subdirectory
                        with open(path + "/config.json", "w") as multi_instance_json_file:
                            multi_instance_json_file.write(json.dumps(head, sort_keys=True, indent=4))
            # This condition is to handle not creating multi instance for
            # subscriber services
            else:
                with open(app_path + '/config.json', "rb") as infile:
                    head = json.load(infile)
                    temp = copy.deepcopy(head)
                    # merge interfaces of multi instance config
                    if 'interfaces' in head.keys():
                        if "Subscribers" in head["interfaces"]:
                            # Generate multi subscribe interface
                            temp = \
                                create_multi_subscribe_interface(head, temp,
                                                                 "Subscribers")
                            data['/' + app_name + '/config'] = head['config']
                            data['/' + app_name + '/interfaces'] = \
                                temp['interfaces']
                            config_json = merge(config_json, data)
                        if "Clients" in head["interfaces"]:
                            # Generate multi client interface
                            temp = \
                                create_multi_subscribe_interface(head, temp,
                                                                 "Clients")
                            data['/' + app_name + '/config'] = head['config']
                            data['/' + app_name + '/interfaces'] = \
                                temp['interfaces']
                            config_json = merge(config_json, data)
                        # This is to handle empty interfaces cases like EtcdUI
                        data['/' + app_name + '/config'] = head['config']
                        data['/' + app_name + '/interfaces'] = \
                            temp['interfaces']
                        config_json = merge(config_json, data)
        # This condition is to handle the default non multi instance flow
        else:
            with open(app_path + '/config.json', "rb") as infile:
                # Merging config & interfaces from all services
                head = json.load(infile)
                # Fetching AppName for all services
                app_name = fetch_appname(app_path)
                if 'config' in head.keys():
                    data['/' + app_name + '/config'] = head['config']
                if 'interfaces' in head.keys():
                    data['/' + app_name + '/interfaces'] = head['interfaces']
                config_json = merge(config_json, data)

    temp = copy.deepcopy(config_json)
    # removing subscriber interface where publisher app is not present
    for key, value in config_json.items():
        if "Subscribers" in value:
            for subscriber in value["Subscribers"]:
                if subscriber["Type"] == "zmq_tcp":
                    # Ignoring ZmqBroker as PublisherAppName is "*" for subscriber
                    if subscriber["PublisherAppName"] != "*":
                        if("/" + subscriber["PublisherAppName"] + \
                            "/interfaces" not in config_json.keys()):
                            temp[key]["Subscribers"].remove(subscriber)
                            if not temp[key]["Subscribers"]:
                                del temp[key]["Subscribers"]

    # removing client interface where server app is not present
    for key, value in config_json.items():
        if "Clients" in value:
            for client in value["Clients"]:
                if client["Type"] == "zmq_tcp":
                    if("/" + client["ServerAppName"] + \
                        "/interfaces" not in config_json.keys()):
                        temp[key]["Clients"].remove(client)
                        if not temp[key]["Clients"]:
                            del temp[key]["Clients"]


    # Writing consolidated json into desired location
    with open(eii_config_path, "w") as json_file:
        json_file.write(json.dumps(temp, sort_keys=True, indent=4))


def get_available_port(curr_port, ports_dict):
    """Method to recursively check for available
       ports in provided ports_dict

    :param curr_port: port to be replaced
    :type curr_port: str
    :param ports_dict: dict to check available ports
    :type ports_dict: dict
    :return: next available port
    :rtype: str
    """
    # Check if port is not available, recursively check for
    # next available port
    if curr_port in ports_dict:
        return get_available_port(str(int(curr_port)+1), ports_dict)
    # If port is available, append it to non-available ports dict
    # and return
    else:
        ports_dict.append(curr_port)
        return curr_port


def helm_yaml_merger(app_list, args):
    """Method merges the values.yaml files of each eii
       modules and generates a consolidated values.yaml
       file. copy all templates files of each eii
       modules to ./helm-eii/eii-deploy/templates.

    :param app_list: List of services
    :type app_list: list
    :type args: argparse
    """
    pv_merged_yaml = ""
    pvc_merged_yaml = ""
    helm_values_dict = []
    value_keys = ["env", "config", "volumes"]
    helm_provision_values_dict = []
    ignore_template_copy = ["eii-pv.yaml", "eii-pvc.yaml"]
    helm_deploy_dir = "./helm-eii/eii-deploy/"
    helm_provision_dir = "./helm-eii/eii-provision"
    # Load the common values.yaml
    if os.path.isdir(helm_deploy_dir + "templates"):
        shutil.rmtree(helm_deploy_dir + "templates")
    os.mkdir(helm_deploy_dir + "templates")
    with open(helm_deploy_dir +"../common-values.yaml", 'r') as \
        values_file:
        data = ruamel.yaml.round_trip_load(values_file,
                                           preserve_quotes=True)
        helm_values_dict.append(data)

    # Read persistent volume yaml
    with open(helm_deploy_dir +"../common-eii-pv.yaml") as pv_yaml_file:
        data = pv_yaml_file.read()
        pv_merged_yaml = pv_merged_yaml + "---\n" + data

    # Read persistent volume claim yaml
    with open(helm_deploy_dir +"../common-eii-pvc.yaml") as pvc_yaml_file:
        data = pvc_yaml_file.read()
        pvc_merged_yaml = pvc_merged_yaml + "---\n" + data

    app_list.extend(override_helm_apps_list)
    for app_path in app_list:
        helm_app_path = os.path.join(app_path + "/helm")
        app_name = app_path.split("/")[-1]
        if args.override_directory is not None:
            if args.override_directory in helm_app_path:
                app_name = app_path.split("/")[-2]

        # Copying all templates files of each eii modules to
        # ./helm-eii/eii-deploy/templates.
        yaml_files = os.listdir(helm_app_path + "/templates")
        for yaml_file in yaml_files:
            helm_yaml_file = os.path.join(helm_app_path, "templates", yaml_file)
            if os.path.isfile(helm_yaml_file) and yaml_file \
                not in ignore_template_copy:
                shutil.copy(helm_yaml_file, helm_deploy_dir + "templates")

        # Load the required values.yaml files
        with open(helm_app_path + "/values.yaml", 'r') as values_file:
            data = ruamel.yaml.round_trip_load(values_file,
                                               preserve_quotes=True)

            helm_values_dict.append(data)


        # Reading persistent volume yaml if present
        if os.path.isfile(helm_app_path + "/templates/eii-pv.yaml"):
            with open(helm_app_path + "/templates/eii-pv.yaml") as pv_yaml_file:
                data = pv_yaml_file.read()
                pv_merged_yaml = pv_merged_yaml + "---\n" + data

        # Writing final persistent volume yaml file
        with open(helm_deploy_dir + "templates/eii-pv.yaml", 'w') as final_pv_yaml:
            final_pv_yaml.write(pv_merged_yaml)

        # Reading persistent volume claim yaml if present
        if os.path.isfile(helm_app_path + "/templates/eii-pvc.yaml"):
            with open(helm_app_path + "/templates/eii-pvc.yaml") as pvc_yaml_file:
                data = pvc_yaml_file.read()
                pvc_merged_yaml = pvc_merged_yaml + "---\n" + data

        # Writing final persistent volume claim yaml file
        with open(helm_deploy_dir + "templates/eii-pvc.yaml", 'w') as final_pvc_yaml:
            final_pvc_yaml.write(pvc_merged_yaml)


    # Updating values.yaml for provision
    with open(helm_provision_dir +"/values.yaml", 'r') as \
        values_file:
        data = ruamel.yaml.round_trip_load(values_file,
                                           preserve_quotes=True)
        helm_provision_values_dict.append(data)

    # helm_provision_dict contains the provisioning values for helm
    helm_provision_dict = helm_provision_values_dict[0]
    for var in helm_provision_dict:
        if var not in value_keys:
            if os.environ.get(var) is not None:
                # Updating outer variables in values yaml file for provision
                helm_provision_dict.update({var: os.getenv(var)})
        elif var == "env":
            for i in helm_provision_dict[var]:
                # Updating environment variables inside env key in values yaml file for provision
                if os.environ.get(i) is not None:
                    if i == "DEV_MODE" or i == "PROFILING_MODE":
                        helm_provision_dict[var].update({i: bool(util.strtobool(os.getenv(i)))})
                    else:
                        helm_provision_dict[var].update({i: os.getenv(i)})
    with open(helm_provision_dir +"/values.yaml", 'w') as value_file:
        ruamel.yaml.round_trip_dump(helm_provision_dict, value_file)



    # Updating final helm values dict
    helm_dict = helm_values_dict[0]
    for var in helm_dict:
        if var not in value_keys:
            if os.environ.get(var) is not None:
                # Updating outer variables in values yaml file for deploy
                helm_dict.update({var: os.getenv(var)})
        elif var == "env":
            for i in helm_dict[var]:
                # Updating environment variables inside env key in values yaml file for deploy
                if os.environ.get(i) is not None:
                    if i == "DEV_MODE" or i == "PROFILING_MODE":
                        helm_dict[var].update({i: bool(util.strtobool(os.getenv(i)))})
                    else:
                        helm_dict[var].update({i: os.getenv(i)})


    if num_multi_instances > 1:
        helm_dict.update({"num_video_instances": num_multi_instances})
    for var in helm_values_dict[1:]:
        for k in var:
            for i in var[k]:
                helm_dict[k].update({i: var[k][i]})

    # Writing consolidated values.yaml file to ./helm-eii/eii-deploy/ dir
    with open(helm_deploy_dir +"values.yaml", 'w') as value_file:
        ruamel.yaml.round_trip_dump(helm_dict, value_file)


def create_multi_instance_yml_dict(mi_data, data, i):
    """Method to generate boilerplate
       yamls

    :param mi_data: input yaml from multi_instance subdirectory
    :type mi_data: ordered dict
    :type data: ordered dict
    :param data: input yaml from parent directory
    :type data: ordered dict
    :param i: number of required instances
    :type i: int
    :return: updated yaml dict
    :rtype: ordered dict
    """
    # Fetching individual app names from override_apps_list
    bm_apps_list = []
    for x in override_apps_list:
        appname = x.split('/' + args.override_directory)[0].split('/')[-1]
        bm_apps_list.append(appname)

    # deep copy to avoid issues during iteration
    temp = copy.deepcopy(mi_data)
    for k, v in temp['services'].items():
        for k1 , _ in data['services'].items():
            k1 = re.sub(r'\d+', '', k1)
        if (k == k1):
            # Update yaml main key name
            k_new = re.sub(r'\d+', '', k) + str(i)
            temp['services'][k_new] = v
            del temp['services'][k]
        # Iterating throught the yaml to update
        # all keys & values
        for k2, v2 in v.items():
            x = data['services'][k1]
            # Update container name
            if k2 == 'container_name':
                if (x['container_name'] == v2):
                    v['container_name'] = re.sub(r'\d+', '', v2) + str(i)
            # Update hostname
            elif k2 == 'hostname':
                if (x['hostname'] == v2):
                    v['hostname'] = re.sub(r'\d+', '', v2) + str(i)
            # Update ports
            elif k2 == 'ports':
                if (x['ports'] == v2):
                    # Iterate through all ports
                    for j in list(v['ports']):
                        # Update new ports & remove
                        # existing ports to avoid duplication
                        port = j.split(':')[0]
                        new_port = str(int(port) + (i-1))
                        v['ports'].append(new_port+':'+new_port)
                        v['ports'].remove(port+':'+port)
            # Update all env key, value pairs of yml
            elif k2 == 'environment':
                for k4, v4 in list(v['environment'].items()):
                    # Update AppName
                    if k4 == 'AppName':
                        if (x['environment']['AppName'] == v4):
                            v['environment'][k4] = v4 + str(i)
                            app_name = v4
                            new_app_name = v4 + str(i)
            # Update the volumes section
            elif k2 == 'volumes':
                for v3 in v['volumes']:
                    if "./Certificates" in v3 and "rootca" not in v3:
                        index = v['volumes'].index(v3)
                        v['volumes'].remove(v3)
                        appname = (v3.split(":")[-1]).split("/")[-1]
                        new_appname = re.sub(r'\d+', '', appname)
                        v3 = v3.replace(appname, new_appname + str(i))
                        v['volumes'].insert(index, v3)

    return temp


def update_yml_dict(app_list, file_to_pick, dev_mode, args):
    """Method to consolidate yml dicts and generate the
       combined yml dict. Picks the yml file specified by
       file_to_pick in the directories mentioned in app_list

    :param app_list: list of apps
    :type app_list: list
    :param file_to_pick: yml file to be picked
    :type app_file_to_picklist: str
    :param dev_mode: dev mode var
    :type dev_mode: bool
    :param args: cli args var
    :type args: argparse
    :return: yaml dict
    :rtype: dict
    """

    # Load the common docker-compose.yml
    yaml_files_dict = []
    with open('common-docker-compose.yml', 'r') as docker_compose_file:
        data = ruamel.yaml.round_trip_load(docker_compose_file,
                                           preserve_quotes=True)
        yaml_files_dict.append(data)

    # Load the required yaml files
    app_list.extend(override_apps_list)
    for k in app_list:
        with open(k + '/' + file_to_pick, 'r') as docker_compose_file:
            data = ruamel.yaml.round_trip_load(docker_compose_file,
                                               preserve_quotes=True)

            # Fetching AppNames of all services
            appname = fetch_appname(k)
            if args.override_directory is not None:
                if args.override_directory in k:
                    appname = k.split("/")[-2]

            # Append valid AppNames to appname_list
            if (appname != "video" and appname != "common" and appname != None):
                appname_list.append(appname)

            # Create multi instance compose if num_multi_instances is > 1
            if num_multi_instances > 1:
                # Create single instance only for services in subscriber_list
                # and for corner case of common/video,
                # create multi instance otherwise
                # TODO: Support AzureBridge
                # multi instance creation if applicable
                if (appname not in subscriber_list.keys() and
                        appname != "video" and
                        appname != "common" and
                        appname != "AzureBridge" and
                        appname != "ConfigMgrAgent" and
                        appname != None):
                    appname_list.remove(appname)
                    for i in range(num_multi_instances):
                        # Path to the multi_instance subdirectory
                        path_app = os.path.join(multi_inst_path, appname + str(i+1))
                        # Create subdirectories VideoIngestion1, VideoIngestion2, ...if not created
                        if (not(os.path.isdir(path_app))):
                            os.mkdir(path_app)
                        # Copy default config.json and docker-compose.yml files to the subdirectory if not present    
                        if (not(os.path.isfile(path_app + "/docker-compose.yml") and 
                            os.path.isfile (path_app +  "/config.json"))):
                            origin = os.path.join (k , "docker-compose.yml")
                            dest = os.path.join (path_app , "docker-compose.yml")
                            shutil.copy(origin, dest)
                            origin = os.path.join(k, "config.json")
                            dest = os.path.join (path_app , "config.json")
                            shutil.copy(origin, dest)

                        if dev_mode:
                            # Copy docker-compose-dev.override.yml file if not already present
                            if (not(os.path.isfile(path_app + '/docker-compose-dev.override.yml'))):
                                origin = os.path.join (k , "docker-compose-dev.override.yml")
                                dest = os.path.join (path_app , "docker-compose-dev.override.yml")
                                shutil.copy(origin, dest)

                        # Update appname_list for current service
                        appname_list.append(appname + str(i+1))
                        # Open the docker-compose.yml file within the subdirectory
                        with open(path_app + '/' + file_to_pick, 'r') as docker_compose_file:
                            mi_data = ruamel.yaml.round_trip_load(docker_compose_file,
                                                                  preserve_quotes = True)
                            data_two = create_multi_instance_yml_dict(mi_data, data, i+1)
                            # Write the changes to the existing docker-compose.yml file within multi_instance directory
                            with open(path_app + "/" + file_to_pick, 'w') as docker_compose_file:
                                ruamel.yaml.round_trip_dump(data_two, docker_compose_file)
                            yaml_files_dict.append(data_two)    

                # To create single instance for subscriber services
                else:
                    yaml_files_dict.append(data)
            # To create single instance
            else:
                yaml_files_dict.append(data)

    # Update SERVICES of config-agent with AppNames of all services
    config_agent_service = ""
    for var in yaml_files_dict[1:]:
        if "ia_configmgr_agent" in var["services"]:
            config_agent_service = \
                var["services"]["ia_configmgr_agent"]["environment"]["SERVICES"]
            for appname in appname_list:
                if appname != "ConfigMgrAgent":
                    config_agent_service = config_agent_service + appname + ','
            var["services"]["ia_configmgr_agent"]["environment"]["SERVICES"] = \
                config_agent_service.rsplit(',', 1)[0]

    # Updating final yaml dict
    yaml_dict = yaml_files_dict[0]
    for var in yaml_files_dict[1:]:
        for k in var:
            # Update the values from current compose
            # file to previous compose file
            for i in var[k]:
                if(k == "version"):
                    pass
                else:
                    yaml_dict[k].update({i: var[k][i]})

    return yaml_dict


def create_docker_compose_override(app_list, dev_mode, args, run_exclude_images):
    """Consolidated docker-compose.override.yml

    :param app_list: List of docker-compose-dev.override.yml file
    :type app_list: list
    :param dev_mode: Dev mode variable
    :type dev_mode: bool
    :param args: cli arguments
    :type args: argparse
    """
    if args.override_directory is None:
        # Load the required override files
        override_data = update_yml_dict(app_list,
                                        'docker-compose-dev.override.yml',
                                        dev_mode,
                                        args)
        temp = copy.deepcopy(override_data)
        for k, v in override_data.items():
            if(k == "services"):
                for service, service_dict in v.items():
                    if(service in run_exclude_images):
                        del temp["services"][service]
            elif (k != "version"):
                del temp[k]
        if temp["services"]:
            with open("./docker-compose.override.yml", 'w') as fp:
                ruamel.yaml.round_trip_dump(temp, fp)


def get_service_dependency_tree(yml_dict, run_exclude_images):
    """Method to generate a dict of services and services it depends on 

    :param yml_dict: dict generated from docker-compose yml
    :type yml_dict: dict
    :param run_exclude_images: List of name of dependency services which
                               are excluded from run
    :type run_exclude_images: array
    :return dep_tree: the dependency tree dict
    :rtype: dict
    """
    dep_tree = {}
    for k, v in yml_dict.items():
        if(k == "services"):
            for service, service_dict in v.items():
                if service not in run_exclude_images:
                    dep_tree[service] = []
                for service_key, service_vals in list(service_dict.items()):
                    if service_key == "depends_on":
                        dep_tree[service] = service_vals
    return dep_tree


def update_dependency_service(service, dep_tree, req_services):
    """Method to append the provided dict with nested dependencies of the 
       provided service
    :param service: Name of the service of which the dependencies need to be 
                    fetched
    :type service: str
    :param dep_tree: The dependency tree dict returned by 
                     get_service_dependency_tree() method
    :type dep_tree: dict
    :param req_services: The list where the dependencies names to be appended
    :type req_services: dict
    """ 
    req_services[service] = "1"
    if service in dep_tree:
        deps = dep_tree[service]
        for i in range(len(deps)):
            update_dependency_service(deps[i], dep_tree, req_services)
        

def get_required_services_list(dep_tree, run_exclude_images):
    """Method to get the list of required services along with the services
       it depends on
    :param dep_tree: The dependency tree dict returned by 
                     get_service_dependency_tree() method
    :type dep_tree: dict
    :param run_exclude_images: List of name of dependency services which
                               are excluded from run
    :type run_exclude_images: array
    :return req_services: dict of required services
    :rtype: dict
    """ 
    req_services = {}
    for service in dep_tree:
        if service not in run_exclude_images:
            update_dependency_service(service, dep_tree, req_services)
        
    return req_services


def yaml_parser(args):
    """Yaml parser method.

    :param args: cli arguments
    :type args: argparse
    """

    # Fetching EII directory path
    eii_dir = os.getcwd() + '/../'
    dir_list = []
    run_exclude_images = ["ia_eiibase", "ia_common", "ia_video_common", "ia_openvino_base", "uwc_common"]
    build_params = ["depends_on", "build", "image"]
    if args.yml_file is not None:
        # Fetching list of subdirectories from yaml file
        print("Fetching required services from {}...".format(args.yml_file))
        with open(args.yml_file, 'r') as sub_dir_file:
            yaml_data = ruamel.yaml.round_trip_load(sub_dir_file,
                                                    preserve_quotes=True)
            for service in yaml_data['AppContexts']:
                # In case user mentions the service as full path instead of
                # relative to IEdgeInsights.
                if service.startswith("/"):
                    prefix_path = service
                else:
                    prefix_path = eii_dir + service
                if os.path.isdir(prefix_path) or os.path.islink(prefix_path):
                    dir_list.append(service)
                else:
                    print("[WARN] {0} directory doesn't exist. Skipping fetching required files"
                          " from {0}".format(service))
    else:
        # Fetching list of subdirectories
        print("Parsing through directory to fetch required services...")
        dir_list = [f.name for f in os.scandir(eii_dir) if
                    (f.is_dir() or os.path.islink(f))]
        # Extend the dir_list to include the subdirectories from the 
        # file builder_config.json- include_services key
        dir_list.extend(include_services)
        # Exclude the services mentioned in the file
        # builder_config.json- exclude_services key
        for service in exclude_services:
            if service in dir_list:
                dir_list.remove(service)
        dir_list = sorted(dir_list)

    # Adding video folder manually since it's not a direct sub-directory
    if os.path.isdir(eii_dir + 'common/video'):
        dir_list.insert(0, 'common/video')

    # Removing the docker-compose.override.yml
    path_override = os.getcwd()+"/docker-compose.override.yml"
    if(os.path.exists(path_override)):
        os.remove(path_override)

    for app_dir in dir_list:
        # In case user mentions the service as full path instead of
        # relative to IEdgeInsights.
        if app_dir.startswith("/"):
            prefix_path = app_dir
        else:
            prefix_path = eii_dir + app_dir
        # Append to app_list if dir has both docker-compose.yml and config.json
        # & append to override_apps_list if override_directory has both
        # docker-compose.yml and config.json
        if args.override_directory is not None:
            override_dir = prefix_path + '/' + args.override_directory
            if os.path.isdir(override_dir):
                if (os.path.isfile(override_dir + '/docker-compose.yml') and
                   os.path.isfile(override_dir + '/config.json')):
                    override_apps_list.append(override_dir)
            elif (os.path.isfile(prefix_path + '/docker-compose.yml') and
                  os.path.isfile(prefix_path + '/config.json')):
                app_list.append(prefix_path)
        else:
            if (os.path.isfile(prefix_path + '/docker-compose.yml') and
               os.path.isfile(prefix_path + '/config.json')):
                app_list.append(prefix_path)

        # Append to helm_app_list if dir has deployment.yaml
        # and values.yaml files.
        # & append to override_helm_apps_list if
        # override_directory has deployment.yaml
        # and values.yaml files.
        if args.override_directory is not None:
            override_dir = prefix_path + '/' + args.override_directory
            if os.path.isdir(override_dir):
                if (os.path.isdir(override_dir + '/helm')):
                    override_helm_apps_list.append(override_dir)
            elif (os.path.isdir(prefix_path + '/helm')):
                helm_app_list.append(prefix_path)
        else:
            if (os.path.isdir(prefix_path + '/helm')):
                helm_app_list.append(prefix_path)

        # Append to override_list if dir has docker-compose-dev.override.yml
        if os.path.isfile(prefix_path + '/docker-compose-dev.override.yml'):
            dev_override_list.append(prefix_path)

    # Adding video folder manually since it's not a direct sub-directory
    # for multi instance feature
    if os.path.isdir(eii_dir + 'common/video'):
        app_list.insert(0, eii_dir + 'common/video')

    # Fetching DEV_MODE from .env
    dev_mode = False
    with open(".env") as f:
        for line in f:
            if line.startswith('DEV_MODE'):
                dev_mode = line.strip().split('=')[1]
                dev_mode = util.strtobool(dev_mode)
                break

    # Create build/multi_instance directory if num_multi_instances is greater than 1
    if (num_multi_instances > 1):
        global multi_inst_path
        multi_inst_path = os.path.join(os.getcwd(), 'multi_instance')
        if (not(os.path.isdir(multi_inst_path))):
            os.mkdir(multi_inst_path) 


    yml_dict = update_yml_dict(app_list, 'docker-compose.yml', dev_mode, args)
    temp = copy.deepcopy(yml_dict)

    deps = get_service_dependency_tree(yml_dict, run_exclude_images)
    # Get the list of services that needs to be written to docker-compose-build.yml
    required_services = get_required_services_list(deps, run_exclude_images)

    # Writing docker-compose-build.yml file. This yml will have services which will only
    # contain the build_params keys for all services.
    for k, v in yml_dict.items():
        if(k == "services"):
            for service, service_dict in v.items():
                if service not in required_services:
                    del temp["services"][service]
                    continue
                for service_keys, _ in list(service_dict.items()):
                    if(service_keys not in build_params):
                        del temp["services"][service][service_keys]
        elif (k != "version"):
            del temp[k]
    with open(DOCKER_COMPOSE_BUILD_PATH, 'w') as docker_compose_file:
        ruamel.yaml.round_trip_dump(temp, docker_compose_file)

    temp = copy.deepcopy(yml_dict)
    build_params.remove("image")
    
    # Writing docker-compose-push.yml file.
    for k, v in temp.items():
        if(k == "services"):
            for service, service_dict in v.items():
                # The docker-compose-push.yml contains the dummy build: . key
                # which is required to push the EII service docker images
                temp["services"][service]["build"] = "."
                temp["services"][service].move_to_end("build", last=False)
    with open(DOCKER_COMPOSE_PUSH_PATH, 'w') as docker_compose_file:
        ruamel.yaml.round_trip_dump(temp, docker_compose_file)

    # Writing docker-compose.yml file.
    for k, v in yml_dict.items():
        if(k == "services"):
            for service, service_dict in v.items():
                for service_keys, _ in list(service_dict.items()):
                    if(service_keys in build_params):
                        del temp["services"][service][service_keys]
                # Removing unnecessary build clause in docker-compose.yml
                if "build" in temp["services"][service]:
                    del temp["services"][service]["build"]
                if(service in run_exclude_images):
                    del temp["services"][service]
    with open(DOCKER_COMPOSE_PATH, 'w') as docker_compose_file:
         ruamel.yaml.round_trip_dump(temp, docker_compose_file)
    

    dev_mode_str = "PROD"
    if dev_mode:
        dev_mode_str = "DEV"
        # Creating docker-compose-dev.override.yml only in DEV mode
        create_docker_compose_override(dev_override_list, True, args, run_exclude_images)

    # Starting json parser
    json_parser(app_list, args)
    log_msg = """
    For deployment on single/multiple nodes,
    successfully created below consolidated files:
    * Required docker compose files: `docker-compose-build.yml`
      `docker-compose.yml` and `docker-compose-push.yml`
    * Consolidated config json of required EII services in
      `docker-compose.yml` at ./eii_config.json
    Please run the below commands to bring up the EII stack:
    Run:
    # For building EII services
    $ docker-compose -f docker-compose-build.yml build
    # For running EII services
    $ docker-compose up -d
    # For pushing EII docker images to registry
    (useful in multi-node deployment scenarios)
    $ docker-compose -f docker-compose-push.yml push
    """
    print(log_msg)
    # Sourcing required env from .env & provision/.env
    source_env("./.env")
    source_env("./provision/.env")

    try:
        helm_yaml_merger(helm_app_list, args)
    except Exception as e:
        print("Exception Occured at helm yml generation {}".format(e))
        sys.exit(1)
    log_msg = """
    For deployment via helm, successfully created consolidated
    values.yaml at ./helm-eii/eii-deploy/.
    Successfully copied all templates files of each eii modules to
    ./helm-eii/eii-deploy/templates/.
    Refer `./helm-eii/README.md` for deployment details.
    """
    print(log_msg)
    log_msg = """
    **NOTE**:
    Please re-run the `builder.py` whenever the individual docker-compose.yml,
    config.json files of individual services are updated
    and you need them to be considered
    """
    print(log_msg)


def parse_args():
    """Parse command line arguments.
    """
    arg_parse = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    arg_parse.add_argument('-f', '--yml_file', default=None,
                           help='Optional config file for list of services'
                           ' to include.\
                           Eg: python3 builder.py -f\
                           usecases/video-streaming.yml')
    arg_parse.add_argument('-v', '--video_pipeline_instances', default=1,
                           help='Optional number of video pipeline '
                                'instances to be created.\
                           Eg: python3 builder.py -v 6')
    arg_parse.add_argument('-d', '--override_directory',
                           default=None,
                           help='Optional directory consisting of '
                           'of benchmarking configs to be present in'
                           'each app directory.\
                           Eg: python3 builder.py -d benchmarking')
    return arg_parse.parse_args()


if __name__ == '__main__':

    # fetching builder config
    with open('builder_config.json', 'r') as config_file,\
         open('builder_schema.json', 'r') as schema_file:
        builder_cfg = json.load(config_file)
        builder_schema = json.load(schema_file)
        try:
            validate(instance=builder_cfg, schema=builder_schema)
        except Exception as e:
            print("JSON schema validation failed {}".format(e))
            sys.exit(1)
        # list of publishers and their endpoints
        publisher_list = builder_cfg['publisher_list']
        # list of subscribers
        subscriber_list = builder_cfg['subscriber_list']
        # list of services to include mandatorily
        include_services = builder_cfg['include_services']
        # list of services to exclude mandatorily
        exclude_services = builder_cfg['exclude_services']

    # Parse command line arguments
    args = parse_args()

    # Setting number of multi instances variable
    if int(args.video_pipeline_instances) > 1:
        num_multi_instances = int(args.video_pipeline_instances)

    # Start yaml parser
    yaml_parser(args)
    try:
        subprocess.check_output('./source.sh', stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as exc:
        raise RuntimeError(
                f'{" ".join(cmd)} failed: '
                f'{exc.output.decode("utf-8")}') from exc

