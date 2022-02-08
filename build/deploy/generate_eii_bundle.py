# Copyright (c) 2020 Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
"""Script to generate eii bundle for worker node setup
   in Multi-node Provisioning & Deployment scenario.
"""
import subprocess
import json
import sys
import argparse
import yaml
from urllib.parse import unquote
import ast
import yaml
import os
import shlex

USER = "eiiuser:eiiuser"


class EiiBundleGenerator:
    """EiiBundleGenerator class
    """

    def __init__(self):
        '''
            Intial Configuration will be initialised here.
        '''
        with open('config.json', 'r') as utilconfig:
            config = json.load(utilconfig)
            self.docker_compose_file_version = \
                config['docker_compose_file_version']
            self.exclude_services = config['exclude_services']
            self.include_services = config['include_services']
            self.env = self.get_env_dict("../.env")
            self.eii_version = ""

    @classmethod
    def get_env_dict(cls, filepath):
        '''
            This method reads the .env file and returns as
            dict values
        '''
        with open(filepath) as env_file:
            env_list = env_file.readlines()

        env_dict = {}
        for env in env_list:
            if env.strip() and '#' not in env.strip()[0]:
                env_val = env.split('=')
                env_dict[env_val[0]] = env_val[1].rstrip()
        return env_dict

    def generate_docker_composeyml(self, usecase=False):
        '''
            This method helps to generate the docker-compose yaml
            file from EII "docker-setup" directory and manipualtes
            new compose yaml file as per config file which will be
            part of telit bundle
        '''
        try:
            with open(self.docker_file_path, 'r') as ymlfile:
                self.config = yaml.safe_load(ymlfile)
            self.config['version'] = self.docker_compose_file_version

            if usecase:
                # Include all services from the chosen builder usecase
                self.include_services = list(self.config['services'].keys())
            else:
                # exclude services if service isn't listed in include_services
                for service in self.config['services'].keys():
                    if service not in self.include_services:
                        self.exclude_services.append(service)

            # Remove Unwanted Services for Build
            for service in self.exclude_services:
                if service in self.config['services']:
                    del self.config['services'][service]

            # Remove Build Option & depends_on Option from docker-compose
            for service in self.config['services'].keys():
                if 'build' in self.config['services'][service].keys():
                    del self.config['services'][service]['build']
                if 'depends_on' in self.config['services'][service].keys():
                    del self.config['services'][service]['depends_on']

            # Write the Docker Compose File to System
            with open("docker-compose.yml", 'w') as tcconf:
                yaml.dump(self.config, tcconf, default_flow_style=False)
        except Exception as err:
            print("Exception Occured", err)
            sys.exit(0)

    def generate_config_json(self):
        self.inc_services = []
        self.update_config_file = {}
        for service in self.config['services'].keys():
            self.inc_services.append(self.config['services']
                    [service]['environment']['AppName'])
        with open('../eii_config.json', 'r') as cnf:
            self.cnf_file = json.load(cnf)
        self.conf_keys = self.cnf_file.keys()
        self.update_config_file.update({"/GlobalEnv/": self.cnf_file["/GlobalEnv/"]})
        for app_name_key in self.inc_services:
            for config_key in self.conf_keys:
                if app_name_key in config_key:
                    self.update_config_file.update({config_key: self.cnf_file[config_key]})
        with open('eii_config.json', 'w') as config:
            json.dump(self.update_config_file, config, indent=4)

    """ docker save images to ./docker_images folder """

    def docker_save(self, docker_load=False):
        cmdlist = []
        pwd = os.getcwd()
        self.docker_images_dir = pwd + "/" + self.bundle_tag_name + "/docker_images"
        try:
            cmdlist.append(["mkdir", "-p", self.bundle_tag_name])
            cmdlist.append(["mkdir", "-p", self.docker_images_dir])
            for service in self.config['services'].keys():
                image_name = service
                if "openedgeinsights" in self.config['services'][service]['image']:
                    image_name = "openedgeinsights/" + service
                cmdlist.append(["docker", "save", "-o",
                                self.docker_images_dir + "/" + service + ".tar",
                                image_name + ":" + self.eii_version])

            if docker_load:
                cmdlist.append(
                    ["cp", "-rf", "docker_load.py", self.bundle_tag_name])
            for cmd in cmdlist:
                print(cmd)
                subprocess.check_output(cmd)
        except Exception as err:
            print("Exception Occured ", str(err))
            sys.exit(0)

    def generate_eii_bundle(self):
        '''
            generate_eii_bundle helps to execute set of pre
            commands which are required for Bundle and finally
            it generates the bundle
        '''
        cmdlist = []
        cmdlist.append(["mkdir", "-p", self.bundle_tag_name])
        cmdlist.append(["cp", "../.env", self.bundle_tag_name])
        cmdlist.append(["mv", "docker-compose.yml", self.bundle_tag_name])
        try:
            for cmd in cmdlist:
                subprocess.check_output(cmd)

            cmdlist = []

            tar_file = self.bundle_tag_name + ".tar.gz"
            cmdlist.append(["chown", "-R", USER, self.bundle_tag_name])
            cmdlist.append(["tar", "-czvf", tar_file, self.bundle_tag_name])
            if self.bundle_folder is False:
                cmdlist.append(["rm", "-rf", self.bundle_tag_name])

            for cmd in cmdlist:
                subprocess.check_output(cmd)
            print("Bundle Generated Succesfully")
        except Exception as err:
            print("Exception Occured ", str(err))
            sys.exit(0)

    def generate_eii_raw_bundle(self):
        cmdlist = []
        cmdlist.append(["mkdir", "-p", self.bundle_tag_name])
        cmdlist.append(["cp", "../.env", self.bundle_tag_name])
        cmdlist.append(["cp", "../docker-compose.yml", self.bundle_tag_name])
        cmdlist.append(["cp", "../eii_config.json", self.bundle_tag_name])
        tar_file = self.bundle_tag_name + ".tar.gz"
        cmdlist.append(["tar", "-czvf", tar_file, self.bundle_tag_name])
        if self.bundle_folder is False:
            cmdlist.append(["rm", "-rf", self.bundle_tag_name])

        try:
            for cmd in cmdlist:
                subprocess.check_output(cmd)
            print("Raw Bundle Generated Succesfully")
        except Exception as err:
            print("Exception Occured ", str(err))
            sys.exit(0)

    def add_docker_save_option(self, dock_save, dock_load):
        if dock_save:
            if dock_load:
                self.docker_save(docker_load=True)
            else:
                self.docker_save()

    def main(self, args):
        """main function
        """
        self.bundle_tag_name = shlex.quote(args.bundle_tag_name)
        self.docker_file_path = shlex.quote(args.compose_file_path)
        self.bundle_folder = args.bundle_folder
        if args.generatebundle:
            self.generate_eii_raw_bundle()
            exit()
        if args.eii_version:
            self.eii_version = shlex.quote(args.eii_version)
        elif args.config:
            config_services = shlex.quote(args.config)
            str_config_path = config_services.replace("\'", "\"")
            with open(str_config_path) as config_file:
                str_config_file = config_file.read()
            config = json.loads(str_config_file)
            self.include_services = config['include_services']
            self.generate_docker_composeyml()
            self.generate_config_json()
            self.add_docker_save_option(args.docker_save, args.docker_load)
            self.generate_eii_bundle()
        elif args.usecase:
            self.generate_docker_composeyml(usecase=True)
            self.generate_config_json()
            self.add_docker_save_option(args.docker_save, args.docker_load)
            self.generate_eii_bundle()
        else:
            self.generate_docker_composeyml()
            self.generate_config_json()
            self.add_docker_save_option(args.docker_save, args.docker_load)
            self.generate_eii_bundle()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="EII Bundle Generator:\
         This Utility helps to Generate the bundle required to deploy EII.\
         ", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-f',
                        dest='compose_file_path',
                        default="../docker-compose.yml",
                        help='Docker Compose File path for EII Deployment')
    parser.add_argument('-t',
                        dest='bundle_tag_name',
                        default="eii_bundle",
                        help='Tag Name used for Bundle Generation')
    parser.add_argument('-bf',
                        dest='bundle_folder',
                        default=False,
                        help='This Flag is set not to generate bundle_tag_name\
                        folder, only bundle_tag_name.tar.gz file')

    parser.add_argument('-gb',
                        '--generatebundle',
                        action='store_true',
                        help='Generate bundle of docker-compose yml and .env file only')


    parser.add_argument('-c',
                        '--config',
                        dest='config',
                        help='config with include services for bundle generation')
    parser.add_argument('-u',
                        '--usecase',
                        action='store_true',
                        help='usecase to generate bundles for')
    parser.add_argument('-ds',
                        '--docker_save',
                        action='store_true',
                        help='docker save services')
    parser.add_argument('-dl',
                        '--docker_load',
                        action='store_true',
                        help='copy docker_load script to eii_bundle')
    parser.add_argument('-v',
                        '--eii_version',
                        dest='eii_version',
                        help='eii_version to save docker and load docker images')

    arg = parser.parse_args()
    eiiBundle = EiiBundleGenerator()
    eiiBundle.main(arg)
