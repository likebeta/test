#!/usr/bin/env python
# -*- coding=utf-8 -*-

# Author: likebeta <ixxoo.me@gmail.com>
# Create: 2015-10-11

import sys
import socket
import struct
import json
from twisted.internet.protocol import Protocol
from twisted.python import log
import logging
import stackless
from twisted.internet import reactor
from twisted.web import client


class TcpProtocol(Protocol):
    def __init__(self):
        self._data = ''

    def connectionMade(self):
        self.transport.setTcpNoDelay(1)
        self.transport.setTcpKeepAlive(1)
        try:
            sock = self.transport.getHandle()
            sock.setsockopt(socket.SOL_TCP, socket.TCP_KEEPINTVL, 30)
            sock.setsockopt(socket.SOL_TCP, socket.TCP_KEEPCNT, 10)
        except Exception, e:
            log.msg('not support TCP_KEEPIDLE', logLevel=logging.ERROR)

    def sendMessage(self, msg):
        try:
            data = json.dumps(msg)
            log.msg('==== SEND TCP:', repr(data))
            if self.transport and self.connected:
                header = struct.pack('I', len(data))
                self.transport.write(header + data)
                return True
            else:
                log.msg('==== ERROR: cannot connected !! protocol =', self, repr(data), logLevel=logging.ERROR)
        except Exception, e:
            import traceback
            traceback.print_exc()
            log.msg(msg, logLevel=logging.ERROR)

        return False

    def dataReceived(self, data):
        self._data += data
        while len(self._data) > 4:
            msg_len = struct.unpack('I', self._data[:4])[0]
            if msg_len > len(self._data) - 4:
                return
            body_data = self._data[4:4+msg_len]
            self._data = self._data[4+msg_len:]
            try:
                log.msg('==== RECV TCP:', repr(body_data))
                msg = json.loads(body_data)
                tasklet = self.makeTasklet(msg, self)
                stackless.tasklet(tasklet.run)()
            except Exception, e:
                import traceback
                traceback.print_exc()
                log.msg(body_data, logLevel=logging.ERROR)
                self.transport.loseConnection()
                return

        reactor.callLater(0, stackless.schedule)

    def makeTasklet(self, msg, connection):
        return TaskletSimple(msg, connection)


class TaskletSimple(object):
    def __init__(self, msg, connection):
        self.msg = msg
        self.connection = connection

    def wait_for_deferred(self, d, tip=None):
        try:
            d.addCallbacks(self.__callback, self.__errorback)
            return self._return_channel.receive()
        except Exception, e:
            import traceback
            traceback.print_exc()
            raise e

    def __callback(self, msg):
        try:
            self._return_channel.send(msg)
        except Exception, e:
            log.msg(str(e), logLevel=logging.ERROR)
            self._return_channel.send_exception(Exception, e)

        if stackless.getcurrent() != self._tasklet_instance:
            stackless.schedule()

    def __errorback(self, fault):
        try:
            self._return_channel.send_exception(fault.type, fault.value)
        except Exception, e:
            log.msg(fault, logLevel=logging.ERROR)
            self._return_channel.send_exception(Exception, e)

        if stackless.getcurrent() != self._tasklet_instance:
            stackless.schedule()

    def run(self):
        self._return_channel = stackless.channel()
        current = stackless.getcurrent()
        current._class_instance = self
        self._tasklet_instance = current
        try:
            self.handle()
        except Exception, e:
            import traceback
            traceback.print_exc()

    def handle(self):
        d = client.getPage('http://119.29.29.29/d?dn=www.ixxoo.me')
        result = self.wait_for_deferred(d)
        msg = {
            'cmd': self.msg['cmd'],
            'param': {
                'userId': self.msg['param']['userId'],
                'gameId': self.msg['param']['gameId'],
                'ip': result,
            }
        }
        self.connection.sendMessage(msg)


class __FileLogObserver__(log.FileLogObserver):
    timeFormat = '%m-%d %H:%M:%S.%f'

    def emit(self, eventDict):
        eventDict['system'] = '%s' % id(stackless.getcurrent())
        log.FileLogObserver.emit(self, eventDict)

if __name__ == '__main__':
    flo = __FileLogObserver__(sys.stdout)
    log.startLoggingWithObserver(flo.emit)
    from twisted.internet.protocol import ServerFactory
    fy = ServerFactory()
    fy.protocol = TcpProtocol
    reactor.listenTCP(9527, fy)
    stackless.tasklet(reactor.run)()
    stackless.run()
