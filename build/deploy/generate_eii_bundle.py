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

    """ docker save images to ./docker_images folder """
    def docker_save(self, docker_load=False):
        cmdlist = []
        pwd = os.getcwd()
        self.docker_images_dir =  pwd + "/" + self.bundle_tag_name + "/docker_images"
        try:
            cmdlist.append(["mkdir", "-p", self.bundle_tag_name])
            cmdlist.append(["mkdir", "-p", self.docker_images_dir])
            for service in self.config['services'].keys():
                print("docker service to save:{} ".format(service))
                cmdlist.append(["docker", "save", "-o",
                        self.docker_images_dir + "/" + service + ".tar",
                        service + ":" + self.eii_version])
            if docker_load:
                cmdlist.append(["cp", "-rf", "docker_load.py", self.bundle_tag_name])
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
        eii_cert_dir = "./" + self.bundle_tag_name + "/provision/Certificates/"
        cmdlist = []
        cmdlist.append(["mkdir", "-p", self.bundle_tag_name])
        cmdlist.append(["cp", "../.env", self.bundle_tag_name])
        cmdlist.append(["mv", "docker-compose.yml", self.bundle_tag_name])
        if self.env["DEV_MODE"] == "false":
            cmdlist.append(["mkdir", "-p", eii_cert_dir + "/ca"])
            for service in self.config['services'].keys():
                if 'environment' not in self.config['services'][service]:
                    break

                servicename =\
                    self.config['services'][service]['environment']['AppName']
                service_dir = eii_cert_dir + servicename

                # TODO: enable etcd_ui only in leader node
                if "ia_etcd_ui" in self.include_services:
                    cmdlist.append(["cp", "-rf", "../provision/Certificates/root", eii_cert_dir])

                cmdlist.append(["mkdir", "-p", service_dir])
                cert_dir = "../provision/Certificates/" + servicename
                cmdlist.append(["cp", "-rf", cert_dir, eii_cert_dir])
            print("Here Appending Certificates")
            cmdlist.append(["cp", "-rf", "../provision/Certificates/ca",
                            eii_cert_dir])
            ca_key_file = eii_cert_dir + "ca/ca_key.pem"
            cmdlist.append(["rm", ca_key_file])
        try:
            for cmd in cmdlist:
                subprocess.check_output(cmd)
            env = open(self.bundle_tag_name + "/.env", "r+")
            envdata = env.read()
            newenvdata = envdata.replace("ETCD_NAME=leader",
                                         "ETCD_NAME=worker")
            env.write(newenvdata)
            env.close()
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

    def generate_provision_bundle(self, node='worker'):
        '''
            generate_provision bundle helps to execute set of pre
            commands which are required for provision Bundle and finally
            it generates the bundle
        '''
        provision_tag_name = 'leader_provisioning'
        if node is 'worker':
            provision_tag_name = 'worker_provisioning'

        provision_dir = "./" + provision_tag_name + "/provision/"
        eii_cert_dir = "./" + provision_tag_name + "/provision/Certificates/"
        dep_dir = "./" + provision_tag_name + "/provision/dep/"
        cmdlist = []
        cmdlist.append(["rm", "-rf", provision_tag_name])
        cmdlist.append(["mkdir", "-p", provision_tag_name])
        cmdlist.append(["cp", "../.env", provision_tag_name])
        cmdlist.append(["mkdir", "-p", provision_dir])
        
        try:
            if node is 'leader':
                cmdlist.append(["mkdir", "-p", dep_dir])
                cmdlist.append(["cp", "-f", "../provision/dep/docker-compose-etcd.yml",
                                dep_dir])
                cmdlist.append(["cp", "-f", "../provision/dep/Dockerfile",
                                dep_dir])
                cmdlist.append(["cp", "-f", "../provision/dep/start_etcd.sh",
                                dep_dir])

                if self.env["DEV_MODE"] == "false":
                    cmdlist.append(["mkdir", "-p", eii_cert_dir + "/ca"])
                    cmdlist.append(["cp", "-rf", "../provision/Certificates/ca/ca_certificate.pem",
                                    eii_cert_dir + "/ca"])
                    cmdlist.append(["cp", "-rf", "../provision/Certificates/etcdserver",
                                    eii_cert_dir])
                    cmdlist.append(["cp", "-f", "../provision/dep/docker-compose-etcd.override.prod.yml",
                                    dep_dir])
            cmdlist.append(["cp", "-f", "../provision/provision.sh",
                            provision_dir])
            cmdlist.append(["chmod", "+x", provision_dir + "provision.sh"])

            for cmd in cmdlist:
                subprocess.check_output(cmd)

            if node is 'worker':
                env = open(provision_tag_name + "/.env", "r+")
                envdata = env.read()
                newenvdata = envdata.replace("ETCD_NAME=leader",
                                            "ETCD_NAME=worker")
                env.write(newenvdata)
                env.close()
            cmdlist = []
            tar_file = provision_tag_name + ".tar.gz"
            cmdlist.append(["tar", "-czvf", tar_file, provision_tag_name])
            if self.bundle_folder is False:
                cmdlist.append(["rm", "-rf", provision_tag_name])

            for cmd in cmdlist:
                subprocess.check_output(cmd)
            print("Provisioning Bundle Generated Succesfully")
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
        self.bundle_tag_name = args.bundle_tag_name
        self.docker_file_path = args.compose_file_path
        self.bundle_folder = args.bundle_folder

        if args.eii_version:
            self.eii_version = args.eii_version
        if args.leader:
            self.generate_provision_bundle("leader")
        elif args.worker:
            self.generate_provision_bundle("worker")
        elif args.config:
            config_services = args.config
            str_config = config_services.replace("\'", "\"")
            config = json.loads(str_config)
            self.include_services = config['include_services']
            self.generate_docker_composeyml()
            self.add_docker_save_option(args.docker_save, args.docker_load)
            self.generate_eii_bundle()
        elif args.usecase:
            self.generate_docker_composeyml(usecase=True)
            self.add_docker_save_option(args.docker_save, args.docker_load)
            self.generate_eii_bundle()
        else:
            self.generate_docker_composeyml()
            if args.provisioning:
                self.generate_provision_bundle()
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
    parser.add_argument('-p',
                        '--provisioning',
                        action='store_true',
                        help='Generates provisioning bundle')

    parser.add_argument('-w',
                        '--worker',
                        action='store_true',
                        help='Generate eii_bundle for worker node')

    parser.add_argument('-l',
                        '--leader',
                        action='store_true',
                        help='Generate provision_bundle for leader node')

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
