import open62541W
import argparse
import time
import os


def parse_arg():
    parser = argparse.ArgumentParser()
    parser.add_argument("-s", "--server", help="start server")
    parser.add_argument("-c", "--client", help="start client")
    return parser.parse_args()


def callback(topic, data):
    print("Inside py callback, topic=" + topic + " data=" + data)
    A.cbFunc(topic, data)


class A:
    @staticmethod
    def cbFunc(topic, msg):
        print("In A/DataBusTest.py...")
        print("Msg: {} received on topic: {}".format(msg, topic))

    def main(self):
        args = parse_arg()
        ns = "StreamManager"
        hostname = "localhost"
        topic = "classifier_results"
        caCert = ["/etc/ssl/grpc_internal/ca_cert.der"]
        serverCert = "/etc/ssl/opcua/opcua_server_certificate.der"
        serverKey = "/etc/ssl/opcua/opcua_server_key.der"
        clientCert = "/etc/ssl/opcua/opcua_client_certificate.der"
        clientKey = "/etc/ssl/opcua/opcua_client_key.der"
        port = 65003

        if args.server:
            errMsg = open62541W.py_serverContextCreate(hostname, port,
                                                       serverCert, serverKey,
                                                       caCert)
            if errMsg.decode() != "0":
                print("py_serverContextCreate() API failed!")
                os._exit(-1)
            print("py_serverContextCreate() API successful!")
            nsIndex = open62541W.py_serverStartTopic(ns, topic)
            if nsIndex == 100:
                print("py_serverStartTopic() API failed!")
                os._exit(-1)
            print("py_serverStartTopic() API successful!")
            result = ""
            for i in range(0, 100):
                result = "Hello " + str(i)
                errMsg = open62541W.py_serverPublish(nsIndex, topic, result)
                if errMsg.decode() != "0":
                    print("py_serverPublish() API failed!")
                    os._exit(-1)
                time.sleep(5)
        elif args.client:
            errMsg = open62541W.py_clientContextCreate(hostname, port,
                                                       clientCert, clientKey,
                                                       caCert)
            if errMsg.decode() != "0":
                print("py_clientContextCreate() API failed!")
                os._exit(-1)
            print("py_clientContextCreate() API successful")
            nsIndex = open62541W.py_clientStartTopic(ns, topic)
            if nsIndex == 100:
                print("py_clientStartTopic() API failed!")
                os._exit(-1)
            print("py_clientStartTopic() API successful!")
            errMsg = open62541W.py_clientSubscribe(nsIndex, topic, callback)
            if errMsg.decode() != "0":
                print("py_clientSubscribe() API failed!")
                os._exit(-1)
            print("py_clientSubscribe() API successful!")

            while True:
                pass


if __name__ == "__main__":
    a = A()
    a.main()
