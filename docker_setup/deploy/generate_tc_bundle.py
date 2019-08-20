import yaml
import subprocess
import json
import os


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
            self.env = self.get_env_dict()
            if self.env['DOCKER_REGISTRY'] is '' or \
                    "/" not in self.env['DOCKER_REGISTRY']:
                print("Please Check the Docker Regsitry Address in \
                'DOCKER_REGISTRY' env of docker_setup\\.env file")
                os._exit(1)

    def get_env_dict(self):
        '''
            This method reads the .env file and returns as
            dict values
        '''
        with open("../.env") as f:
            envList = f.readlines()

        envDict = {}
        for env in envList:
            if env.strip() and '#' not in env.strip()[0]:
                envVal = env.split('=')
                envDict[envVal[0]] = envVal[1].rstrip()
        return envDict

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
            # Remove Build Option & depends_on Option from docker-compose
            for service in self.config['services'].keys():
                if 'build' in self.config['services'][service].keys():
                    del self.config['services'][service]['build']
                if 'depends_on' in self.config['services'][service].keys():
                    del self.config['services'][service]['depends_on']

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
        cmdlist.append("cp -rf ../Certificates/ ./eis_installer")
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
