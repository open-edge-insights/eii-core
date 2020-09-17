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
benchmark_app_list = []
benchmark_csl_app_list = []


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
        if 'rtspsrc' in config['ingestor']['pipeline']:
            port = config['ingestor']['pipeline'].\
                split(":", 2)[2].split("/")[0]
            new_port = str(int(port) + i)
            config['ingestor']['pipeline'] =\
                config['ingestor']['pipeline'].\
                replace(port, new_port)

    return config


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
        rel_dir = data['services'][key]['build']['context'].replace(
            '$PWD/..', '..')
        app_list.append(rel_dir)

    # Removing duplicates & unwanted dirs from app list
    app_list = list(dict.fromkeys(app_list))
    app_list.remove(SCAN_DIR + '/common')

    eis_config_path = "./provision/config/eis_config.json"
    # Fetching and merging individual App configs
    for app_path in app_list:
        data = {}
        with open(app_path + '/config.json', "rb") as infile:
            # Merging config & interfaces from all services
            head = json.load(infile)
            config_json = merge(config_json, head)

    # Creating multiple boiler-plate app configs
    if args.override_directory is not None:
        for x in benchmark_app_list:
            appname = x.split('/' + args.override_directory)[0].split('/')[-1]
            for k in list(config_json):
                data = {}
                if appname in k:
                    del config_json[k]
                    for i in range(int(args.video_pipeline_instances)):
                        with open(x + '/config.json', "rb") as infile:
                            head = json.load(infile)
                            # Increments rtsp port number if required
                            head = increment_rtsp_port(appname, head, i)
                            data['/' + appname + str(i+1) + '/config'] = head
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
    # Fetching individual app names from benchmark_app_list
    bm_appnames_list = []
    for x in benchmark_app_list:
        appname = x.split('/' + args.override_directory)[0].split('/')[-1]
        bm_appnames_list.append(appname)

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
        for x in bm_appnames_list:
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

    bm_app_list = []
    for app in benchmark_csl_app_list:
        app_name = app.split('/' + args.override_directory)[0].\
            split('/')[-1]
        bm_app_list.append(app_name)

    # Creating deploy AppSpec from individual AppSpecs
    all_link_backend = []
    for app in app_list:
        if args.override_directory is not None:
            app_name = app.split('/' + args.override_directory)[0].\
                split('/')[-1]
        with open(app + '/app_spec.json', "rb") as infile:
            json_file = json.load(infile)
            head = json_file["Module"]
            # Generate boilerplate config for benchmark apps
            if args.override_directory is not None:
                if app_name in bm_app_list:
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
                            #  keys of itself
                            if publisher != head["Name"]:
                                pub_list.append(csl_pub_endpoint_list[
                                    publisher])

                        # Matching the links based on the order
                        # [VA, InfluxDBConnector]. If a publisher is
                        # empty (i.e VA or Influx is not present)
                        # endpoint of the service which subscribe to VA
                        # or Influx will be removed
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
        if args.override_directory is not None:
            if args.override_directory in app:
                app_name = app.rsplit("/", 2)[1]
        else:
            app_name = app.rsplit("/", 1)[1]
        module_spec_path = "./csl/" + app_name + "_module_spec.json"
        app_path = app + "/module_spec.json"
        # Substituting sourced env in module specs
        cmd = subprocess.run(["cat", app_path], stdout=subprocess.PIPE,
                             check=False)

        # Updating module specs for multi instance
        if args.override_directory is not None:
            json_value = json.loads(cmd.stdout.decode())
            for i in range(int(args.video_pipeline_instances)):
                nm = json_value['DataStore']['DataBuckets'][0]['Name']
                nm_val = nm + str(i+1)
                json_value['DataStore']['DataBuckets'].append({"Name":
                                                               nm_val,
                                                               "Permissions":
                                                               "RO"})
            cmd = json.dumps(json_value).encode()
        try:
            if args.override_directory is not None:
                env_subst(app_path, module_spec_path)
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
    yaml_prod = ruamel.yaml.round_trip_load(string_io_data,preserve_quotes=True)
    yaml_dict = dict()
    for k, v in yaml_prod.items():
        yaml_dict[k] = v

    vol_removal_list = []
    for d in yaml_dict['spec']['volumes']:
        if 'cert' in d['name']:
            vol_removal_list.append(d)
        elif 'key' in d['name']:
            vol_removal_list.append(d)

    con_vol_removal_list = []
    for d in yaml_dict['spec']['containers'][0]['volumeMounts']:
        if 'cert' in d['name']:
            con_vol_removal_list.append(d)
        elif 'key' in d['name']:
            con_vol_removal_list.append(d)

    for r in vol_removal_list:
        yaml_dict['spec']['volumes'].remove(r)
    for r in con_vol_removal_list:
        yaml_dict['spec']['containers'][0]['volumeMounts'].remove(r)

    kube_yaml = ruamel.yaml.round_trip_dump(yaml_dict)

    if "---" in yaml_data:
        kube_yaml = yaml_data_list[0] + "---\n" + kube_yaml

    return kube_yaml

def k8s_yaml_merger(app_list, dev_mode):
    """Method merges the k8s yml files of each eis
       modules and generates a consolidated ymlfile.

    :param app_list: List of services
    :type app_list: list
    :param dev_mode: Dev Mode key
    :type dev_mode: bool
    """

    merged_yaml = ""
    for kube_yaml in app_list:
        with open(kube_yaml) as yaml_file:
            data = yaml_file.read()
            if dev_mode:
                merged_yaml = merged_yaml + "---\n" + k8s_yaml_remove_secrets(data)
            else:
                merged_yaml = merged_yaml + "---\n" + data
    k8s_service_yaml = './k8s/eis-k8s-deploy.yml'
    with open (k8s_service_yaml, 'w') as final_yaml:
        final_yaml.write(merged_yaml)
    # Substituting sourced env in module specs
    env_subst(k8s_service_yaml,k8s_service_yaml)

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
    # Fetching individual app names from benchmark_app_list
    bm_appnames_list = []
    for x in benchmark_app_list:
        appname = x.split('/' + args.override_directory)[0].split('/')[-1]
        bm_appnames_list.append(appname)

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
                        for x in bm_appnames_list:
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
                        for x in bm_appnames_list:
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
    for k in app_list:
        with open(k + '/' + file_to_pick, 'r') as docker_compose_file:
            data = ruamel.yaml.round_trip_load(docker_compose_file,
                                               preserve_quotes=True)
            if args.override_directory is not None:
                if args.override_directory in k:
                    for i in range(int(args.video_pipeline_instances)):
                        data_two = create_multi_instance_yml_dict(data, i+1)
                        yaml_files_dict.append(data_two)
                else:
                    yaml_files_dict.append(data)
            else:
                yaml_files_dict.append(data)

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

    override_app_list = []
    app_list = []
    csl_app_list = []
    k8s_app_list = []
    for app_dir in dir_list:
        # In case user mentions the service as full path instead of
        # relative to IEdgeInsights.
        if app_dir.startswith("/"):
            prefix_path = app_dir
        else:
            prefix_path = eis_dir + app_dir
        # Append to app_list if dir has both docker-compose.yml and config.json
        if args.override_directory is not None:
            override_dir = prefix_path + '/' + args.override_directory
            if os.path.isdir(override_dir):
                if os.path.isfile(override_dir + '/docker-compose.yml') and \
                   os.path.isfile(override_dir + '/config.json'):
                    app_list.append(override_dir)
                    benchmark_app_list.append(override_dir)
        elif os.path.isfile(prefix_path + '/docker-compose.yml') and\
                os.path.isfile(prefix_path + '/config.json'):
            app_list.append(prefix_path)
        # Append to csl_app_list if dir has both app_spec.json and
        # module_spec.json
        if args.override_directory is not None:
            override_dir = prefix_path + '/' + args.override_directory
            if os.path.isdir(override_dir):
                if os.path.isfile(override_dir + '/app_spec.json') and \
                   os.path.isfile(override_dir + '/module_spec.json'):
                    csl_app_list.append(override_dir)
                    benchmark_csl_app_list.append(override_dir)
        elif os.path.isfile(prefix_path + '/app_spec.json') and\
                os.path.isfile(prefix_path + '/module_spec.json'):
            csl_app_list.append(prefix_path)
        # Append to override_list if dir has docker-compose-dev.override.yml
        if os.path.isfile(prefix_path + '/docker-compose-dev.override.yml'):
            override_app_list.append(prefix_path)

        # Prepare K8s Applist
        if os.path.isfile(prefix_path + '/k8s-service.yml'):
            k8s_app_list.append(prefix_path + '/k8s-service.yml')
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
        create_docker_compose_override(override_app_list, True, args)
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
        k8s_yaml_merger(k8s_app_list, dev_mode)
        print("Successfully created consolidated Kubernetes deployment yml at ./k8s/eis-k8s-deploy.yml")
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
        # To store the csl endpoint json
        csl_pub_endpoint_list = eis_builder_cfg['csl_pub_endpoint_list']
        # To store the endpoint cfg
        eis_builder_cfg['pub_endpoint_cfg'] =\
            eis_builder_cfg['csl_pub_endpoint_list']

    # Parse command line arguments
    args = parse_args()

    # Start yaml parser
    yaml_parser(args)
