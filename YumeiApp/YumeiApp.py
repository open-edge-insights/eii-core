

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

from pymodbus.client.sync import ModbusTcpClient as ModbusClient
from StreamSubLib.StreamSubLib import StreamSubLib
from Util.log import configure_logging, LOG_LEVELS
import logging
import json
import ast
import argparse
import os


class YumeiApp:
    '''This Class controls the robotic_arm, PanelLight,
    AlarmLight and ResetButton'''

    def __init__(self, args, log):
        ''' Reads the config file and connects
        to the io_module'''
        self.args = args
        self.log = log

        with open(args.config, 'r') as f:
            self.config = json.load(f)

        self.ip = self.config["io_module_ip"]
        self.port = self.config["io_module_port"]
        self.modbus_client = ModbusClient(
            self.ip, self.port, timeout=1, retry_on_empty=True)

    def light_ctrl_cb(self, classified_result_data):
        ''' Controls the Alarm Light, i.e., alarm light turns on
        if there are any defects in the classified results
        Argument: classifier_results from influxdb
        Output: Turns on the Alarm Light
        '''

        self.log.info("data received is : " + classified_result_data)
        classified_result_data = ast.literal_eval(classified_result_data)
        json_data = json.dumps(classified_result_data)
        classified_result_data = json.loads(json_data)

        if 'defects' in classified_result_data:
            if ast.literal_eval(classified_result_data['defects']):
                # write_coil(regester address, bit value)
                # bit value will be stored in the register address
                # bit value is either 1 or 0
                # 1 is on and 0 is off
                self.modbus_client.write_coil(self.config["bit_register"], 1)

    def reset_ctrl_cb(self, io_module_data):
        ''' Controls the Reset Button, i.e., once the reset
        button is clicked, alarm light turns off
        Argement: Data of io_module from influxdb
        Output: Turns off the Alarm Light
        '''

        io_module_data = ast.literal_eval(io_module_data)
        json_data = json.dumps(io_module_data)
        io_module_data = json.loads(json_data)
        if (io_module_data[self.config["reset_button_in"]] == "false"):
            self.log.info("Reset Clicked")
            self.modbus_client.write_coil(self.config["bit_register"], 0)

    def main(self):
        ''' Subscription to the required Streams
        in influxdb'''

        try:
            ret = self.modbus_client.connect()
            self.log.info("Modbus connect on %s:%s returned %s" % (
                self.ip, self.port, ret))

            streamSubLib = StreamSubLib()
            streamSubLib.init()

            streamSubLib.Subscribe('module_io', self.reset_ctrl_cb)
            streamSubLib.Subscribe('classifier_results', self.light_ctrl_cb)
        except Exception as e:
            self.log.info("Exception Occured" + str(e))


def parse_args():
    '''Parse command line arguments
    '''

    parser = argparse.ArgumentParser()
    parser.add_argument('--config', default='yumeiapp_config.json',
                        help='JSON configuration file')

    parser.add_argument('--log', dest='log', choices=LOG_LEVELS.keys(),
                        default='DEBUG', help='Logging level (df: DEFAULT)')

    parser.add_argument('--log-name', dest='log_name',
                        default='yumei_app_logs', help='Logfile name')

    parser.add_argument('--log-dir', dest='log_dir', default='logs',
                        help='Directory to for log files')

    return parser.parse_args()

if __name__ == "__main__":
    args = parse_args()

    if not os.path.exists(args.log_dir):
        os.mkdir(args.log_dir)

    log = configure_logging(args.log.upper(), args.log_name,
                            args.log_dir, __name__)

    yumeiApp = YumeiApp(args, log)
    yumeiApp.main()
