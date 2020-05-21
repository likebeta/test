#!/usr/bin/env python
# -*- coding=utf-8 -*-

# Author: likebeta <ixxoo.me@gmail.com>
# Create: 2015-10-11

import sys
import json
import struct
import logging
from twisted.python import log
from twisted.internet import defer
from twisted.internet import reactor
from twisted.internet.protocol import Protocol
from twisted.internet.protocol import ClientFactory
from twisted.internet.protocol import connectionDone


class SimpleProtocol(Protocol):
    def __init__(self):
        self._data = ''

    def dataReceived(self, data):
        self._data += data
        while len(self._data) > 4:
            msg_len = struct.unpack('I', self._data[:4])[0]
            if msg_len > len(self._data) - 4:
                return
            body_data = self._data[4:4 + msg_len]
            self._data = self._data[4 + msg_len:]
            msg = json.loads(body_data)
            log.msg("====recv====, len=%d body=%s" % (msg_len, body_data))
            self.player.on_msg(msg)

    def sendMsg(self, msg):
        if self.connected:
            data = json.dumps(msg)
            header_data = struct.pack('I', len(data))
            try:
                self.transport.write(header_data + data)
                log.msg("====send====, len=%d body=%s" % (len(data), data))
                return True
            except Exception, e:
                log.msg(msg['cmd'], logLevel=logging.ERROR)
                return False
        else:
            log.msg('not connect, cannot send msg %s' % msg['cmd'])
            return False

    def stop(self):
        log.msg('active close connect')
        self.transport.loseConnection()

    def connectionMade(self):
        self._data = ''
        self.player.run()

    def connectionLost(self, reason=connectionDone):
        self.factory.done(reason)
        self.connected = 0


class SimpleFactory(ClientFactory):
    protocol = SimpleProtocol

    def __init__(self, deferred, player):
        self.deferred = deferred
        self.player = player

    def done(self, reason):
        if self.deferred:
            d, self.deferred = self.deferred, None
            d.callback(reason)

    def clientConnectionFailed(self, connector, reason):
        if self.deferred:
            d, self.deferred = self.deferred, None
            d.errback(reason)

    def buildProtocol(self, addr):
        p = ClientFactory.buildProtocol(self, addr)
        p.player = self.player
        self.player.protocol = p
        return p

class PlayerClient(object):
    gid = 1

    def __init__(self, userId):
        self.userId = userId

    def close(self):
        self.protocol.stop()

    def run(self):
        if not self.req_hold():
            self.close()

    def on_msg(self, msg):
        if 'error' in msg:
            self.close()
            return

    def send_to_svrd(self, msg):
        return self.protocol.sendMsg(msg)

    def req_hold(self):
        if not self.protocol.connected:
            return

        msg = {
            'cmd': 'hold',
            'param': {
                'userId': self.userId,
                'gameId': self.gid,
            }
        }
        reactor.callLater(1, self.req_hold)
        return self.send_to_svrd(msg)

def done(reason):
    log.msg('connect lost:', reason)
    reactor.stop()


def connect_failed(err):
    if str(err) != str(connectionDone):
        log.msg(err, logLevel=logging.ERROR)
    else:
        log.msg('connect close gracefully')
    reactor.stop()


class __FileLogObserver__(log.FileLogObserver):
    timeFormat = '%m-%d %H:%M:%S.%f'

    def emit(self, eventDict):
        eventDict['system'] = '-'
        log.FileLogObserver.emit(self, eventDict)

if __name__ == '__main__':
    flo = __FileLogObserver__(sys.stdout)
    log.startLoggingWithObserver(flo.emit)
    for i in range(10000, 10101):
        p = PlayerClient(i)
        d = defer.Deferred()
        fy = SimpleFactory(d, p)
        reactor.connectTCP('127.0.0.1', 9527, fy)
    reactor.run()
