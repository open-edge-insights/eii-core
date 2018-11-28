

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

import os
import time
import json
import serial
from ret_codes import ReturnCodes


class SerialLightController:
    def __init__(self):
        self.controllers = []

    def _openserial(self, port):
        errorcode = ReturnCodes.Good
        com_id = port[4:]
        com_param = [19200, 8, 'N', 1, 0, 0, 0, 0]
        try:
            ser_client = serial.Serial(baudrate=com_param[0],
                                       bytesize=com_param[1],
                                       parity=com_param[2],
                                       stopbits=com_param[3],
                                       timeout=com_param[4],
                                       dsrdtr=com_param[5],
                                       xonxoff=com_param[6],
                                       rtscts=com_param[7])
            ser_client.setPort(com_id)
            if not ser_client.isOpen():
                ser_client.open()
                if not ser_client.isOpen():
                    print('port %s could not open.' % com_id)
                    errorcode = ReturnCodes.SCPI_SerialOpenFail
        except:
            print('exception:failed to open serial port %s' % com_id)
            errorcode = ReturnCodes.SCPI_SerialOpenFail

        return errorcode, ser_client

    def _closeserial(self, port):
        errorcode = ReturnCodes.Good
        for ctrl in self.controllers:    
            if ctrl["channel"] == port:
                ser_client = ctrl["serialclient"]

        if not ser_client:
            print("port %s is not in open status!"%com_id)
            errorcode = ReturnCodes.SCPI_SerialNotOpen
            return errorcode
        try:
            ser_client.close()
            if not ser_client.isOpen():
                print("port %s is closed!"%com_id)
            else:
                print("port %s could not close!"%com_id)

        except:
            errorcode = ReturnCodes.SCPI_SerialCloseFail
            print('failed to close serial port %s' % port)            

        return errorcode

    def _sendserial(self, port, data):
        errorcode = ReturnCodes.Good
        com_id = port[4:]
        for ctrl in self.controllers:
            if ctrl["channel"] == port:
                ser_client = ctrl["serialclient"]

        print("ser_client", ser_client)
        try:
            if not ser_client.isOpen():            
                print("open a new serial port: %s" % com_id)
                ser_client.open()
                if not ser_client.isOpen():
                    print("port %s could not open." % com_id)
                    errorcode = ReturnCodes.SCPI_SerialOpenFail
                    return errorcode
        except:
            print('failed to open serial port %s' % port)
            errorcode = ReturnCodes.SCPI_SerialOpenFail
            return errorcode

        try:
            print("write to serial port: %s" % com_id)
            ser_client.write(data.encode("ascii"))
            response = ser_client.readline()
            print("response: ", response)
            response = response.decode("ascii")
            if not response:
                print("No serial device connected at port: %s" % com_id)
                errorcode = ReturnCodes.SCPI_SerialNoResponse
                return errorcode
            if response == "&":
                print("light controller response fails!")
                errorcode = 0x8000
                return errorcode
        except:
            print('failed to send data thru serial port %s' % port)
            errorcode = ReturnCodes.SCPI_SerialSendFail
            return errorcode

        return errorcode

    def _xorall(self, data):
        check = 0
        i = 0
        while i < 6:
            check ^= ord(data[i])
            i += 1
        result = data+str(hex(check))[2:]

        return result

    def lightctrl(self, port, op, brightlevel):
        errorcode = ReturnCodes.Good
        channel_in_use = False
        for ctrl in self.controllers:
            if port == ctrl["channel"]:
                print("channel %s is already in use" % port)
                channel_in_use = True
                ser_client = ctrl["serialclient"]

        if not channel_in_use:
            errorcode, ser_client = self._openserial(port)
            if errorcode != ReturnCodes.Good:
                print("Open serial port:%s fails" % port)
                return errorcode

            controller = {"channel": port, "serialclient": ser_client}
            self.controllers.append(controller)
            print("light controllers:%s" % self.controllers)

        if op == "on":
            cmd1 = "S200T200T200T200TC#"
            errorcode = self._sendserial(port, cmd1)
            print("cmd1: ", errorcode)
            if errorcode != ReturnCodes.Good:
                print('send data <%s> to port <%s> fails' % (cmd1, port))
                return errorcode

            '''
            cmd1="$310"
            cmd1+=str(hex(int(brightlevel)))[2:]
            cmd1=self._xorall(cmd1)
            cmd2="$110"
            cmd2+=str(hex(int(brightlevel)))[2:]
            cmd2=self._xorall(cmd2)
            errorcode = self._sendserial(port, cmd1)
            print("cmd1: ",errorcode)
            if errorcode != ReturnCodes.Good:
                print('send data <%s> to port <%s> fails' % (cmd1, port))
                return errorcode
            errorcode = self._sendserial(port, cmd2)
            print("cmd2: ",errorcode)
            if errorcode != ReturnCodes.Good:
                print('send data <%s> to port <%s> fails' % (cmd2, port))
                return errorcode
            '''
        elif op == "off":
            cmd1 = "S200F200F200F120FC#"
            errorcode = self._sendserial(port, cmd1)
            print("cmd1: ", errorcode)
            if errorcode != ReturnCodes.Good:
                print('send data <%s> to port <%s> fails' % (cmd1, port))
                return errorcode
            '''
            cmd1="$210291c"
            cmd2="$220291f"
            errorcode = self._sendserial(port, cmd1)
            print("cmd1 response: ",errorcode)
            if errorcode != ReturnCodes.Good:
                print('send data <%s> to port <%s> fails' % (cmd1, port))
                return errorcode            
            errorcode = self._sendserial(port, cmd2)            
            if errorcode != ReturnCodes.Good:
                print('send data <%s> to port <%s> fails' % (cmd2, port))
                return errorcode
            '''
        else:
            print('operation parameter:<%s>:is not recognizable' % op)
            errorcode = 0x9000
            return errorcode

        return errorcode