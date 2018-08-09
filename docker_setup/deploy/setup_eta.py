#!/usr/bin/python3
import argparse
import subprocess
import os
import tarfile

SYSTEMD_PATH = "/etc/systemd/system"


def parse_args():
    parser = argparse.ArgumentParser()
    # TODO: Add --unnstall_ETA option
    parser.add_argument('-c', '--create_eta_pkg',
                        help='Creates a ETA package', action="store_true")

    parser.add_argument('-i', '--install_eta', help='install eta pkg',
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
    print("systemctl start of ETA service in progress...")
    output = subprocess.run(["systemctl", "start", "eta"])
    if output.returncode != 0:
        print("Unable to start ETA systemd service file")
        exit(-1)

    print("systemctl enable of ETA service in progress...")
    output = subprocess.run(["systemctl", "enable", "eta"])
    if output.returncode != 0:
        print("Unable to add ETA systemd service as part of boot")
        exit(-1)


if __name__ == '__main__':
    args = parse_args()
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
        output_filename = "var_lib_eta.tar.gz"
        source_dir = "/var/lib/eta/"
        with tarfile.open(output_filename, "w:gz") as tar:
            tar.add(source_dir, arcname=os.path.basename(source_dir))

        # Archive and zip docker setup folder
        # TODO: Will add version to it
        print("***Prparing compressed archive of ETA package***")
        cur_dir = os.getcwd()
        output_filename = "docker_setup.tar.gz"
        source_dir = os.path.join(cur_dir, "..", "..", "docker_setup")
        with tarfile.open(output_filename, "w:gz") as tar:
            tar.add(source_dir, arcname=os.path.basename(source_dir))

        output_filename = "eta.tar.gz"
        source_dir = os.path.join(cur_dir, "..", "..", "docker_setup")
        with tarfile.open(output_filename, "w:gz") as tar:
            tar.add("docker_setup.tar.gz", arcname="docker_setup.tar.gz")
            tar.add("setup_eta.py", arcname="setup_eta.py")

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
        install_path = "/opt/intel/eta/"
        # Untar the source
        print("Creating /opt/intel/eta before install if it doesn't exist...")
        output = subprocess.run(["mkdir", "-p", install_path])
        if output.returncode != 0:
            print("Unable to create install path" + install_path)
            exit(-1)

        print("Decompressing docker_setup in install path...")
        with tarfile.open('docker_setup.tar.gz') as tar:
            tar.extractall(path=install_path)

        print("Creatig systemd entry for eta and enabling it ...")
        enable_systemd_service()

        print("***********Installation Finished***********")
