#!/usr/bin/env python
# coding: utf-8
from twisted.internet.protocol import Protocol,Factory
from twisted.protocols.basic import LineReceiver
from twisted.internet import reactor

class Chat(LineReceiver):
    def __init__(self):
        self.name = None

    def connectionMade(self):
        self.sendLine('welcome, what is you name!')

    def lineReceived(self,line):
        if self.name:
            self.handle_chat(line)
        else:
            self.handle_name(line)

    def handle_chat(self,line):
        self.sendLine("we are chating")

    def handle_name(self,line):
        if line in self.factory.names:
            self.sendLine('name used, please choose an other')
        else:
            self.name = line
            self.factory.names[self.name] = self
            self.sendLine("hi %s" % line)

    def connectionLost(self,reason):
        if self.name != None:
            del self.factory.names[self.name]
        self.name = None

class ChatFactory(Factory):
    def __init__(self,protocol):
        self.protocol = protocol
        self.names = {}

    def buildFactory(self):
        return self.protocol()

if __name__ == '__main__':
    reactor.listenTCP(9527,ChatFactory(Chat))
    print 'svrd started'
    reactor.run()
    print 'svrd stopped'

