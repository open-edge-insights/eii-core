#!/usr/bin/python3

"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""


import argparse
import subprocess
import os
import tarfile
from distutils.dir_util import copy_tree

SYSTEMD_PATH = "/etc/systemd/system"
INSTALL_PATH = "/opt/intel/iei/"


def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument('-c', '--create_iei_pkg',
                        help='Creates a IEI package', action="store_true")

    parser.add_argument('-i', '--install_iei', help='install iei pkg',
                        action="store_true")

    parser.add_argument('-p', '--path_to_certs_dir',
                        help='Directory path of certificates')

    parser.add_argument('-u', '--uninstall_iei', help='uninstall iei pkg',
                        action="store_true")

    parser.add_argument('-a', '--add_iei_without_tar',
                        help='Install iei in the target system without any \
                        container tar file',
                        action="store_true")

    args = parser.parse_args()
    if args.install_iei and not args.path_to_certs_dir:
        parser.error("-i/--install_iei requires \"-p\" option")
    
    return args


def enable_systemd_service():
    # Copy the systemd service file to /etc/systemd/system
    print("Copying the service file to systemd path...")
    os.chdir("{0}{1}".format(INSTALL_PATH, "deploy/"))
    output = subprocess.run(["cp", "iei.service", SYSTEMD_PATH])
    if output.returncode != 0:
        print("Unable to copy the systemd service file")
        exit(-1)

    # This is to keep systemctl DB sane after adding IEI service.
    print("systemctl daemon reload in progress...")
    output = subprocess.run(["systemctl", "daemon-reload"])
    if output.returncode != 0:
        print("Failed to execute systemctl daemon-reload")
        exit(-1)

    os.chdir("{0}{1}".format(INSTALL_PATH, "deploy/"))
    print("systemctl start of IEI service in progress...")
    output = subprocess.run(["systemctl", "start", "iei"])
    if output.returncode != 0:
        print("Unable to start IEI systemd service")
        exit(-1)

    print("systemctl enable of IEI service in progress...")
    output = subprocess.run(["systemctl", "enable", "iei"])
    if output.returncode != 0:
        print("Unable to add IEI systemd service as part of boot")
        exit(-1)


def create_install_dir():
    print("Creating {0} before install if it doesn't exist...".format(
        INSTALL_PATH))
    output = subprocess.run(["mkdir", "-p", INSTALL_PATH])
    if output.returncode != 0:
        print("Unable to create install path" + INSTALL_PATH)
        exit(-1)


def copy_docker_setup_files():
    # Copy docker setup files to install path
    # TODO: Check if this function is really needed or not.
    #       Can we get rid of it.
    try:
        copy_tree(os.getcwd(), INSTALL_PATH)
        print("docker setup files copied to %s path" % INSTALL_PATH)
    except OSError as e:
        print('Directory docker_setup failed to be copied. Error: %s' % e)
        exit(-1)


def execute_compose_startup():

    os.chdir("../")
    copy_docker_setup_files()

    output = subprocess.run(["./compose_startup.sh", "--build", "|", "tee",
                            "compose_startup.txt"])
    if output.returncode != 0:
        print("Failed to run compose_startup.sh successfully")
        exit(-1)

    try:
        subprocess.run(["sudo", "./create_client_dist_package.sh"])
    except Exception as e:
        print("Failed to copy dist_files. Error: %s" % e)
        exit(-1)


def uninstall_iei():
    print("***********Un-installing IEI***********")
    uninstall_list = ["/opt/intel/iei/",
                      "/etc/systemd/system/iei.service"]
    print("systemctl stop of IEI service in progress...")
    output = subprocess.run(["systemctl", "stop", "iei"])
    if output.returncode != 0:
        print("Unable to stop IEI systemd service file")

    cwd = os.getcwd()
    os.chdir(INSTALL_PATH)
    for i in uninstall_list:
        print("Removing "+i+" ...")
        if i == "/opt/intel/iei/":
            keepers = ['secret_store', 'tpm_secret']
            for filename in os.listdir('.'):
                if filename not in keepers:
                    print('Removing %s' % (filename,))
                    output = subprocess.run(["rm", "-rf", filename])
            if output.returncode != 0:
                print("Unable to remove" + i)
        else:
            output = subprocess.run(["rm", "-rf", i])
            if output.returncode != 0:
                print("Unable to remove" + i)
    os.chdir(cwd)

    # This is to keep systemctl DB sane after deleting IEI service.
    print("systemctl daemon reload in progress...")
    output = subprocess.run(["systemctl", "daemon-reload"])
    if output.returncode != 0:
        print("Unable to do systemd daemon-reload")

    print("***********Finished Un-installing IEI***********")


if __name__ == '__main__':
    args = parse_args()

    if args.path_to_certs_dir:
        CERT_PATH = os.path.abspath(args.path_to_certs_dir)

    docker_setup_tar_name = "docker_setup.tar.gz"
    test_videos_tar_name = "test_videos.tar.gz"
    dist_libs_tar_name = "dist_libs.tar.gz"

    if args.add_iei_without_tar:
        # uninstall previous iei instance and files
        if os.path.exists(INSTALL_PATH):
            uninstall_iei()
        print("*****Installing IEI without any pre-created container*******")
        execute_compose_startup()
        enable_systemd_service()
        print("***********Installation Finished***********")

    if args.create_iei_pkg:
        print("******Preparing IEI tarball for deployment box********")
        if os.path.exists(INSTALL_PATH):
            uninstall_iei()
        execute_compose_startup()

        # Save the images
        output = subprocess.run(["./deploy/save_built_images.sh", "|", "tee",
                                "docker_save_log.txt"])
        if output.returncode != 0:
            print("Failed to run save_built_images.sh successfully")
            exit(-1)

        print("***Preparing compressed archive of IEI package***")
        docker_setup_dir = os.getcwd()
        os.chdir(INSTALL_PATH + "dist_libs/")
        dist_libs_dir = os.getcwd()
        dist_libs_path = docker_setup_dir + "/deploy/" + \
            dist_libs_tar_name
        print("Preparing {0}".format(dist_libs_path))
        with tarfile.open(dist_libs_path, "w:gz") as tar:
            for file in os.listdir(dist_libs_dir):
                tar.add(file)

        os.chdir(docker_setup_dir)
        docker_setup_path = docker_setup_dir + "/deploy/" + \
            docker_setup_tar_name
        print("Preparing {0}".format(docker_setup_path))
        with tarfile.open(docker_setup_path, "w:gz") as tar:
            for file in os.listdir(docker_setup_dir):
                tar.add(file)

        test_videos_path = docker_setup_dir + "/deploy/" + \
            test_videos_tar_name
        test_videos_dir = os.getcwd() + "/../test_videos/"
        os.chdir(test_videos_dir)  # IEI root directory

        if os.path.exists(os.getcwd()):
            print("Preparing {0}".format(test_videos_path))
            print(test_videos_dir)
            with tarfile.open(test_videos_path, "w:gz") as tar:
                for file in os.listdir(test_videos_dir):
                    tar.add(file)

        iei_tar_name = "iei.tar.gz"
        iei_path = docker_setup_dir + "/deploy/" + \
            iei_tar_name
        iei_setup_file = "setup_iei.py"
        print("Preparing {0}. It consists of {1}, {2} and {3}".format(
              iei_path, test_videos_tar_name, docker_setup_tar_name,
              iei_setup_file))
        os.chdir("../docker_setup/deploy")
        with tarfile.open(iei_path, "w:gz") as tar:
            tar.add(dist_libs_tar_name)
            tar.add(test_videos_tar_name)
            tar.add(docker_setup_tar_name)
            tar.add(iei_setup_file)

        for file in [docker_setup_path, test_videos_path, dist_libs_path]:
            print("Removing {0}...".format(file))
            if os.path.exists(file):
                os.remove(file)

        install_guide = '''
        Completed preparing the compressed archive for IEI package.
        Kindly copy the archive name "iei.tar.gz" to the deployment
        system's prefered directory.

        extarct the tar ball in prefered directory and execute below
        command:
            ./setup_iei.py -i Or  ./setup_iei.py --install_iei

        This will install necessary files in "/opt/intel/iei" folder.
        Additionally it will create systemd service having name "iei"

        If the installation is successful, one can start/stop iei
        service by excuting below:
            systemctl stop iei
            systemctl start iei
            systemctl status iei
        '''
        print(install_guide)

    if args.install_iei:
        # uninstall previous iei instance and files
        if os.path.exists(INSTALL_PATH):
            uninstall_iei()
        create_install_dir()

        # untar the docker setup files
        for file in [docker_setup_tar_name, test_videos_tar_name,
                     dist_libs_tar_name]:
            print("Extracting {0} at {1}".format(file, INSTALL_PATH))
            with tarfile.open(file) as tar:
                if "test_videos" in file:
                    tar.extractall(path=(INSTALL_PATH + "/test_videos"))
                elif "dist_libs" in file:
                    tar.extractall(path=(INSTALL_PATH + "/dist_libs"))
                else:
                    tar.extractall(path=INSTALL_PATH)

        os.chdir(INSTALL_PATH)
        # Load the images
        output = subprocess.run(["./deploy/load_built_images.sh", "|", "tee",
                                 "docker_compose_load_built_images_log.txt"])
        if output.returncode != 0:
            print("Failed to run load_built_images.sh successfully")
            exit(-1)

        print(CERT_PATH)
        print("Provisioning the system with certificates provided by user")
        output = subprocess.run(["./provision_startup.sh", CERT_PATH,
                                 "deploy_mode",
                                 "|", "tee", "provision_iei_log.txt"])
        if output.returncode != 0:
            print("Failed to run provision_startup.sh successfully")
            exit(-1)

        print("Creating systemd entry for iei and enabling it ...")
        enable_systemd_service()

        print("***********Installation Finished***********")

    if args.uninstall_iei:
        uninstall_iei()
