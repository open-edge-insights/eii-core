

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


class ReturnCodes(object):
    Good = 0

    # for SCPI Serial Plugin
    SCPI_SerialOpenFail = 0x00002000
    SCPI_SerialCloseFail = 0x00002001
    SCPI_SerialNotOpen = 0x00002002
    SCPI_SerialNoResponse = 0x00002003
    SCPI_SerialNoInited = 0x00002004
    SCPI_SerialSendFail = 0x00002005
    SCPI_SerialListStateFail = 0x00002006
    SCPI_SerialReadStateException = 0x00002007

    #for SCPI Tcp Plugin
    SCPI_TcpOpenException = 0x00003000
    SCPI_TcpCloseFail = 0x00003001
    SCPI_TcpCloseException = 0x00003002
    SCPI_TcpNoInited = 0x00003003
    SCPI_TcpSendException = 0x00003004
    SCPI_TcpListStateFail = 0x00003005
    SCPI_TcpReadStateException = 0x00003006

    # for Plugin Global
    PLUGIN_ParamError = 0x00004000
    PLUGIN_RpcError = 0x00004001
    # for user definition
