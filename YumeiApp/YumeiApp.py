

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
import argparse
import ast
import json
import os

import paho.mqtt.client as paho
import SerialLightController
from ret_codes import ReturnCodes


class YumeiApp:
    '''This Class controls the robotic_arm, PanelLight,
    AlarmLight and ResetButton'''

    def __init__(self, args, log):
        ''' Reads the config file and connects
        to the io_module'''
        self.args = args
        self.log = log

        self.robotic_arm = False
        # This is for lightController
        self.lightcontroller = SerialLightController.SerialLightController()

        with open(args.config, 'r') as f:
            self.config = json.load(f)

        self.ip = self.config["io_module_ip"]
        self.port = self.config["io_module_port"]
        self.modbus_client = ModbusClient(
            self.ip, self.port, timeout=1, retry_on_empty=True)

    def panel_light_ctrl(self, port, op, brightlevel):
        self.log.info("light controllers: " + str(self.lightcontroller.controllers))
        errorcode = self.lightcontroller.lightctrl(port, op, brightlevel)
        self.log.info("LIGHT OPERATION COMPLETE WITH CODE: " + str(errorcode))
        if errorcode == ReturnCodes.Good:
            result = "Light control operation done!"
            self.log.info(result)
        else:
            result = "Light control operation fails!"
            self.log.info(result)
        json_out = {"code": str(errorcode), "data": result}
        return json_out

    def light_ctrl_cb(self, classified_result_data):
        ''' Controls the Alarm Light, i.e., alarm light turns on
        if there are any defects in the classified results
        Argument: classifier_results from influxdb
        Output: Turns on the Alarm Light
        '''

        classified_result_data = json.loads(classified_result_data)
        # self.log.info("data received is : " + classified_result_data)
        defect_types = []
        if 'defects' in classified_result_data:
            if classified_result_data['defects']:
                classified_result_data['defects'] = ast.literal_eval(
                    classified_result_data['defects'])

                for i in classified_result_data['defects']:
                    defect_types.append(i['type'])

                if (1 in defect_types) or (2 in defect_types) or \
                   (3 in defect_types) or (0 in defect_types):
                    # write_coil(regester address, bit value)
                    # bit value will be stored in the register address
                    # bit value is either 1 or 0
                    # 1 is on and 0 is off
                    self.modbus_client.write_coil(
                        self.config["bit_register"], 1)

    def sendCamOnTrigger(self):
        """
            This will send Cam On Trigger Data to Mqtt
        """
        data = {"camera_on": 1}
        json_data = json.dumps(data)
        try:
            self.client.publish(self.config["cam_on_topic"], payload=json_data, qos=1)
        except Exception as e:
            self.log.info("Exception Occured " + str(e))

    def sendCamOffTrigger(self):
        """
            This will send Cam Off Trigger Data to Mqtt
        """
        data = {"camera_on": 0}
        json_data = json.dumps(data)
        try:
            self.client.publish(self.config["cam_on_topic"], payload=json_data, qos=1)
        except Exception as e:
            self.log.info("Exception Occured " + str(e))

    def onsubScribeIoModuleData(self, io_module_data):
        ''' Controls the Reset Button, i.e., once the reset
        button is clicked, alarm light turns off
        Argement: Data of io_module from influxdb
        Output: Turns off the Alarm Light
        '''
        io_module_data = json.loads(io_module_data)

        if (io_module_data[self.config["reset_button_in"]] == "false"):
            self.log.info("Reset Clicked")
            self.modbus_client.write_coil(self.config["bit_register"], 0)

        if (io_module_data[self.config["robotic_arm_in"]] == "false" and not self.robotic_arm):
            self.robotic_arm = True
            self.log.info("Robotic Arm is On")
            try:
                panelLight = self.panel_light_ctrl([self.config["panel_light_1_port"],\
                                                   self.config["panel_light_2_port"]],\
                                                   "on", [self.config["panel_light_1_bright"],\
                                                    self.config["panel_light_2_bright"]])
                self.log.info(str(panelLight))
            except Exception as e:
                self.log.info("Exception Occured " + str(e))

            self.sendCamOnTrigger()

        if (io_module_data[self.config["robotic_arm_in"]] == "true" and self.robotic_arm):
            self.robotic_arm = False
            self.log.info("Robotic Arm is Off")

            self.sendCamOffTrigger()

            try:
                panelLight = self.panel_light_ctrl([self.config["panel_light_1_port"],\
                                                   self.config["panel_light_2_port"]],\
                                                   "off",\
                                                   [self.config["panel_light_1_bright"],\
                                                   self.config["panel_light_2_bright"]])
                self.log.info(str(panelLight))
            except Exception as e:
                self.log.info("Exception Occured " + str(e))

    def main(self):
        ''' Subscription to the required Streams
        in influxdb'''

        try:
            ret = self.modbus_client.connect()
            self.log.info("Modbus connect on %s:%s returned %s" % (
                self.ip, self.port, ret))

            self.client = paho.Client("client-001")
            self.client.connect(self.config["mqtt_broker"])

            streamSubLib = StreamSubLib()
            streamSubLib.init()

            streamSubLib.Subscribe('module_io', self.onsubScribeIoModuleData)
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
