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
"""script to generate consolidated docker-compose.yml, eis_config.json
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

DOCKER_COMPOSE_PATH = './docker-compose.yml'
SCAN_DIR = ".."
dev_mode = False
# Initializing multi instance related variables
num_multi_instances = 0
override_apps_list, override_csl_apps_list, override_k8s_apps_list = ([] for _ in range(3))
dev_override_list, app_list, csl_app_list, k8s_app_list = ([] for _ in range(4))
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
       eis_builder_cfg['increment_rtsp_port'] is True:
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
                            if bool(re.search(r'\d', x["AllowedClients"][index])):
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
                cli_instance["PublisherAppName"] = client["PublisherAppName"] + str(i+1)
            # Updating ServerAppName of client instance
            if "ServerAppName" in client.keys():
                cli_instance["ServerAppName"] = client["ServerAppName"] + str(i+1)
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
                cli_instance["EndPoint"] = client["EndPoint"].replace(port, new_port)
            # Appending client multi instances
            temp["interfaces"][client_type].append(cli_instance)
        # Remove existing instances
        temp["interfaces"][client_type].remove(client)
    return temp


def json_parser(file, args):
    """Generate etcd config by parsing through
       individual app configs

    :param file: Path of json file
    :type file: str
    :param args: cli args
    :type args: argparse
    """
    # Fetching GlobalEnv config
    with open('./common_config.json', "rb") as infile:
        data = {}
        head = json.load(infile)
        data['/GlobalEnv/'] = head
        config_json = data

    # Fetching docker-compose.yml to retrieve required App dirs
    with open(file, 'r') as compose_file:
        data = ruamel.yaml.round_trip_load(compose_file, preserve_quotes=True)

    app_list = []
    # Replacing $PWD with relative path to EIS dir
    for key in data['services']:
        try:
            rel_dir = data['services'][key]['build']['context'].replace(
                '$PWD/..', '..')
        except Exception:
            # If build isn't found, use CONTEXT set in the environment
            rel_dir = data['services'][key]['environment']['CONTEXT'].replace(
                '$PWD/..', '..')
        app_list.append(rel_dir)

    # Removing duplicates & unwanted dirs from app list
    app_list = list(dict.fromkeys(app_list))
    app_list.remove(SCAN_DIR + '/common')

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

    eis_config_path = "./provision/config/eis_config.json"
    for app_path in app_list:
        data = {}
        # Creating multi instance config if num_multi_instances > 1
        if num_multi_instances > 1:
            dirname = app_path.split("/")[-1]
            if args.override_directory is not None:
                if args.override_directory in app_path:
                    dirname = app_path.split("/")[-2]
            # Ignoring EtcdUI & common/video service to not create multi instance
            # TODO: Support EISAzureBridge multi instance creation if applicable
            if dirname not in subscriber_list and dirname != "video" and dirname != "EtcdUI" and "EISAzureBridge" not in app_path:
                for i in range(num_multi_instances):
                    with open(app_path + '/config.json', "rb") as infile:
                        head = json.load(infile)
                        # Increments rtsp port number if required
                        head = increment_rtsp_port(dirname, head, i)
                        # Create multi instance interfaces
                        head = \
                            create_multi_instance_interfaces(head,
                                                            i,
                                                            bm_apps_list)
                        # merge config of multi instance config
                        if 'config' in head.keys():
                            data['/' + dirname + str(i+1) + '/config'] = \
                                head['config']
                        # merge interfaces of multi instance config
                        if 'interfaces' in head.keys():
                            data['/' + dirname + str(i+1) +
                                '/interfaces'] = \
                                head['interfaces']
                        # merge multi instance generated json to eis config
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
                            temp = create_multi_subscribe_interface(head, temp, "Subscribers")
                            data['/' + dirname + '/config'] = head['config']
                            data['/' + dirname + '/interfaces'] = temp['interfaces']
                            config_json = merge(config_json, data)
                        if "Clients" in head["interfaces"]:
                            # Generate multi client interface
                            temp = create_multi_subscribe_interface(head, temp, "Clients")
                            data['/' + dirname + '/config'] = head['config']
                            data['/' + dirname + '/interfaces'] = temp['interfaces']
                            config_json = merge(config_json, data)
                        # This is to handle empty interfaces cases like EtcdUI
                        data['/' + dirname + '/config'] = head['config']
                        data['/' + dirname + '/interfaces'] = temp['interfaces']
                        config_json = merge(config_json, data)
        # This condition is to handle the default non multi instance flow
        else:
            with open(app_path + '/config.json', "rb") as infile:
                # Merging config & interfaces from all services
                head = json.load(infile)
                app_name = app_path.replace('.', '')
                # remove trailing '/'
                app_name = app_name.rstrip('/')
                # Fetching AppName & succeeding it with "/"
                app_name = app_name.split('/')[-1]
                if 'config' in head.keys():
                    data['/' + app_name + '/config'] = head['config']
                if 'interfaces' in head.keys():
                    data['/' + app_name + '/interfaces'] = head['interfaces']
                # Merging individual app configs & interfaces into one json
                config_json = merge(config_json, data)

    # Writing consolidated json into desired location
    with open(eis_config_path, "w") as json_file:
        json_file.write(json.dumps(config_json, sort_keys=True, indent=4))
        print("Successfully created consolidated config json at {}".format(
              eis_config_path))


def create_multi_instance_csl_app_spec(app_spec_json, args):
    """Method to generate csl boilerplate appspecs

    :param app_spec_json: appspec json
    :type app_spec_json: dict
    :param args: cli args
    :type args: argparse
    :return: list of duplicated csl appspecs
    :rtype: list
    """
    # Fetching individual app names from override_apps_list
    bm_apps_list = []
    for x in override_apps_list:
        appname = x.split('/' + args.override_directory)[0].split('/')[-1]
        bm_apps_list.append(appname)

    # Generating boilerplate csl appspec
    new_list = []
    for i in range(int(args.video_pipeline_instances)):
        temp = copy.deepcopy(app_spec_json)
        # Updating module name
        app_name = re.sub(r'\d+', '', temp['Name'])
        temp['Name'] = app_name + str(i+1)
        temp['ExecutionEnv']['AppName'] = app_name + str(i+1)
        # Updating PubTopics
        if 'PubTopics' in temp['ExecutionEnv']:
            temp['ExecutionEnv']['PubTopics'] = re.sub(r'\d+',
                                                       str(i+1),
                                                       temp['ExecutionEnv']
                                                           ['PubTopics'])
        # Updating SubTopics
        if 'SubTopics' in temp['ExecutionEnv']:
            temp['ExecutionEnv']['SubTopics'] = re.sub(r'\d+',
                                                       str(i+1),
                                                       temp['ExecutionEnv']
                                                           ['SubTopics'])
        # Updating individual topic configs
        for k in list(temp['ExecutionEnv'].keys()):
            if '_cfg' in k:
                if bool(re.search(r'\d', k)):
                    new_k = re.sub(r'\d+', str(i+1), k)
                    temp['ExecutionEnv'][new_k] = temp['ExecutionEnv'][k]
                    del temp['ExecutionEnv'][k]
                else:
                    new_k = k + str(i+1)
                    temp['ExecutionEnv'][new_k] = temp['ExecutionEnv'][k]
                    del temp['ExecutionEnv'][k]
        # Updating Clients section & SubTopics AppNames
        for x in bm_apps_list:
            if 'Clients' in temp['ExecutionEnv']:
                if x in temp['ExecutionEnv']['Clients']:
                    temp['ExecutionEnv']['Clients'] =\
                        temp['ExecutionEnv']['Clients'].replace(x,
                                                                x + '1,' +
                                                                x + str(i+1))
            if 'SubTopics' in temp['ExecutionEnv']:
                if x in temp['ExecutionEnv']['SubTopics']:
                    temp['ExecutionEnv']['SubTopics'] =\
                        temp['ExecutionEnv']['SubTopics'].replace(x,
                                                                  x + str(i+1))

        # Updating ports & links
        if 'Endpoints' in temp.keys():
            for x in temp['Endpoints']:
                if 'Link' in x:
                    x['Link'] = x['Link'] + '-' + str(i+1)
                if 'Port' in x:
                    if '$' not in x['Port']:
                        x['Port'] = str(int(x['Port']) + i)

        # Append every appspec to list of appspecs
        new_list.append(temp)

    return new_list


def csl_parser(app_list, args):
    """Generate CSL AppSpec by parsing through
       individual app specific AppSpecs

    :param app_list: List of services
    :type app_list: list
    :param args: cli args
    :type args: argparse
    """
    # Fetching the template json
    with open('./csl/csl_template.json', "rb") as infile:
        csl_template = json.load(infile)

    # Creating benchmarking apps list to edit interfaces accordingly
    app_list.extend(override_csl_apps_list)
    bm_app_list = []
    for x in app_list:
        bm_appname = x.split("/")[-1]
        if args.override_directory is not None:
            if args.override_directory in x:
                bm_appname = x.split("/")[-2]
        bm_app_list.append(bm_appname)

    # Updating publisher apps endpoints
    for app in app_list:
        with open(app + '/app_spec.json', "rb") as infile:
            json_file = json.load(infile)
            head = json_file["Module"]
            if head["Name"] == list(publisher_list.keys())[0] or\
               head["Name"] == list(publisher_list.keys())[1]:
                for end_points in head["Endpoints"]:
                    if end_points[
                            "Name"] == list(publisher_list.values())[0] or \
                            end_points[
                                "Name"] == list(publisher_list.values())[1]:
                        csl_pub_endpoint_list[head["Name"]] = end_points

    # Creating deploy AppSpec from individual AppSpecs
    all_link_backend = []
    for app in app_list:
        if num_multi_instances > 1:
            app_name = app.split('/')[-1]
            if args.override_directory is not None:
                if args.override_directory in app:
                    app_name = app.split('/')[-2]
        with open(app + '/app_spec.json', "rb") as infile:
            json_file = json.load(infile)
            head = json_file["Module"]
            # Generate boilerplate config for benchmark apps
            if num_multi_instances > 1 and app_name not in subscriber_list.keys():
                data = create_multi_instance_csl_app_spec(head, args)
                csl_template["Modules"].extend(data)
            else:
                csl_template["Modules"].append(head)

            count = 0
            endpoint_to_remove = []
            # Fetch links from all AppSpecs & update deploy AppSpec
            for head in csl_template['Modules']:
                for endpoint in head["Endpoints"]:
                    # Variable to handle no publishers
                    publisher_present = False
                    if "Link" in endpoint.keys():
                        csl_template["Links"].append({"Name":
                                                      endpoint["Link"]})
                        all_link_backend.append(endpoint["Link"])
                    else:
                        pub_list = []
                        # Iterate through available publisher list
                        for publisher in csl_pub_endpoint_list.keys():
                            # Verify publiser doesn't try to fetch
                            # keys of itself
                            if publisher != head["Name"]:
                                pub_list.append(csl_pub_endpoint_list[
                                    publisher])

                        # Matching the links based on the order
                        # [VA, InfluxDBConnector]. If a publisher is
                        # empty (i.e VA or Influx is not present)
                        # endpoint of the service which subscribe to VA
                        # or Influx will be removed
                        if count < len(publisher_list.keys()):
                            if pub_list[count] != {}:
                                if pub_list[count]["Endtype"] == "server":
                                    endpoint["Link"] = pub_list[count]["Link"]
                                    publisher_present = True
                                    all_link_backend.append(endpoint["Link"])
                                    count += 1

                            if publisher_present is False:
                                endpoint_to_remove.append(endpoint)
                                count += 1

                # Remove link if no publishers are available
                for endpoint in endpoint_to_remove:
                    head["Endpoints"].remove(endpoint)

                # Remove those Key-Value pair which are using the
                # deleted endpoint in the config
                keys_to_remove = []
                for key, value in head["ExecutionEnv"].items():
                    for endpoint in endpoint_to_remove:
                        if endpoint["Name"] in value:
                            keys_to_remove.append(key)

                for key in keys_to_remove:
                    del head["ExecutionEnv"][key]

                # Removing all duplicate links
                csl_template["Links"] = [dict(t) for t in {tuple(d.items())
                                                           for d
                                                           in csl_template[
                                                            "Links"]}]

                # Fetch links from all AppSpecs & update deploy AppSpec
                if "Ingress" in json_file.keys():
                    if json_file["Ingress"] not in csl_template["Ingresses"]:
                        csl_template["Ingresses"].append(json_file["Ingress"])
                    all_link_backend.append(json_file["Ingress"]["Options"][
                        "Backend"])

    # Counting the occurance of links from all list and backends
    open_link = {}
    count = 1
    all_link_backend.sort()
    for index in range(0, len(all_link_backend)):
        if index+1 < len(all_link_backend):
            if all_link_backend[index] == all_link_backend[index+1]:
                count += 1
            else:
                open_link[all_link_backend[index]] = count
                if index != 0:
                    count = 1
        else:
            if all_link_backend[index] != all_link_backend[index-1]:
                open_link[all_link_backend[index]] = count

    # If the links, occur more than once, removing those links
    # remaining links will be either server open link or client open link
    links_to_del = []
    for key, value in open_link.items():
        if value > 1:
            links_to_del.append(key)

    for key in links_to_del:
        del open_link[key]

    # Find the client open link and save it to delete the endpoint and
    # ExecutionEnv Key: Value to delete the config
    links_to_del = []
    client_link = []
    for index in range(len(app_list)):
        head = csl_template["Modules"][index]
        for key in open_link.keys():
            for endpoint in head["Endpoints"]:
                if endpoint["Link"] == key:
                    if endpoint["Endtype"] == "client":
                        links_to_del.append(endpoint["Name"])
                        client_link.append(key)

        if links_to_del:
            for link in links_to_del:
                for endpoint in head["Endpoints"]:
                    if endpoint["Name"] == link:
                        index = head["Endpoints"].index(endpoint)
                        del head["Endpoints"][index]

            keys_to_remove = []
            for key, value in head["ExecutionEnv"].items():
                for endpoint in links_to_del:
                    if not isinstance(value, int) and endpoint in value:
                        keys_to_remove.append(key)

            for key in keys_to_remove:
                del head["ExecutionEnv"][key]

    # Delete the link from csl_app_spec links list
    del_index = []
    for index in range(0, len(csl_template["Links"])):
        for key in client_link:
            if key == csl_template["Links"][index]["Name"]:
                del_index.append(index)

    for index in del_index:
        del csl_template["Links"][index]

    # Creating consolidated json
    with open('./csl/tmp_csl_app_spec.json', "w") as json_file:
        json_file.write(json.dumps(csl_template, sort_keys=False, indent=4))

    # Generating module specs for all apps
    for app in app_list:
        app_name = app.rsplit("/", 1)[1]
        if args.override_directory is not None:
            if args.override_directory in app:
                app_name = app.rsplit("/", 2)[1]
        module_spec_path = "./csl/" + app_name + "_module_spec.json"
        app_path = app + "/module_spec.json"
        # Substituting sourced env in module specs
        cmd = subprocess.run(["cat", app_path], stdout=subprocess.PIPE,
                             check=False)

        # Updating module specs for multi instance
        if num_multi_instances > 1:
            json_value = json.loads(cmd.stdout.decode())
            for i in range(num_multi_instances):
                nm = json_value['DataStore']['DataBuckets'][0]['Name']
                nm_val = nm + str(i+1)
                json_value['DataStore']['DataBuckets'].append({"Name":
                                                               nm_val,
                                                               "Permissions":
                                                               "RO"})
            cmd = json.dumps(json_value)
        try:
            if num_multi_instances > 1:
                # Write module spec to temporary file for envsubst
                with open("./"+app_name+"module_spec_temp.json", "w") \
                   as module_spec_file:
                    module_spec_file.write(cmd)
                # Execute envsubt on temp file & store output
                # in module_spec_path
                env_subst("./"+app_name+"module_spec_temp.json",
                          module_spec_path)
                # Remove temporary file
                os.remove("./"+app_name+"module_spec_temp.json")
            else:
                env_subst(cmd, module_spec_path, input_type='cmd')
        except subprocess.CalledProcessError as err:
            print("Subprocess error: {}, {}".format(err.returncode,
                                                    err.output))
            sys.exit(1)

    # Substituting sourced env in AppSpec
    csl_config_path = "./csl/csl_app_spec.json"
    env_subst("./csl/tmp_csl_app_spec.json", csl_config_path)

    # Removing generated temporary file
    os.remove("./csl/tmp_csl_app_spec.json")

    print("Successfully created consolidated AppSpec json at "
          "{}".format(csl_config_path))


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
    for d in yaml_dict['spec']['template']['spec']['containers'][0]['volumeMounts']:
        if 'cert' in d['name']:
            con_vol_removal_list.append(d)
        elif 'key' in d['name']:
            con_vol_removal_list.append(d)

    for r in vol_removal_list:
        yaml_dict['spec']['template']['spec']['volumes'].remove(r)
    for r in con_vol_removal_list:
        yaml_dict['spec']['template']['spec']['containers'][0]['volumeMounts'].remove(r)

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
    yaml_dict["spec"]["selector"]["matchLabels"]["app"] = yaml_dict["spec"]["selector"]["matchLabels"]["app"] + str(i+1)
    yaml_dict["spec"]["template"]["metadata"]["labels"]["app"] = yaml_dict["spec"]["template"]["metadata"]["labels"]["app"] + str(i+1)
    yaml_dict["spec"]["template"]["spec"]["containers"][0]["name"] = yaml_dict["spec"]["template"]["spec"]["containers"][0]["name"] + str(i+1)

    # Updating volume mount part of k8s yaml dict
    volume_mount_dict = yaml_dict["spec"]["template"]["spec"]["containers"][0]["volumeMounts"]
    for v in range(len(volume_mount_dict)):
        # Update cert section
        if "cert" in volume_mount_dict[v]["name"] and "ca-cert" not in volume_mount_dict[v]["name"]:
            cert_temp = volume_mount_dict[v]["mountPath"].split('_cert')[0]
            new_cert = re.sub(r'\d+', '', cert_temp) + str(i+1) + '_cert'
            volume_mount_dict[v]["mountPath"] = new_cert
            volume_mount_dict[v]["name"] = volume_mount_dict[v]["name"] + str(i+1)
        # Update key section
        if "key" in volume_mount_dict[v]["name"] and "ca-cert" not in volume_mount_dict[v]["name"]:
            key_temp = volume_mount_dict[v]["mountPath"].split('_key')[0]
            new_key = re.sub(r'\d+', '', key_temp) + str(i+1) + '_key'
            volume_mount_dict[v]["mountPath"] = new_key
            volume_mount_dict[v]["name"] = volume_mount_dict[v]["name"] + str(i+1)
    # Updating env part of k8s yaml dict
    env_dict = yaml_dict["spec"]["template"]["spec"]["containers"][0]["env"]
    for v in range(len(env_dict)):
        # Update AppName
        if env_dict[v]["name"] == "AppName":
            env_dict[v]["value"] = env_dict[v]["value"] + str(i+1)
        # Update CONFIGMGR_CERT & CONFIGMGR_KEY section
        if env_dict[v]["name"] == "CONFIGMGR_CERT" or env_dict[v]["name"] == "CONFIGMGR_KEY":
            app_name = env_dict[v]["value"].split("/")[-1].split("_")[0]
            new_app_name = app_name + str(i+1)
            env_dict[v]["value"] = env_dict[v]["value"].replace(app_name, new_app_name)
        # Update tcp ports section
        if "ENDPOINT" in env_dict[v]["name"] and ":" in env_dict[v]["value"]:
            # Updating ports for PUBLISHER/SERVER ENDPOINTS
            if "SERVER" in env_dict[v]["name"] or "PUBLISHER" in env_dict[v]["name"]:
                port = env_dict[v]["value"].split(":")[-1]
                new_port = get_available_port(port, used_ports_dict["send_ports"])
                env_dict[v]["value"] = env_dict[v]["value"].replace(port, new_port)
            # Updating ports, appname for SUBSCRIBER/CLIENT ENDPOINTS
            if "SUBSCRIBER" in env_dict[v]["name"] or "CLIENT" in env_dict[v]["name"]:
                port = env_dict[v]["value"].split(":")[-1]
                new_port = get_available_port(port, used_ports_dict["recv_ports"])
                env_dict[v]["value"] = env_dict[v]["value"].replace(port, new_port)
                # Update appnames in ENDPOINTS
                appname = env_dict[v]["value"].split(":")[0]
                env_dict[v]["value"] = env_dict[v]["value"].replace(appname, appname + str(i+1))
                # Update endpoint name if it has a unique name
                if env_dict[v]["name"].count("_") > 1:
                    ep_name = env_dict[v]["name"].split("_")[1]
                    env_dict[v]["name"] = env_dict[v]["name"].replace(ep_name, ep_name + str(i+1))
    # Updating volumes section of k8s yaml dict
    volume_dict = yaml_dict["spec"]["template"]["spec"]["volumes"]
    for v in range(len(volume_dict)):
        # Update cert section
        if "cert" in volume_dict[v]["name"] and "ca-cert" not in volume_dict[v]["name"]:
            volume_dict[v]["name"] = volume_dict[v]["name"] + str(i+1)
            app_name = volume_dict[v]["secret"]["secretName"].split("-")[0]
            new_app_name = app_name + str(i+1)
            volume_dict[v]["secret"]["secretName"] = volume_dict[v]["secret"]["secretName"].replace(app_name, new_app_name)
        # Update key section
        if "key" in volume_dict[v]["name"]:
            volume_dict[v]["name"] = volume_dict[v]["name"] + str(i+1)
            app_name = volume_dict[v]["secret"]["secretName"].split("-")[0]
            new_app_name = app_name + str(i+1)
            volume_dict[v]["secret"]["secretName"] = volume_dict[v]["secret"]["secretName"].replace(app_name, new_app_name)
    return yaml_dict


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
            yaml_dict_srvc['metadata']['name'] = yaml_dict_srvc['metadata']['name'] + str(i+1)
            yaml_dict_srvc['spec']['selector']['app'] = yaml_dict_srvc['spec']['selector']['app'] + str(i+1)
            # Updating ports in service section
            ports_dict = yaml_dict_srvc['spec']['ports']
            for v in range(len(ports_dict)):
                # Update port
                if "port" in ports_dict[v]:
                    if "$" not in str(ports_dict[v]['port']):
                        port = ports_dict[v]['port']
                        new_port = get_available_port(str(port), used_ports_dict['srvc_ports'])
                        ports_dict[v]['port'] = int(new_port)
                    # Update targetPort if it exists
                    if "targetPort" in ports_dict[v]:
                        if "$" not in str(ports_dict[v]['targetPort']):
                            ports_dict[v]['targetPort'] = int(new_port)
                    # Update nodePort if it exists
                    if "nodePort" in ports_dict[v]:
                        if "$" not in str(ports_dict[v]['nodePort']):
                            port = ports_dict[v]['nodePort']
                            new_port = get_available_port(str(port), used_ports_dict['srvc_ports'])
                            ports_dict[v]['nodePort'] = int(new_port)

            # dict to update deployment section
            yaml_dict = dict()
            for k, v in yaml_prod.items():
                yaml_dict[k] = v

            # Update deployment section app name
            yaml_dict["metadata"]["labels"]["app"] = yaml_dict["metadata"]["labels"]["app"] + str(i+1)
            yaml_dict["metadata"]["name"] = yaml_dict["metadata"]["name"] + str(i+1)
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
            yaml_dict["metadata"]["labels"]["app"] = yaml_dict["metadata"]["labels"]["app"] + str(i+1)
            yaml_dict["metadata"]["name"] = yaml_dict["metadata"]["name"] + str(i+1)
            # Update deployment section
            yaml_dict = multi_instance_k8s_deployment(yaml_dict, i)

            # Merging deployment section updates
            kube_yaml = ruamel.yaml.round_trip_dump(yaml_dict)
            # Creating final k8s yml
            data = kube_yaml
        # Updating yml based on dev_mode
        if dev_mode:
            merged_yaml = merged_yaml + "---\n" + k8s_yaml_remove_secrets(data)
        else:
            merged_yaml = merged_yaml + "---\n" + data

    return merged_yaml


def k8s_yaml_merger(app_list, dev_mode, args):
    """Method merges the k8s yml files of each eis
       modules and generates a consolidated ymlfile.

    :param app_list: List of services
    :type app_list: list
    :param dev_mode: Dev Mode key
    :type dev_mode: bool
    """
    app_list.extend(override_k8s_apps_list)
    merged_yaml = ""
    # Iterate through & create multi instance k8s yml
    # for apps having override directory
    for kube_yaml in app_list:
        app_name = kube_yaml.split("/")[-2]
        if args.override_directory is not None:
            if args.override_directory in kube_yaml:
                app_name = kube_yaml.split("/")[-3]
        if num_multi_instances > 1 and app_name not in subscriber_list.keys():
            for i in range(num_multi_instances):
                multi_instance_yml = create_multi_instance_k8s_yml(kube_yaml, dev_mode, i)
                merged_yaml = merged_yaml + "---\n" + multi_instance_yml
        else:
            with open(kube_yaml) as yaml_file:
                data = yaml_file.read()
                if dev_mode:
                    merged_yaml = merged_yaml + "---\n" + k8s_yaml_remove_secrets(data)
                else:
                    merged_yaml = merged_yaml + "---\n" + data


    # Iterate through app_list & merge k8s yaml
    for kube_yaml in app_list:
        with open(kube_yaml) as yaml_file:
            data = yaml_file.read()
            if dev_mode:
                merged_yaml = merged_yaml + "---\n" + k8s_yaml_remove_secrets(data)
            else:
                merged_yaml = merged_yaml + "---\n" + data
    k8s_service_yaml = './k8s/eis-k8s-deploy.yml'
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
                # Update publisher config in eis_builder_cfg
                for k4, v4 in list(v['environment'].items()):
                    if k4 == 'AppName':
                        app_name = v4
                    # Add cfg of all publishers to eis_builder_cfg
                    if k4 == 'PubTopics':
                        if app_name in eis_builder_cfg[
                           'pub_endpoint_cfg'].keys():
                            eis_builder_cfg['pub_endpoint_cfg'][app_name] = \
                                {v4: v['environment'][v4 + '_cfg']}
                # Add cfg of all publishers to eis_builder_cfg
                for k4, v4 in list(v['environment'].items()):
                    # Add SubTopics is not available in subscribers
                    # update SubTopics & stream cfg from publisher cfg
                    if 'SubTopics' not in dict(v['environment']).keys() and\
                       app_name in eis_builder_cfg['subscriber_list'].keys():
                        for cfg_val in eis_builder_cfg[
                                'pub_endpoint_cfg'].keys():
                            if eis_builder_cfg[
                                    'pub_endpoint_cfg'][cfg_val] != {}:
                                for k5, v5 in eis_builder_cfg[
                                        'pub_endpoint_cfg'][cfg_val].items():
                                    v['environment']['SubTopics'] =\
                                        cfg_val + '/' + k5
                                    v['environment'][k5 + '_cfg'] = v5
                for k4, v4 in list(v['environment'].items()):
                    # Update AppName
                    if k4 == 'AppName':
                        v['environment'][k4] = v4 + str(i)
                        app_name = v4
                        new_app_name = v4 + str(i)
                    # Update Clients section in publishers
                    if k4 == 'Clients':
                        for x in bm_apps_list:
                            if x in v['environment'][k4]:
                                v['environment'][k4] = v4.replace(x,
                                                                  x + '1,'
                                                                  + x + str(i))
                    # Update PubTopics SubTopics
                    # If topics contain an integer, increment it
                    # if topics don't contain integer, suffix one
                    if k4 == 'PubTopics':
                        if bool(re.search(r'\d', v['environment'][k4])):
                            v['environment'][k4] = re.sub(r'\d+', str(i),
                                                          v['environment'][k4])
                        else:
                            v['environment'][k4] = v['environment'][k4] +\
                                str(i)
                    # Update clients in SubTopics
                    if k4 == 'SubTopics':
                        for x in bm_apps_list:
                            if x in v['environment'][k4]:
                                v['environment'][k4] = v4.replace(x,
                                                                  x + str(i))
                    # Update topic names in SubTopics
                    if k4 == 'SubTopics':
                        if bool(re.search(r'\d', v['environment'][k4])):
                            v['environment'][k4] = re.sub(r'\d+', str(i),
                                                          v['environment'][k4])
                        else:
                            v['environment'][k4] = v['environment'][k4] +\
                                str(i)
                    # Update cfg of every topic
                    if '_cfg' in k4:
                        # If tcp mode, increment port number
                        if 'tcp' in v['environment'][k4]:
                            port = v['environment'][k4].split(':')[1]
                            new_port = str(int(port) + (i-1))
                            v4 = v4.replace(port, new_port)
                        new_cfg = re.sub(r'\d+', str(i), k4)
                        if k4 in v['environment'].keys():
                            del v['environment'][k4]
                        v['environment'][new_cfg] = v4

    # Update outer secrets section
    if not dev_mode:
        if 'secrets' in temp.keys():
            for k, v in list(temp['secrets'].items()):
                if 'cert' in k:
                    cert_temp = k.split('_cert')[0]
                    new_cert = re.sub(r'\d+', '', cert_temp) + str(i) + '_cert'
                    for k2 in v.items():
                        yml_map = ruamel.yaml.comments.CommentedMap()
                        yml_map.insert(1, 'file', list(k2)[1].replace(app_name,
                                                                    new_app_name))
                    temp['secrets'][new_cert] = yml_map
                    del temp['secrets'][k]
                if 'key' in k:
                    key_temp = k.split('_key')[0]
                    new_key = re.sub(r'\d+', '', key_temp) + str(i) + '_key'
                    for k2 in v.items():
                        yml_map = ruamel.yaml.comments.CommentedMap()
                        yml_map.insert(1, 'file', list(k2)[1].replace(app_name,
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
                # Create single instance only for services in subscriber_list and 
                # for corner case of common/video, create multi instance otherwise
                # TODO: Support EISAzureBridge multi instance creation if applicable
                if appname not in subscriber_list.keys() and appname != "video" and appname != "common" and appname != "EISAzureBridge":
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
                    for service_keys, _ in service_dict.items():
                        if(service_keys == "secrets"):
                            del service_dict[service_keys]
    return yaml_dict


def create_docker_compose_override(app_list, dev_mode, args):
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

        with open("./docker-compose.override.yml", 'w') as fp:
            ruamel.yaml.round_trip_dump(override_data, fp)


def yaml_parser(args):
    """Yaml parser method.

    :param args: cli arguments
    :type args: argparse
    """

    # Fetching EIS directory path
    eis_dir = os.getcwd() + '/../'
    dir_list = []
    if args.yml_file is not None:
        # Fetching list of subdirectories from yaml file
        print("Fetching required services from {}...".format(args.yml_file))
        with open(args.yml_file, 'r') as sub_dir_file:
            yaml_data = ruamel.yaml.round_trip_load(sub_dir_file,
                                                    preserve_quotes=True)
            for service in yaml_data['AppName']:
                # In case user mentions the service as full path instead of
                # relative to IEdgeInsights.
                if service.startswith("/"):
                    prefix_path = service
                else:
                    prefix_path = eis_dir + service
                if os.path.isdir(prefix_path) or os.path.islink(prefix_path):
                    dir_list.append(service)
    else:
        # Fetching list of subdirectories
        print("Parsing through directory to fetch required services...")
        dir_list = [f.name for f in os.scandir(eis_dir) if
                    (f.is_dir() or os.path.islink(f))]
        dir_list = sorted(dir_list)

    # Adding video folder manually since it's not a direct sub-directory
    if os.path.isdir(eis_dir + 'common/video'):
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
            prefix_path = eis_dir + app_dir
        # Append to app_list if dir has both docker-compose.yml and config.json
        # & append to override_apps_list if override_directory has both
        # docker-compose.yml and config.json
        if args.override_directory is not None:
            override_dir = prefix_path + '/' + args.override_directory
            if os.path.isdir(override_dir):
                if os.path.isfile(override_dir + '/docker-compose.yml') and \
                   os.path.isfile(override_dir + '/config.json'):
                    override_apps_list.append(override_dir)
            elif os.path.isfile(prefix_path + '/docker-compose.yml') and\
                 os.path.isfile(prefix_path + '/config.json'):
                app_list.append(prefix_path)
        else:
            if os.path.isfile(prefix_path + '/docker-compose.yml') and \
               os.path.isfile(prefix_path + '/config.json'):
                app_list.append(prefix_path)
        # Append to csl_app_list if dir has both app_spec.json and
        # module_spec.json & append to override_csl_apps_list if
        # override_directory has both app_spec.json & module_spec.json
        if args.override_directory is not None:
            override_dir = prefix_path + '/' + args.override_directory
            if os.path.isdir(override_dir):
                if os.path.isfile(override_dir + '/app_spec.json') and \
                   os.path.isfile(override_dir + '/module_spec.json'):
                    override_csl_apps_list.append(override_dir)
            elif os.path.isfile(prefix_path + '/app_spec.json') and\
                 os.path.isfile(prefix_path + '/module_spec.json'):
                csl_app_list.append(prefix_path)
        else:
            if os.path.isfile(prefix_path + '/app_spec.json') and \
               os.path.isfile(prefix_path + '/module_spec.json'):
                csl_app_list.append(prefix_path)
        # Append to k8s_app_list if dir has k8s-service.yml
        # & append to override_k8s_apps_list if
        # override_directory has k8s-service.yml
        if args.override_directory is not None:
            override_dir = prefix_path + '/' + args.override_directory
            if os.path.isdir(override_dir):
                if os.path.isfile(override_dir + '/k8s-service.yml'):
                    override_k8s_apps_list.append(override_dir + '/k8s-service.yml')
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
    if os.path.isdir(eis_dir + 'common/video'):
        app_list.insert(0, eis_dir + 'common/video')

    # Fetching DEV_MODE from .env
    dev_mode = False
    with open(".env") as f:
        for line in f:
            if line.startswith('DEV_MODE'):
                dev_mode = line.strip().split('=')[1]
                dev_mode = util.strtobool(dev_mode)
                break

    yml_dict = update_yml_dict(app_list, 'docker-compose.yml', dev_mode, args)

    with open(DOCKER_COMPOSE_PATH, 'w') as docker_compose_file:
        ruamel.yaml.round_trip_dump(yml_dict, docker_compose_file)

    dev_mode_str = "PROD"
    if dev_mode:
        dev_mode_str = "DEV"
        # Creating docker-compose-dev.override.yml only in DEV mode
        create_docker_compose_override(dev_override_list, True, args)
    print("Successfully created docker-compose.yml"
          " file for {} mode".format(dev_mode_str))

    # Starting json parser
    json_parser(DOCKER_COMPOSE_PATH, args)

    # Sourcing required env from .env & provision/.env
    source_env("./.env")
    source_env("./provision/.env")

    # Starting csl json parser
    csl_parser(csl_app_list, args)

    # Generating Consolidated k8s yaml file for deployment
    try:
        k8s_yaml_merger(k8s_app_list, dev_mode, args)
        print("Successfully created consolidated Kubernetes "
              "deployment yml at ./k8s/eis-k8s-deploy.yml")
    except Exception as e:
        print("Exception Occured at Kubernetes yml generation {}".format(e))
        sys.exit(1)


def parse_args():
    """Parse command line arguments.
    """
    arg_parse = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    arg_parse.add_argument('-f', '--yml_file', default=None,
                           help='Optional config file for list of services'
                           ' to include.\
                           Eg: python3.6 eis_builder.py -f\
                           video-streaming.yml')
    arg_parse.add_argument('-v', '--video_pipeline_instances', default=1,
                           help='Optional number of video pipeline '
                                'instances to be created.\
                           Eg: python3.6 eis_builder.py -v 6')
    arg_parse.add_argument('-d', '--override_directory',
                           default=None,
                           help='Optional directory consisting of '
                           'of benchmarking configs to be present in'
                           'each app directory.\
                           Eg: python3.6 eis_builder.py -d benchmarking')
    return arg_parse.parse_args()


if __name__ == '__main__':

    # fetching eis_builder config
    with open('eis_builder_config.json', 'r') as config_file,\
         open('eis_builder_schema.json', 'r') as schema_file:
        eis_builder_cfg = json.load(config_file)
        eis_builder_schema = json.load(schema_file)
        try:
            validate(instance=eis_builder_cfg, schema=eis_builder_schema)
        except Exception as e:
            print("JSON schema validation failed {}".format(e))
            sys.exit(1)
        # list of publishers and their endpoints
        publisher_list = eis_builder_cfg['publisher_list']
        # list of subscribers
        subscriber_list = eis_builder_cfg['subscriber_list']
        # To store the csl endpoint json
        csl_pub_endpoint_list = eis_builder_cfg['csl_pub_endpoint_list']
        # To store the endpoint cfg
        eis_builder_cfg['pub_endpoint_cfg'] =\
            eis_builder_cfg['csl_pub_endpoint_list']

    # Parse command line arguments
    args = parse_args()

    # Setting number of multi instances variable
    if int(args.video_pipeline_instances) > 1:
        num_multi_instances = int(args.video_pipeline_instances)

    # Start yaml parser
    yaml_parser(args)
