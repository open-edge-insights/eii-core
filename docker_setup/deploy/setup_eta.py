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
INSTALL_PATH = "/opt/intel/eta/"


def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument('-c', '--create_eta_pkg',
                        help='Creates a ETA package', action="store_true")

    parser.add_argument('-i', '--install_eta', help='install eta pkg',
                        action="store_true")

    parser.add_argument('-u', '--uninstall_eta', help='uninstall eta pkg',
                        action="store_true")

    parser.add_argument('-a', '--add_eta_without_tar',
                        help='Install eta in the target system without any \
                        container tar file',
                        action="store_true")

    return parser.parse_args()


def enable_systemd_service():
    # Copy the systemd service file to /etc/systemd/system
    print("Copying the service file to systemd path...")
    os.chdir("{0}{1}".format(INSTALL_PATH, "deploy/"))
    output = subprocess.run(["cp", "eta.service", SYSTEMD_PATH])
    if output.returncode != 0:
        print("Unable to copy the systemd service file")
        exit(-1)

    # This is to keep systemctl DB sane after adding ETA service.
    print("systemctl daemon reload in progress...")
    output = subprocess.run(["systemctl", "daemon-reload"])
    if output.returncode != 0:
        print("Failed to execute systemctl daemon-reload")
        exit(-1)

    os.chdir("{0}{1}".format(INSTALL_PATH, "deploy/"))
    print("systemctl start of ETA service in progress...")
    output = subprocess.run(["systemctl", "start", "eta"])
    if output.returncode != 0:
        print("Unable to start ETA systemd service")
        exit(-1)

    print("systemctl enable of ETA service in progress...")
    output = subprocess.run(["systemctl", "enable", "eta"])
    if output.returncode != 0:
        print("Unable to add ETA systemd service as part of boot")
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
    try:
        copy_tree(os.getcwd(), INSTALL_PATH)
        print("docker setup files copied to %s path" % INSTALL_PATH)
    except OSError as e:
        print('Directory docker_setup failed to be copied. Error: %s' % e)
        exit(-1)


def execute_compose_startup():

    os.chdir("../")
    copy_docker_setup_files()

    output = subprocess.run(["./compose_startup.sh", "0", "|", "tee",
                            "compose_startup.txt"])
    if output.returncode != 0:
        print("Failed to run compose_startup.sh successfully")
        exit(-1)


def uninstall_eta():
    print("***********Un-installing ETA***********")
    uninstall_list = ["/opt/intel/eta/",
                      "/etc/systemd/system/eta.service"]
    print("systemctl stop of ETA service in progress...")
    output = subprocess.run(["systemctl", "stop", "eta"])
    if output.returncode != 0:
        print("Unable to stop ETA systemd service file")

    cwd = os.getcwd()
    os.chdir("/opt/intel/eta/")
    for i in uninstall_list:
        print("Removing "+i+" ...")
        if i == "/opt/intel/eta/":
            keepers = ['secret_store']
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

    # This is to keep systemctl DB sane after deleting ETA service.
    print("systemctl daemon reload in progress...")
    output = subprocess.run(["systemctl", "daemon-reload"])
    if output.returncode != 0:
        print("Unable to do systemd daemon-reload")

    print("***********Finished Un-installing ETA***********")


if __name__ == '__main__':
    args = parse_args()
    docker_setup_tar_name = "docker_setup.tar.gz"
    test_videos_tar_name = "test_videos.tar.gz"
    dist_libs_tar_name = "dist_libs.tar.gz"

    if args.add_eta_without_tar:
        # uninstall previous eta instance and files
        if os.path.exists(INSTALL_PATH):
            uninstall_eta()
        print("*****Installing ETA without any pre-created container*******")
        execute_compose_startup()
        enable_systemd_service()
        print("***********Installation Finished***********")

    if args.create_eta_pkg:
        print("******Preparing ETA tarball for deployment box********")
        uninstall_eta()
        execute_compose_startup()

        # Save the images
        output = subprocess.run(["./deploy/save_built_images.sh", "|", "tee",
                                "docker_save_log.txt"])
        if output.returncode != 0:
            print("Failed to run save_built_images.sh successfully")
            exit(-1)

        print("***Preparing compressed archive of ETA package***")
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
        os.chdir(test_videos_dir)  # ETA root directory

        if os.path.exists(os.getcwd()):
            print("Preparing {0}".format(test_videos_path))
            print(test_videos_dir)
            with tarfile.open(test_videos_path, "w:gz") as tar:
                for file in os.listdir(test_videos_dir):
                    tar.add(file)

        eta_tar_name = "eta.tar.gz"
        eta_path = docker_setup_dir + "/deploy/" + \
            eta_tar_name
        eta_setup_file = "setup_eta.py"
        print("Preparing {0}. It consists of {1}, {2} and {3}".format(
              eta_path, test_videos_tar_name, docker_setup_tar_name,
              eta_setup_file))
        os.chdir("../docker_setup/deploy")
        with tarfile.open(eta_path, "w:gz") as tar:
            tar.add(dist_libs_tar_name)
            tar.add(test_videos_tar_name)
            tar.add(docker_setup_tar_name)
            tar.add(eta_setup_file)

        for file in [docker_setup_path, test_videos_path, dist_libs_path]:
            print("Removing {0}...".format(file))
            if os.path.exists(file):
                os.remove(file)

        install_guide = '''
        Completed preparing the compressed archive for ETA package.
        Kindly copy the archive name "eta.tar.gz" to the deployment
        system's prefered directory.

        extarct the tar ball in prefered directory and execute below
        command:
            ./setup_eta.py -i Or  ./setup_eta.py --install_eta

        This will install necessary files in "/opt/intel/eta" folder.
        Additionally it will create systemd service having name "eta"

        If the installation is successful, one can start/stop eta
        service by excuting below:
            systemctl stop eta
            systemctl start eta
            systemctl status eta
        '''
        print(install_guide)

    if args.install_eta:
        # uninstall previous eta instance and files
        uninstall_eta()
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

        print("Creating systemd entry for eta and enabling it ...")
        enable_systemd_service()

        print("***********Installation Finished***********")

    if args.uninstall_eta:
        uninstall_eta()
