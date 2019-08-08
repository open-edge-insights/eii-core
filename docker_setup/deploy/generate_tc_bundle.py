import yaml
import subprocess
import json


class TurtleCreekBundleGenerator:

    def __init__(self):
        '''
            Intial Configuration will be initialised here.
        '''
        with open('config.json', 'r') as utilconfig:
            config = json.load(utilconfig)
            self.docker_compose_file_version = \
                config['docker_compose_file_version']
            self.exclude_services = config['exclude_services']
            self.docker_registry = config['docker_registry']

    def generate_docker_composeyml(self):
        '''
            This method helps to generate the docker-compose yaml
            file from EIS "docker-setup" directory and manipualtes
            new compose yaml file as per config file which will be
            part of telit bundle
        '''
        try:
            with open("../docker-compose.yml", 'r') as ymlfile:
                self.config = yaml.load(ymlfile)
            self.config['version'] = self.docker_compose_file_version
            # Remove Unwanted Services for Build
            for service in self.exclude_services:
                del self.config['services'][service]
            # Remove Build Option & Append docker_registry Name
            for service in self.config['services'].keys():
                self.config['services'][service]['image'] =\
                    self.docker_registry + '/' +\
                    self.config['services'][service]['image']
                del self.config['services'][service]['build']
            # Write the Docker Compose File to System
            with open("docker-compose.yml", 'w') as tcconf:
                yaml.dump(self.config, tcconf, default_flow_style=False)
        except Exception as e:
            print("Exception Occured", e)

    def generate_tutle_creebundle(self):
        '''
            generateTelitBundle helps to execute set of pre
            commands which is required for TC Bundle and finally
            it generates the bundle
        '''
        cmdlist = []
        cmdlist.append("mkdir -p eis_installer")
        cmdlist.append("mv docker-compose.yml ./eis_installer/")
        cmdlist.append("cp -rf ../config ./eis_installer/")
        cmdlist.append("cp ../.env ./eis_installer/")
        cmdlist.append("cp -rf ../test_videos/ ./eis_installer")
        cmdlist.append("tar -czvf eis_installer.tar.gz ./eis_installer")
        cmdlist.append("rm -rf eis_installer/")
        try:
            for cmd in cmdlist:
                subprocess.check_output(cmd, shell=True)
            print("TurtleCreek Bundle Generated Succesfully")
        except Exception as e:
            print("Exception Occured ", e.message)

tcBundle = TurtleCreekBundleGenerator()
tcBundle.generate_docker_composeyml()
tcBundle.generate_tutle_creebundle()
