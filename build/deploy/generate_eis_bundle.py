import yaml
import subprocess
import json
import sys
import argparse


class EisBundleGenerator:

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
            self.provisionenv = self.get_env_dict("../provision/.env")
            if self.provisionenv['PROVISION_MODE'] != "csl":
                if self.env['DOCKER_REGISTRY'] is '' or \
                        "/" not in self.env['DOCKER_REGISTRY']:
                    print("Please Check the Docker Regsitry Address in \
                    'DOCKER_REGISTRY' env of build\\.env file")
                    sys.exit(0)

    def get_env_dict(self, filepath):
        '''
            This method reads the .env file and returns as
            dict values
        '''
        with open(filepath) as f:
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
            with open(self.docker_file_path, 'r') as ymlfile:
                self.config = yaml.load(ymlfile, Loader=yaml.FullLoader)
            self.config['version'] = self.docker_compose_file_version

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
        except Exception as e:
            print("Exception Occured", e)

    def generate_eis_bundle_for_csl(self):
        '''
            generate EIS Bundle helps to execute set of pre
            commands which is required for EIS CSL worker node setup and finally
            it generates the bundle
        '''
        eis_provision_dir = "./" + self.bundle_tag_name + "/provision/"
        cmdlist = []
        cmdlist.append("rm -rf " + self.bundle_tag_name)
        cmdlist.append("mkdir -p " + self.bundle_tag_name)
        cmdlist.append("mv docker-compose.yml ./" + self.bundle_tag_name)
        cmdlist.append("cp ../.env ./" + self.bundle_tag_name)
        cmdlist.append("mkdir -p " + self.bundle_tag_name + "/provision")
        cmdlist.append("cp ../.env ./" + self.bundle_tag_name + "/provision")
        cmdlist.append("sudo chmod +x ../provision/provision_eis.sh")
        cmdlist.append("sudo cp -f " + "../provision/provision_eis.sh"
                                       + " " + eis_provision_dir)
        cmdlist.append("chown -R eisuser:eisuser ./" + self.bundle_tag_name)
        
        try:
            for cmd in cmdlist:
                subprocess.check_output(cmd, shell=True)
            env = open(self.bundle_tag_name + "/.env", "rt")
            envdata = env.read()
            newenvdata = envdata.replace("ETCD_NAME=master","ETCD_NAME=worker")
            env.close()
            newenv = open(self.bundle_tag_name + "/.env", "wt")
            newenv.write(newenvdata)
            newenv.close()
            cmdlist = []
            cmdlist.append("tar -czvf \
                " + self.bundle_tag_name + ".tar.gz ./" + self.bundle_tag_name)
            if self.bundle_folder is False:
                cmdlist.append("rm -rf " + self.bundle_tag_name + "/")
            else:
                pass
            
            if self.bundle_folder is False:
                cmdlist.append("rm -rf " + self.bundle_tag_name + "/")
            else:
                pass
            
            for cmd in cmdlist:
                subprocess.check_output(cmd, shell=True)
            print("EIS CSL Worker Node Setup Bundle Generated Succesfully")
        except Exception as e:
            print("Exception Occured ", str(e))

    def generate_eis_bundle_for_tc(self):
        '''
            generate_eis_bundle_for_tc helps to execute set of pre
            commands which is required for TC Bundle and finally
            it generates the bundle
        '''

        eis_cert_dir = "./" + self.bundle_tag_name + "/provision/Certificates/"
        eis_provision_dir = "./" + self.bundle_tag_name + "/provision/"
        cmdlist = []
        cmdlist.append("rm -rf " + self.bundle_tag_name)
        cmdlist.append("mkdir -p " + self.bundle_tag_name)
        cmdlist.append("mv docker-compose.yml ./" + self.bundle_tag_name)
        cmdlist.append("cp ../.env ./" + self.bundle_tag_name)
        cmdlist.append("mkdir -p " + self.bundle_tag_name + "/provision")
        if self.env["DEV_MODE"] == "false":
            for service in self.config['services'].keys():
                servicename =\
                    self.config['services'][service]['environment']['AppName']
                cmdlist.append("mkdir -p " + eis_cert_dir + servicename)
                cmdlist.append("cp -rf " +
                               "../provision/Certificates/" +
                               servicename + "/ " + eis_cert_dir)

            cmdlist.append("cp -rf " +
                           "../provision/Certificates/" +
                           "ca" + "/ " + eis_cert_dir)

            cmdlist.append("sudo rm " + eis_cert_dir + "ca/ca_key.pem")

        cmdlist.append("sudo chmod +x ../provision/provision_eis.sh")
        cmdlist.append("sudo cp -f " + "../provision/provision_eis.sh"
                                       + " " + eis_provision_dir)
        cmdlist.append("chown -R eisuser:eisuser ./" + self.bundle_tag_name)
        cmdlist.append("tar -czvf \
                " + self.bundle_tag_name + ".tar.gz ./" + self.bundle_tag_name)
        if self.bundle_folder is False:
            cmdlist.append("rm -rf " + self.bundle_tag_name + "/")
        else:
            pass
        try:
            for cmd in cmdlist:
                subprocess.check_output(cmd, shell=True)
            print("TurtleCreek Bundle Generated Succesfully")
        except Exception as e:
            print("Exception Occured ", str(e))
   
    def main(self, args):
        self.bundle_tag_name = args.bundle_tag_name
        self.docker_file_path = args.compose_file_path
        self.bundle_folder = args.bundle_folder
        self.generate_docker_composeyml()
        if self.provisionenv['PROVISION_MODE'] == "csl":
            self.generate_eis_bundle_for_csl()
        else:
            self.generate_eis_bundle_for_tc()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="EIS Bundle Generator:\
         This Utility helps to Generate the bundle required to deploy EIS.\
         ", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-f',
                        dest='compose_file_path',
                        default="../docker-compose.yml",
                        help='Docker Compose File path for EIS Deployment')
    parser.add_argument('-t',
                        dest='bundle_tag_name',
                        default="eis_bundle",
                        help='Tag Name used for Bundle Generation')
    parser.add_argument('-bf',
                        dest='bundle_folder',
                        default=False,
                        help='This Flag is set not to generate \
                        bundleName.tar.gz file.For Only Bundle folder')
    args = parser.parse_args()

    eisBundle = EisBundleGenerator()
    eisBundle.main(args)
