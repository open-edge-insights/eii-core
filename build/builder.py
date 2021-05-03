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

DOCKER_COMPOSE_PATH = './docker-compose.yml'
DOCKER_COMPOSE_BUILD_PATH = './docker-compose-build.yml'
DOCKER_COMPOSE_PUSH_PATH = './docker-compose-push.yml'
SCAN_DIR = ".."
dev_mode = False
# Initializing multi instance related variables
num_multi_instances = 0
override_apps_list, override_k8s_apps_list = \
    ([] for _ in range(2))
dev_override_list, app_list, k8s_app_list = \
    ([] for _ in range(3))
used_ports_dict = {"send_ports": [], "recv_ports": [], "srvc_ports": []}
override_apps_list, override_k8s_apps_list = ([] for _ in range(2))
dev_override_list, app_list, k8s_app_list = ([] for _ in range(3))
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
                    key, value = line.strip().split("=")
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
            new_port = str(int(port) + i)
            config['config']['ingestor']['pipeline'] =\
                config['config']['ingestor']['pipeline'].\
                replace(port, new_port)

    return config


def create_multi_instance_interfaces(config, i, bm_apps_list):
    """Generate multi instance interfaces for a given config

    :param config: app config
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
            if "Name" in x.keys():
                x["Name"] = x["Name"] + str(i+1)
            # Updating Topics section of all interfaces
            if "Topics" in x.keys():
                for index, topic in enumerate(x["Topics"]):
                    if bool(re.search(r'\d', x["Topics"][index])):
                        x["Topics"][index] = re.sub(r'\d+', str(i+1),
                                                    x["Topics"][index])
                    else:
                        x["Topics"][index] = x["Topics"][index] +\
                                                str(i+1)
            # Updating AllowedClients section of all interfaces
            if "AllowedClients" in x.keys():
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
                    if bool(re.search(r'\d', x["PublisherAppName"])):
                        x["PublisherAppName"] = re.sub(r'\d+', str(i+1),
                                                       x["PublisherAppName"])
                    else:
                        x["PublisherAppName"] = x["PublisherAppName"] +\
                                                      str(i+1)
            # Updating ServerAppName section of all interfaces
            if "ServerAppName" in x.keys():
                if x["ServerAppName"] in bm_apps_list:
                    if bool(re.search(r'\d', x["ServerAppName"])):
                        x["ServerAppName"] = re.sub(r'\d+', str(i+1),
                                                    x["ServerAppName"])
                    else:
                        x["ServerAppName"] = x["ServerAppName"] +\
                                                      str(i+1)
            # Updating Type section of all interfaces
            if "Type" in x.keys():
                if "tcp" in x["Type"]:
                    # If tcp mode, increment port numbers
                    port = x["EndPoint"].split(':')[1]
                    new_port = str(int(port) + (i))
                    x["EndPoint"] = x["EndPoint"].replace(port, new_port)
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
                port = client["EndPoint"].split(":")[1]
                new_port = str(int(port)+i)
                cli_instance["EndPoint"] = \
                    client["EndPoint"].replace(port, new_port)
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

    eii_config_path = "./provision/config/eii_config.json"
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
            if app_name not in subscriber_list and dirname != "video" and dirname != "EtcdUI" and "AzureBridge" not in app_path:
                for i in range(num_multi_instances):
                    with open(app_path + '/config.json', "rb") as infile:
                        head = json.load(infile)
                        # Increments rtsp port number if required
                        head = increment_rtsp_port(app_name, head, i)
                        # Create multi instance interfaces
                        head = \
                            create_multi_instance_interfaces(head,
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
                        # merge multi instance generated json to eii config
                        config_json = merge(config_json, data)
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
                # Merging individual app configs & interfaces into one json
                config_json = merge(config_json, data)

    # Writing consolidated json into desired location
    with open(eii_config_path, "w") as json_file:
        json_file.write(json.dumps(config_json, sort_keys=True, indent=4))

def k8s_yaml_remove_secrets(yaml_data):
    """This method takes input as a yml data and removes
       the secrets from volume mounts and envs and returns
       non-secrets yml data of k8s yml file

    :param yaml_data: Yaml value
    :type yaml_data: str
    """
    string_io_data = io.StringIO(yaml_data)
    if "---" in yaml_data:
        yaml_data_list = yaml_data.split("---")
        string_io_data = io.StringIO(yaml_data_list[1])
    yaml_prod = ruamel.yaml.round_trip_load(string_io_data,
                                            preserve_quotes=True)
    yaml_dict = dict()
    for k, v in yaml_prod.items():
        yaml_dict[k] = v

    vol_removal_list = []
    for d in yaml_dict['spec']['template']['spec']['volumes']:
        if 'cert' in d['name']:
            vol_removal_list.append(d)
        elif 'key' in d['name']:
            vol_removal_list.append(d)

    con_vol_removal_list = []
    for d in yaml_dict['spec']['template'
                               ]['spec']['containers'][0]['volumeMounts']:
        if 'cert' in d['name']:
            con_vol_removal_list.append(d)
        elif 'key' in d['name']:
            con_vol_removal_list.append(d)

    for r in vol_removal_list:
        yaml_dict['spec']['template']['spec']['volumes'].remove(r)
    for r in con_vol_removal_list:
        yaml_dict['spec']['template'
                          ]['spec']['containers'][0]['volumeMounts'].remove(r)

    kube_yaml = ruamel.yaml.round_trip_dump(yaml_dict)

    if "---" in yaml_data:
        kube_yaml = yaml_data_list[0] + "---\n" + kube_yaml

    return kube_yaml


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


def multi_instance_k8s_deployment(yaml_dict, i):
    """Method to update multi instance k8s yml for
       deployment section

    :param yaml_dict: k8s yaml dict
    :type yaml_dict: dict
    :param i: index of multi instance
    :type i: int
    :return: updated yml_dict
    :rtype: dict
    """
    # Updating appname
    yaml_dict["spec"]["selector"]["matchLabels"]["app"] = \
        yaml_dict["spec"]["selector"]["matchLabels"]["app"] + str(i+1)
    yaml_dict["spec"]["template"]["metadata"]["labels"]["app"] = \
        yaml_dict["spec"]["template"]["metadata"]["labels"]["app"] + str(i+1)
    yaml_dict["spec"]["template"]["spec"]["containers"][0]["name"] = \
        yaml_dict["spec"]["template"]["spec"]["containers"][0]["name"] + \
        str(i+1)

    # Updating volume mount part of k8s yaml dict
    volume_mount_dict = \
        yaml_dict["spec"]["template"]["spec"]["containers"][0]["volumeMounts"]
    for v in range(len(volume_mount_dict)):
        # Update cert section
        if ("cert" in volume_mount_dict[v]["name"] and
                "ca-cert" not in volume_mount_dict[v]["name"]):
            cert_temp = volume_mount_dict[v]["mountPath"].split('_cert')[0]
            new_cert = re.sub(r'\d+', '', cert_temp) + str(i+1) + '_cert'
            volume_mount_dict[v]["mountPath"] = new_cert
            volume_mount_dict[v]["name"] = \
                volume_mount_dict[v]["name"] + str(i+1)
        # Update key section
        if ("key" in volume_mount_dict[v]["name"] and
                "ca-cert" not in volume_mount_dict[v]["name"]):
            key_temp = volume_mount_dict[v]["mountPath"].split('_key')[0]
            new_key = re.sub(r'\d+', '', key_temp) + str(i+1) + '_key'
            volume_mount_dict[v]["mountPath"] = new_key
            volume_mount_dict[v]["name"] = \
                volume_mount_dict[v]["name"] + str(i+1)
    # Updating env part of k8s yaml dict
    env_dict = yaml_dict["spec"]["template"]["spec"]["containers"][0]["env"]
    for v in range(len(env_dict)):
        # Update AppName
        if env_dict[v]["name"] == "AppName":
            env_dict[v]["value"] = env_dict[v]["value"] + str(i+1)
        # Update CONFIGMGR_CERT & CONFIGMGR_KEY section
        if (env_dict[v]["name"] == "CONFIGMGR_CERT" or
                env_dict[v]["name"] == "CONFIGMGR_KEY"):
            app_name = env_dict[v]["value"].split("/")[-1].split("_")[0]
            new_app_name = app_name + str(i+1)
            env_dict[v]["value"] = \
                env_dict[v]["value"].replace(app_name, new_app_name)
        # Update tcp ports section
        if "ENDPOINT" in env_dict[v]["name"] and ":" in env_dict[v]["value"]:
            # Updating ports for PUBLISHER/SERVER ENDPOINTS
            if ("SERVER" in env_dict[v]["name"] or
                    "PUBLISHER" in env_dict[v]["name"]):
                port = env_dict[v]["value"].split(":")[-1]
                new_port = \
                    get_available_port(port, used_ports_dict["send_ports"])
                env_dict[v]["value"] = \
                    env_dict[v]["value"].replace(port, new_port)
            # Updating ports, appname for SUBSCRIBER/CLIENT ENDPOINTS
            if ("SUBSCRIBER" in env_dict[v]["name"] or
                    "CLIENT" in env_dict[v]["name"]):
                port = env_dict[v]["value"].split(":")[-1]
                new_port = \
                    get_available_port(port, used_ports_dict["recv_ports"])
                env_dict[v]["value"] = \
                    env_dict[v]["value"].replace(port, new_port)
                # Update appnames in ENDPOINTS
                appname = env_dict[v]["value"].split(":")[0]
                env_dict[v]["value"] = \
                    env_dict[v]["value"].replace(appname, appname + str(i+1))
                # Update endpoint name if it has a unique name
                if env_dict[v]["name"].count("_") > 1:
                    ep_name = env_dict[v]["name"].split("_")[1]
                    env_dict[v]["name"] = \
                        env_dict[v]["name"
                                    ].replace(ep_name, ep_name + str(i+1))
    # Updating volumes section of k8s yaml dict
    volume_dict = yaml_dict["spec"]["template"]["spec"]["volumes"]
    for v in range(len(volume_dict)):
        # Update cert section
        if ("cert" in volume_dict[v]["name"] and
                "ca-cert" not in volume_dict[v]["name"]):
            volume_dict[v]["name"] = volume_dict[v]["name"] + str(i+1)
            app_name = volume_dict[v]["secret"]["secretName"].split("-")[0]
            new_app_name = app_name + str(i+1)
            volume_dict[v]["secret"]["secretName"] = \
                volume_dict[v]["secret"
                               ]["secretName"].replace(app_name, new_app_name)
        # Update key section
        if "key" in volume_dict[v]["name"]:
            volume_dict[v]["name"] = volume_dict[v]["name"] + str(i+1)
            app_name = volume_dict[v]["secret"]["secretName"].split("-")[0]
            new_app_name = app_name + str(i+1)
            volume_dict[v]["secret"]["secretName"] = \
                volume_dict[v]["secret"
                               ]["secretName"].replace(app_name, new_app_name)
    return yaml_dict


def create_multi_subscribe_k8s_yml(k8s_path, dev_mode):
    """Method to create multi subscribe instance k8s yml

    :param k8s_path: path of yml dict
    :type k8s_path: str
    :param dev_mode: dev mode variable
    :type dev_mode: bool
    :return: multi subscribe instance k8s yml
    :rtype: str
    """
    merged_yaml = ""
    data = ""
    with open(k8s_path) as yaml_file:
        file_contents = yaml_file.read()
        string_io_data = io.StringIO(file_contents)
        # Create multi instance if app has both service & deployment section
        if "---" in file_contents:
            yaml_data_list = file_contents.split("---")
            string_io_data_one = io.StringIO(yaml_data_list[0])
            string_io_data = io.StringIO(yaml_data_list[1])
            yaml_prod_srvc = ruamel.yaml.round_trip_load(string_io_data_one,
                                                         preserve_quotes=True)
            yaml_prod = ruamel.yaml.round_trip_load(string_io_data,
                                                    preserve_quotes=True)

            # dict to update service section
            yaml_dict_srvc = dict()
            for k, v in yaml_prod_srvc.items():
                yaml_dict_srvc[k] = v

            # dict to update deployment section
            yaml_dict = dict()
            for k, v in yaml_prod.items():
                yaml_dict[k] = v

            env_dict = \
                yaml_dict["spec"]["template"]["spec"]["containers"][0]["env"]

            for v in range(len(env_dict)):
                # Update tcp ports section
                if ("ENDPOINT" in env_dict[v]["name"] and
                        ":" in env_dict[v]["value"]):
                    # Updating ports, appname for SUBSCRIBER/CLIENT ENDPOINTS
                    if ("SUBSCRIBER" in env_dict[v]["name"] or
                            "CLIENT" in env_dict[v]["name"]):
                        for i in range(num_multi_instances):
                            # Create a CommentedMap() to store updated values
                            new_env_dict = CommentedMap()
                            # Update ports
                            port = env_dict[v]["value"].split(":")[-1]
                            new_port = \
                                get_available_port(port,
                                                   used_ports_dict[
                                                       "recv_ports"])
                            # Update appname in endpoint
                            appname = env_dict[v]["value"].split(":")[0]
                            temp_value = \
                                env_dict[v]["value"].replace(port, new_port)
                            new_env_dict["value"] = \
                                temp_value.replace(appname,
                                                   appname + str(i+1))
                            # Update endpoint name if it has a unique name
                            if env_dict[v]["name"].count("_") > 1:
                                ep_name = env_dict[v]["name"].split("_")[1]
                                new_env_dict['name'] = \
                                    env_dict[v]["name"].replace(ep_name,
                                                                ep_name +
                                                                str(i+1))
                            # Append CommentedMap() to CommentedSeq()
                            env_dict.append(new_env_dict)
                        # Condition to remove recv_ports last few entries
                        # if multiple subscribers are subscribing to
                        # same publisher
                        used_ports_dict["recv_ports"] = \
                            used_ports_dict[
                                "recv_ports"][:len(used_ports_dict[
                                    "recv_ports"])-num_multi_instances]
                        del env_dict[v]

            # Merging deployment section updates
            kube_yaml = ruamel.yaml.round_trip_dump(yaml_dict)
            # Merging service section updates
            kube_yml_srvc = ruamel.yaml.round_trip_dump(yaml_dict_srvc)
            # Creating final k8s yml
            data = kube_yml_srvc + "---\n" + kube_yaml
        # Create multi instance if app has only deployment section
        else:
            yaml_prod = ruamel.yaml.round_trip_load(string_io_data,
                                                    preserve_quotes=True)
            # dict to update deployment section
            yaml_dict = dict()
            for k, v in yaml_prod.items():
                yaml_dict[k] = v

            env_dict = \
                yaml_dict["spec"]["template"]["spec"]["containers"][0]["env"]

            for v in range(len(env_dict)):
                # Update tcp ports section
                if ("ENDPOINT" in env_dict[v]["name"] and
                        ":" in env_dict[v]["value"]):
                    # Updating ports, appname for SUBSCRIBER/CLIENT ENDPOINTS
                    if ("SUBSCRIBER" in env_dict[v]["name"] or
                            "CLIENT" in env_dict[v]["name"]):
                        for i in range(num_multi_instances):
                            # Create a CommentedMap() to store updated values
                            new_env_dict = CommentedMap()
                            # Update ports
                            port = env_dict[v]["value"].split(":")[-1]
                            new_port = \
                                get_available_port(port,
                                                   used_ports_dict[
                                                       "recv_ports"])
                            # Update appname in endpoint
                            appname = env_dict[v]["value"].split(":")[0]
                            temp_value = \
                                env_dict[v]["value"].replace(port, new_port)
                            new_env_dict["value"] = \
                                temp_value.replace(appname,
                                                   appname + str(i+1))
                            # Update endpoint name if it has a unique name
                            if env_dict[v]["name"].count("_") > 1:
                                ep_name = env_dict[v]["name"].split("_")[1]
                                new_env_dict['name'] = \
                                    env_dict[v]["name"].replace(ep_name,
                                                                ep_name +
                                                                str(i+1))
                            # Append CommentedMap() to CommentedSeq()
                            env_dict.append(new_env_dict)
                        # Condition to remove recv_ports last few entries
                        # if multiple subscribers are subscribing to
                        # same publisher
                        used_ports_dict["recv_ports"] = \
                            used_ports_dict[
                                "recv_ports"][:len(used_ports_dict[
                                    "recv_ports"])-num_multi_instances]
                        del env_dict[v]

            # Merging deployment section updates
            kube_yaml = ruamel.yaml.round_trip_dump(yaml_dict)
            # Creating final k8s yml
            data = kube_yaml
        # Updating yml based on dev_mode
        if dev_mode:
            merged_yaml = \
                merged_yaml + "---\n" + k8s_yaml_remove_secrets(data)
        else:
            merged_yaml = merged_yaml + "---\n" + data

    return merged_yaml


def create_multi_instance_k8s_yml(k8s_path, dev_mode, i):
    """Method to create multi instance k8s yml

    :param k8s_path: path of yml dict
    :type k8s_path: str
    :param dev_mode: dev mode variable
    :type dev_mode: bool
    :param i: index of multi instance
    :type i: int
    :return: multi instance k8s yml
    :rtype: str
    """
    merged_yaml = ""
    data = ""
    with open(k8s_path) as yaml_file:
        file_contents = yaml_file.read()
        string_io_data = io.StringIO(file_contents)
        # Create multi instance if app has both service & deployment section
        if "---" in file_contents:
            yaml_data_list = file_contents.split("---")
            string_io_data_one = io.StringIO(yaml_data_list[0])
            string_io_data = io.StringIO(yaml_data_list[1])
            yaml_prod_srvc = ruamel.yaml.round_trip_load(string_io_data_one,
                                                         preserve_quotes=True)
            yaml_prod = ruamel.yaml.round_trip_load(string_io_data,
                                                    preserve_quotes=True)

            # dict to update service section
            yaml_dict_srvc = dict()
            for k, v in yaml_prod_srvc.items():
                yaml_dict_srvc[k] = v
            # Updating app name
            yaml_dict_srvc['metadata']['name'] = \
                yaml_dict_srvc['metadata']['name'] + str(i+1)
            yaml_dict_srvc['spec']['selector']['app'] = \
                yaml_dict_srvc['spec']['selector']['app'] + str(i+1)
            # Updating ports in service section
            ports_dict = yaml_dict_srvc['spec']['ports']
            for v in range(len(ports_dict)):
                # Update port
                if "port" in ports_dict[v]:
                    if "$" not in str(ports_dict[v]['port']):
                        port = ports_dict[v]['port']
                        new_port = \
                            get_available_port(str(port),
                                               used_ports_dict['srvc_ports'])
                        ports_dict[v]['port'] = int(new_port)
                    # Update targetPort if it exists
                    if "targetPort" in ports_dict[v]:
                        if "$" not in str(ports_dict[v]['targetPort']):
                            ports_dict[v]['targetPort'] = int(new_port)
                    # Update nodePort if it exists
                    if "nodePort" in ports_dict[v]:
                        if "$" not in str(ports_dict[v]['nodePort']):
                            port = ports_dict[v]['nodePort']
                            new_port = \
                                get_available_port(str(port),
                                                   used_ports_dict['srvc_ports'
                                                                   ])
                            ports_dict[v]['nodePort'] = int(new_port)

            # dict to update deployment section
            yaml_dict = dict()
            for k, v in yaml_prod.items():
                yaml_dict[k] = v

            # Update deployment section app name
            yaml_dict["metadata"]["labels"]["app"] = \
                yaml_dict["metadata"]["labels"]["app"] + str(i+1)
            yaml_dict["metadata"]["name"] = \
                yaml_dict["metadata"]["name"] + str(i+1)
            # Update deployment section
            yaml_dict = multi_instance_k8s_deployment(yaml_dict, i)

            # Merging deployment section updates
            kube_yaml = ruamel.yaml.round_trip_dump(yaml_dict)
            # Merging service section updates
            kube_yml_srvc = ruamel.yaml.round_trip_dump(yaml_dict_srvc)
            # Creating final k8s yml
            data = kube_yml_srvc + "---\n" + kube_yaml
        # Create multi instance if app has only deployment section
        else:
            yaml_prod = ruamel.yaml.round_trip_load(string_io_data,
                                                    preserve_quotes=True)
            # dict to update deployment section
            yaml_dict = dict()
            for k, v in yaml_prod.items():
                yaml_dict[k] = v
            # Update deployment section app name
            yaml_dict["metadata"]["labels"]["app"] = \
                yaml_dict["metadata"]["labels"]["app"] + str(i+1)
            yaml_dict["metadata"]["name"] = \
                yaml_dict["metadata"]["name"] + str(i+1)
            # Update deployment section
            yaml_dict = multi_instance_k8s_deployment(yaml_dict, i)

            # Merging deployment section updates
            kube_yaml = ruamel.yaml.round_trip_dump(yaml_dict)
            # Creating final k8s yml
            data = kube_yaml
        # Updating yml based on dev_mode
        if dev_mode:
            merged_yaml = \
                merged_yaml + "---\n" + k8s_yaml_remove_secrets(data)
        else:
            merged_yaml = merged_yaml + "---\n" + data

    return merged_yaml


def k8s_yaml_merger(app_list, dev_mode, args):
    """Method merges the k8s yml files of each eii
       modules and generates a consolidated ymlfile.

    :param app_list: List of services
    :type app_list: list
    :param dev_mode: Dev Mode key
    :type dev_mode: bool
    """
    app_list.extend(override_k8s_apps_list)
    merged_yaml = ""
    # Iterate through app_list & merge k8s yaml
    # and create multi instance yaml if multi
    # instance is required
    for kube_yaml in app_list:
        app_name = kube_yaml.split("/")[-2]
        if args.override_directory is not None:
            if args.override_directory in kube_yaml:
                app_name = kube_yaml.split("/")[-3]
        # Generating multi instance for Publishers/Servers
        if num_multi_instances > 1 and app_name not in subscriber_list.keys():
            for i in range(num_multi_instances):
                multi_instance_yml = \
                    create_multi_instance_k8s_yml(kube_yaml, dev_mode, i)
                merged_yaml = merged_yaml + "---\n" + multi_instance_yml
        # Generating multi instance for Subscribers/Clients
        elif num_multi_instances > 1 and app_name in subscriber_list.keys():
            multi_instance_yml = \
                create_multi_subscribe_k8s_yml(kube_yaml, dev_mode)
            merged_yaml = merged_yaml + "---\n" + multi_instance_yml
        else:
            with open(kube_yaml) as yaml_file:
                data = yaml_file.read()
                if dev_mode:
                    merged_yaml = \
                        merged_yaml + "---\n" + k8s_yaml_remove_secrets(data)
                else:
                    merged_yaml = merged_yaml + "---\n" + data

    k8s_service_yaml = './k8s/eii-k8s-deploy.yml'
    with open(k8s_service_yaml, 'w') as final_yaml:
        final_yaml.write(merged_yaml)
    # Substituting sourced env in k8s_service_yaml
    env_subst(k8s_service_yaml, k8s_service_yaml)


def create_multi_instance_yml_dict(data, i):
    """Method to generate boilerplate
       yamls

    :param data: input yaml
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
    temp = copy.deepcopy(data)
    for k, v in temp['services'].items():
        # Update yaml main key name
        k_new = re.sub(r'\d+', '', k) + str(i)
        temp['services'][k_new] = v
        del temp['services'][k]
        # Iterating throught the yaml to update
        # all keys & values
        for k2, v2 in v.items():
            # Update container_name
            if k2 == 'container_name':
                v['container_name'] = re.sub(r'\d+', '', v2) + str(i)
            # Update hostname
            elif k2 == 'hostname':
                v['hostname'] = re.sub(r'\d+', '', v2) + str(i)
            # Update secrets section of yml
            elif k2 == 'secrets':
                for v3 in v['secrets']:
                    if 'cert' in v3:
                        cert_temp = v3.split('_cert')[0]
                        new_cert = re.sub(r'\d+', '', cert_temp) +\
                            str(i) + '_cert'
                        v['secrets'].remove(cert_temp+'_cert')
                        v['secrets'].insert(1, new_cert)
                    if 'key' in v3:
                        key_temp = v3.split('_key')[0]
                        new_key = re.sub(r'\d+', '', key_temp) +\
                            str(i) + '_key'
                        v['secrets'].remove(key_temp+'_key')
                        v['secrets'].insert(2, new_key)
            # Update all env key, value pairs of yml
            elif k2 == 'environment':
                for k4, v4 in list(v['environment'].items()):
                    # Update AppName
                    if k4 == 'AppName':
                        v['environment'][k4] = v4 + str(i)
                        app_name = v4
                        new_app_name = v4 + str(i)

    # Update outer secrets section
    if not dev_mode:
        if 'secrets' in temp.keys():
            for k, v in list(temp['secrets'].items()):
                if 'cert' in k:
                    cert_temp = k.split('_cert')[0]
                    new_cert = re.sub(r'\d+', '', cert_temp) + str(i) + '_cert'
                    for k2 in v.items():
                        yml_map = ruamel.yaml.comments.CommentedMap()
                        yml_map.insert(1, 'file',
                                       list(k2)[1].replace(app_name,
                                                           new_app_name))
                    temp['secrets'][new_cert] = yml_map
                    del temp['secrets'][k]
                if 'key' in k:
                    key_temp = k.split('_key')[0]
                    new_key = re.sub(r'\d+', '', key_temp) + str(i) + '_key'
                    for k2 in v.items():
                        yml_map = ruamel.yaml.comments.CommentedMap()
                        yml_map.insert(1, 'file',
                                       list(k2)[1].replace(app_name,
                                                           new_app_name))
                    temp['secrets'][new_key] = yml_map
                    del temp['secrets'][k]

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

            # Create multi instance compose if num_multi_instances is > 1
            if num_multi_instances > 1:
                appname = k.split("/")[-1]
                if args.override_directory is not None:
                    if args.override_directory in k:
                        appname = k.split("/")[-2]
                # Create single instance only for services in subscriber_list
                # and for corner case of common/video,
                # create multi instance otherwise
                # TODO: Support AzureBridge
                # multi instance creation if applicable
                if (appname not in subscriber_list.keys() and
                        appname != "video" and
                        appname != "common" and
                        appname != "AzureBridge"):
                    for i in range(num_multi_instances):
                        data_two = create_multi_instance_yml_dict(data, i+1)
                        yaml_files_dict.append(data_two)
                # To create single instance for subscriber services
                else:
                    yaml_files_dict.append(data)
            # To create single instance
            else:
                yaml_files_dict.append(data)

    # Updating final yaml dict
    yaml_dict = yaml_files_dict[0]
    for var in yaml_files_dict[1:]:
        for k in var:
            try:
                # Update the values from current compose
                # file to previous compose file
                for i in var[k]:
                    if(k == "version"):
                        pass
                    else:
                        yaml_dict[k].update({i: var[k][i]})
            # If secrets is not existing, add it to the dict
            except KeyError:
                if (k == "secrets"):
                    yaml_dict[k] = var[k]

    # Updating yaml dict for dev mode
    if dev_mode:
        for k, v in yaml_dict.items():
            # Deleting the main secrets section
            if(k == "secrets"):
                del yaml_dict[k]
            # Deleting secrets section for individual services
            elif(k == "services"):
                for _, service_dict in v.items():
                    for service_keys, _ in list(service_dict.items()):
                        if(service_keys == "secrets"):
                            del service_dict[service_keys]
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


def yaml_parser(args):
    """Yaml parser method.

    :param args: cli arguments
    :type args: argparse
    """

    # Fetching EII directory path
    eii_dir = os.getcwd() + '/../'
    dir_list = []
    run_exclude_images = ["ia_eiibase", "ia_common", "ia_video_common", "ia_openvino_base"]
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
        # Fetching list of subdirectories
        print("Parsing through directory to fetch required services...")
        dir_list = [f.name for f in os.scandir(eii_dir) if
                    (f.is_dir() or os.path.islink(f))]
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
        # Append to k8s_app_list if dir has k8s-service.yml
        # & append to override_k8s_apps_list if
        # override_directory has k8s-service.yml
        if args.override_directory is not None:
            override_dir = prefix_path + '/' + args.override_directory
            if os.path.isdir(override_dir):
                if os.path.isfile(override_dir + '/k8s-service.yml'):
                    override_k8s_apps_list.append(override_dir +
                                                  '/k8s-service.yml')
            elif os.path.isfile(prefix_path + '/k8s-service.yml'):
                k8s_app_list.append(prefix_path + '/k8s-service.yml')
        else:
            if os.path.isfile(prefix_path + '/k8s-service.yml'):
                k8s_app_list.append(prefix_path + '/k8s-service.yml')

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

    yml_dict = update_yml_dict(app_list, 'docker-compose.yml', dev_mode, args)
    temp = copy.deepcopy(yml_dict)
    # Writing docker-compose-build.yml file. This yml will have services which will only
    # contain the build_params keys for all services.
    for k, v in yml_dict.items():
        if(k == "services"):
            for service, service_dict in v.items():
                for service_keys, _ in list(service_dict.items()):
                    if(service_keys not in build_params):
                        del temp["services"][service][service_keys]
        elif (k != "version"):
            del temp[k]
    with open(DOCKER_COMPOSE_BUILD_PATH, 'w') as docker_compose_file:
        ruamel.yaml.round_trip_dump(temp, docker_compose_file)

    temp = copy.deepcopy(yml_dict)
    build_params.remove("image")
    # Writing docker-compose.yml file.
    for k, v in yml_dict.items():
        if(k == "services"):
            for service, service_dict in v.items():
                for service_keys, _ in list(service_dict.items()):
                    if(service_keys in build_params):
                        del temp["services"][service][service_keys]
                if(service in run_exclude_images):
                    del temp["services"][service]

    with open(DOCKER_COMPOSE_PATH, 'w') as docker_compose_file:
        ruamel.yaml.round_trip_dump(temp, docker_compose_file)

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
      `docker-compose.yml` at ./provision/config/eii_config.json
    Please run the below commands to bring up the EII stack:
    Run:
    # provision the node to run EII
    $ cd provision && sudo ./provision.sh ../docker-compose.yml
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

    # Generating Consolidated k8s yaml file for deployment
    try:
        k8s_yaml_merger(k8s_app_list, dev_mode, args)
    except Exception as e:
        print("Exception Occured at Kubernetes yml generation {}".format(e))
        sys.exit(1)
    log_msg = """
    For deployment via k8s orchestrator,
    successfully created consolidated Kubernetes deployment
    yml at ./k8s/eii-k8s-deploy.yml. Refer `./k8s/README.md`
    for deployment details.
    """
    print(log_msg)
    log_msg = """
    **NOTE**:
    Please re-run the `builder.py` whenever
    the individual docker-compose.yml, config.json and
    k8s-service files of individual services are updated
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

    # Parse command line arguments
    args = parse_args()

    # Setting number of multi instances variable
    if int(args.video_pipeline_instances) > 1:
        num_multi_instances = int(args.video_pipeline_instances)

    # Start yaml parser
    yaml_parser(args)
