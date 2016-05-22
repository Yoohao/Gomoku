#!/usr/bin/env python
'''
import socket, sys
from pwn import *
ip = sys.argv[1]
port = 6666

sock = remote(ip, port)

try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
except socket.error, msg:
    sys.stderr.write("[ERROR] %s\n" % msg[1])
    sys.exit(1)

try:
    sock.connect((ip, port))
except socket.error, msg:
    sys.stderr.write("[ERROR] %s\n" % msg[1])
    exit(1)

print sock.recv(1024)
sock.close()
'''
from twisted.internet import stdio, reactor, protocol
from twisted.protocols import basic
import re,sys

class DataForwardingProtocol(protocol.Protocol):
    def __init__(self):
        self.output = None

    def dataReceived(self,data):
        if self.output:
            self.output.write(data)

class StdioProxyProtocol(DataForwardingProtocol):
    def connectionMade(self):
        inputForwarder = DataForwardingProtocol()
        inputForwarder.output = self.transport
        stdioWrapper = stdio.StandardIO(inputForwarder)
        self.output = stdioWrapper
        print "Connected to server"

class StdioProxyFactory(protocol.ClientFactory):
    protocol = StdioProxyProtocol

def main():
    reactor.connectTCP("127.0.0.1",8000,StdioProxyFactory( ))
    reactor.run()

if __name__ == '__main__':
    main()
