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
import shutil

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
    os.chdir("/opt/intel/eta/docker_setup/deploy/")
    output = subprocess.run(["cp", "eta.service", SYSTEMD_PATH])
    if output.returncode != 0:
        print("Unable to copy the systemd service file")
        exit(-1)
    # TODO: Need to copy var_lib_eta_tar dir also. Some investigation needed.

    # This is to keep systemctl DB sane after adding ETA service.
    print("systemctl daemon reload in progress...")
    output = subprocess.run(["systemctl", "daemon-reload"])
    if output.returncode != 0:
        print("Failed to execute systemctl daemon-reload")
        exit(-1)

    os.chdir("/opt/intel/eta/docker_setup/deploy/")
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
    print("Creating /opt/intel/eta before install if it doesn't exist...")
    output = subprocess.run(["mkdir", "-p", INSTALL_PATH])
    if output.returncode != 0:
        print("Unable to create install path" + INSTALL_PATH)
        exit(-1)


if __name__ == '__main__':
    args = parse_args()
    if args.add_eta_without_tar:
        print("*****Installing ETA without any pre-created container*******")

        os.chdir("../")
        output = subprocess.run(["./compose_startup.sh", "|", "tee",
                                 "compose_startup.txt"])
        if output.returncode != 0:
            print("Failed to run compose_startup.sh successfully")
            exit(-1)

        create_install_dir()

        # Copy docker setup files to install path
        # NOTE: Don't chnage the curent working Directory in code. else below
        # code may break.
        try:
            shutil.copytree(os.getcwd(), INSTALL_PATH + "docker_setup",
                            ignore=shutil.ignore_patterns('*.tar.gz', '*.tar'))
        except OSError as e:
            print('Directory docker_setup failed to be copied. Error: %s' % e)

        enable_systemd_service()

        print("***********Installation Finished***********")

    if args.create_eta_pkg:
        print("******Preparing ETA tarball for deloyment box********")

        os.chdir("../")
        output = subprocess.run(["./compose_startup.sh", "|", "tee",
                                 "compose_startup.txt"])
        if output.returncode != 0:
            print("Failed to run compose_startup.sh successfully")
            exit(-1)

        # Save the images
        output = subprocess.run(["./deploy/save_built_images.sh", "|", "tee",
                                 "docker_save_log.txt"])
        if output.returncode != 0:
            print("Failed to run save_built_images.sh successfully")
            exit(-1)

        # archive /var/lib/eta folder
        print("***Creating /var/lib/eta dir's compressed archive***")
        os.chdir("./deploy")
        var_lib_filename = "var_lib_eta.tar.gz"
        source_dir = "/var/lib/eta/"
        with tarfile.open(var_lib_filename, "w:gz") as tar:
            tar.add(source_dir, arcname=os.path.basename(source_dir))

        # Archive and zip docker setup folder
        # TODO: Will add version to it
        print("***Preparing compressed archive of ETA package***")
        cur_dir = os.getcwd()
        docker_setup_filename = "docker_setup.tar.gz"
        source_dir = os.path.join(cur_dir, "..", "..", "docker_setup")
        with tarfile.open(docker_setup_filename, "w:gz") as tar:
            tar.add(source_dir, arcname=os.path.basename(source_dir))

        output_filename = "eta.tar.gz"
        source_dir = os.path.join(cur_dir, "..", "..", "docker_setup")
        with tarfile.open(output_filename, "w:gz") as tar:
            tar.add("docker_setup.tar.gz", arcname="docker_setup.tar.gz")
            tar.add("setup_eta.py", arcname="setup_eta.py")

        print("Removing docker_setup.tar.gz and var_lib_eta.tar.gz files...")

        if os.path.exists(var_lib_filename):
            os.remove(var_lib_filename)

        if os.path.exists(docker_setup_filename):
            os.remove(docker_setup_filename)

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

        create_install_dir()

        # untar the docker setup files
        if not os.listdir(INSTALL_PATH):
            print("Decompressing docker_setup in install path...")
            with tarfile.open('docker_setup.tar.gz') as tar:
                tar.extractall(path=INSTALL_PATH)
        else:
            print("Found a pre-existing nonempty " + INSTALL_PATH +
                  " directory")
            print("Hence avoiding overwrite of it.")

        # Load the images
        os.chdir("/opt/intel/eta/docker_setup/")
        output = subprocess.run(["./deploy/deploy_compose_load.sh", "|", "tee",
                                 "docker_compose_load_log.txt"])
        if output.returncode != 0:
            print("Failed to run deploy_compose_load.sh successfully")
            exit(-1)

        print("Creating systemd entry for eta and enabling it ...")
        enable_systemd_service()

        print("***********Installation Finished***********")

    if args.uninstall_eta:
        uninstall_list = ["/var/lib/eta/", "/opt/intel/eta/",
                          "/etc/systemd/system/eta.service"]
        print("systemctl stop of ETA service in progress...")
        output = subprocess.run(["systemctl", "stop", "eta"])
        if output.returncode != 0:
            print("Unable to stop ETA systemd service file")
            exit(-1)

        for i in uninstall_list:
            print("Removing "+i+" ...")
            output = subprocess.run(["rm", "-rf", i])
            if output.returncode != 0:
                print("Unable to remove" + i)
                exit(-1)

        # This is to keep systemctl DB sane after deleting ETA service.
        print("systemctl daemon reload in progress...")
        output = subprocess.run(["systemctl", "daemon-reload"])
        if output.returncode != 0:
            print("Unable to do systemd daemon-reload")
            exit(-1)

        print("***********Finished Un-installing ETA***********")
