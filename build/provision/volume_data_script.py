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
"""Script to list out and remove all the docker volumes created by
   EIS services.
"""
import subprocess
import yaml

DOCKER_COMPOSE_PATH = "../docker-compose.yml"


def _execute_cmd(cmd):
    cmd_output = subprocess.check_output(cmd)
    return cmd_output


def _execute_vol_rm_cmds(volume):
    """function to return list of volumes
    """
    cmd1 = subprocess.run(["docker", "volume", "ls"],
                          stdout=subprocess.PIPE, check=False)
    cmd2 = subprocess.run(["grep", volume], input=cmd1.stdout,
                          stdout=subprocess.PIPE, check=False)
    cmd = subprocess.run(["wc", "-l"], input=cmd2.stdout,
                         stdout=subprocess.PIPE, check=False)
    cmd = cmd.stdout.decode('utf-8').rstrip("\n")
    return cmd


def get_volume_info(filepath):
    """function to get all the volume info

    :param filepath: docker-compose.yml file path
    :type: str
    """
    with open(filepath) as compose_file:
        docs = yaml.load_all(compose_file, Loader=yaml.FullLoader)
        for doc in docs:
            for key, value in doc.items():
                if key == "volumes":
                    list_of_volumes = list(value.keys())

    str_edge = "edgeinsightssoftware_"
    str_edge += '{0}'
    list_of_volumes = [str_edge.format(i) for i in list_of_volumes]

    for vol in list_of_volumes:
        cmd = _execute_vol_rm_cmds(vol)

        if int(cmd) == 1:
            print("Cleaning volume:", vol)
            try:
                _execute_cmd(["docker", "volume", "rm", vol])
            except subprocess.CalledProcessError as err:
                print("Subprocess error: {}, {}".format(err.returncode,
                                                        err.output))

    command = _execute_vol_rm_cmds(str_edge.format("vol_etcd"))

    if int(command) == 1:
        try:
            _execute_cmd(["docker", "kill", "ia_etcd"])
            _execute_cmd(["docker", "rm", "-f", "ia_etcd"])
            _execute_cmd(["docker", "volume", "rm",
                          str_edge.format("vol_etcd")])
        except subprocess.CalledProcessError as err:
            print("Subprocess error: {}, {}".format(err.returncode,
                                                    err.output))


if __name__ == "__main__":

    get_volume_info(DOCKER_COMPOSE_PATH)
