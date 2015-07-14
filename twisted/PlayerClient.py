#!/usr/bin/env python
# -*- coding=utf-8 -*-

# Author: likebeta <ixxoo.me@gmail.com>
# Create: 2015-07-14

import sys
import json
import struct
from python.Macro import *
from python.HttpClient import HttpClient
from framework.context import Context
from twisted.internet import reactor
from twisted.internet.protocol import Protocol
from twisted.internet.protocol import ClientFactory
from twisted.internet.protocol import connectionDone
from twisted.internet import defer


class PlayerProtocol(Protocol):

    def __init__(self):
        self._data = ''

    def dataReceived(self, data):
        self._data += data
        while len(self._data) > 12:
            msg_id, msg_len, msg_seq = struct.unpack('III', self._data[:12])
            if msg_len > len(self._data) - 12:
                return
            body_data = self._data[12:12+msg_len]
            self._data = self._data[12+msg_len:]
            msg = json.loads(body_data)
            Context.Log.info("==== recv msg ====, id=0x%x, seq=%d, len=%d body=%s" % (msg_id, msg_seq, msg_len, body_data))
            self.player.on_msg(msg_id, msg_seq, msg)

    def sendMsg(self, msg):
        if self.connected:
            data = json.dumps(msg)
            header_data = struct.pack('III', msg['cmd'], len(data), 0)
            try:
                self.transport.write(header_data + data)
                Context.Log.info("==== send msg ====, id=0x%x, seq=%d, len=%d body=%s" % (msg['cmd'], 0, len(data), data))
                return True
            except Exception, e:
                Context.Log.exception(msg['cmd'])
                return False
        else:
            Context.Log.info('not connect, cannot send msg 0x%x' % msg['cmd'])
            return False

    def stop(self):
        Context.Log.info('active close connect')
        self.transport.loseConnection()

    def connectionMade(self):
        self._data = ''
        self.player.run()

    def connectionLost(self, reason=connectionDone):
        self.factory.done(reason)


class PlayerFactory(ClientFactory):

    protocol = PlayerProtocol

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

    def __init__(self, http):
        self.http = http

    def run(self):
        if not self.req_user_info():
            self.protocol.stop()

        if not self.req_game_info():
            self.protocol.stop()

        if not self.req_room_list():
            self.protocol.stop()

    def on_msg(self, msg_id, msg_seq, msg):
        if 'error' in msg:
            self.protocol.stop()
            return
        if msg_id == MSGID_SYS_USER_INFO | ID_ACK:
            pass
        elif msg_id == MSGID_SYS_GAME_INFO | ID_ACK:
            pass
        elif msg_id == MSGID_SYS_ROOM_LIST | ID_ACK:
            self.req_quick_start()
        elif msg_id == MSGID_SYS_QUICK_START | ID_ACK:
            for k, v in msg["param"].iteritems():
                if k == "tableId" and hasattr(self, "tableId"):
                    continue
                if k == "seatId" and hasattr(self, "seatId"):
                    continue
                setattr(self, k, v)
            self.req_join_table()
        elif msg_id == MSGID_SYS_JOIN_TABLE | ID_ACK:
            self.req_sit_down()
        elif msg_id == MSGID_SYS_SIT_DOWN | ID_ACK:
            self.req_ready()
        elif msg_id == MSGID_SYS_READY | ID_ACK:
            pass
        elif msg_id == DDZ_MSGID_GAME_INIT | ID_NTF:
            pass
        elif msg_id == DDZ_MSGID_DEAL_CARD | ID_NTF:
            self.req_deal_end()
        elif msg_id == MSGID_SYS_TABLE_EVENT | ID_NTF:
            self.on_ntf_table_event(msg)
        elif msg_id == DDZ_MSGID_TOKEN | ID_NTF:
            self.on_ntf_token(msg)
        elif msg_id == MSGID_SYS_FORCE_QUIT | ID_ACK:
            self.protocol.stop()

    def send_to_svrd(self, msg):
        return self.protocol.sendMsg(msg)

    def req_hold(self):
        msg = {
            'cmd': MSGID_SYS_HOLD | ID_REQ,
            'param': {}
        }
        return self.send_to_svrd(msg)

    def req_user_info(self):
        msg = {
            'cmd': MSGID_SYS_USER_INFO | ID_REQ,
            'param': {
                'userId': int(self.http.userId),
                'gameId': 1,
                'sessionKey': self.http.sessionKey,
            }
        }
        return self.send_to_svrd(msg)

    def req_game_info(self):
        msg = {
            'cmd': MSGID_SYS_GAME_INFO | ID_REQ,
            'param': {
                'gameId': 1,
            }
        }
        return self.send_to_svrd(msg)

    def req_room_list(self):
        msg = {
            'cmd': MSGID_SYS_ROOM_LIST | ID_REQ,
            'param': {
                'gameId': 1,
            }
        }
        return self.send_to_svrd(msg)

    def req_quick_start(self):
        msg = {
            'cmd': MSGID_SYS_QUICK_START | ID_REQ,
            'param': {
                'userId': int(self.http.userId),
                'gameId': 1,
            }
        }
        return self.send_to_svrd(msg)

    def req_join_table(self):
        msg = {
            'cmd': MSGID_SYS_JOIN_TABLE | ID_REQ,
            'param': {
                'roomId': self.roomId,
                'tableId': self.tableId,
            }
        }
        return self.send_to_svrd(msg)

    def req_sit_down(self):
        msg = {
            'cmd': MSGID_SYS_SIT_DOWN | ID_REQ,
            'param': {
                'seatId': self.seatId,
            }
        }
        return self.send_to_svrd(msg)

    def req_ready(self):
        msg = {
            'cmd': MSGID_SYS_READY | ID_REQ,
            'param': {
            }
        }
        return self.send_to_svrd(msg)

    def req_deal_end(self):
        msg = {
            'cmd': DDZ_MSGID_DEAL_END | ID_REQ,
            'param': {
            }
        }
        return self.send_to_svrd(msg)

    def req_jdz(self):
        msg = {
            'cmd': DDZ_MSGID_JDZ | ID_REQ,
            'param': {
                'jdz': 0
            }
        }
        return self.send_to_svrd(msg)

    def req_qdz(self):
        msg = {
            'cmd': DDZ_MSGID_QDZ | ID_REQ,
            'param': {
                'qdz': 0
            }
        }
        return self.send_to_svrd(msg)

    def req_jb(self):
        msg = {
            'cmd': DDZ_MSGID_JB | ID_REQ,
            'param': {
                'jb': 0
            }
        }
        return self.send_to_svrd(msg)

    def req_trustee(self):
        msg = {
            'cmd': MSGID_SYS_TRUSTEE | ID_REQ,
            'param': {
                'trustee': 1
            }
        }
        return self.send_to_svrd(msg)

    def req_force_quit(self):
        msg = {
            'cmd': MSGID_SYS_FORCE_QUIT | ID_REQ,
            'param': {
            }
        }
        return self.send_to_svrd(msg)

    def req_leave_table(self):
        msg = {
            'cmd': MSGID_SYS_LEAVE_TABLE | ID_REQ,
            'param': {
            }
        }
        return self.send_to_svrd(msg)

    def on_ntf_table_event(self, msg):
        for event in msg['param']['event']:
            if event['type'] == enum_table_event.table_event_login:
                pass
            elif event['type'] == enum_table_event.table_event_join_table:
                pass
            elif event['type'] == enum_table_event.table_event_sit_down:
                pass
            elif event['type'] == enum_table_event.table_event_stand_up:
                pass
            elif event['type'] == enum_table_event.table_event_ready:
                pass
            elif event['type'] == enum_table_event.table_event_cancel_ready:
                pass
            elif event['type'] == enum_table_event.table_event_leave_table:
                pass
            elif event['type'] == enum_table_event.table_event_force_quit:
                pass
            elif event['type'] == enum_table_event.table_event_viewer_join_table:
                pass
            elif event['type'] == enum_table_event.table_event_viewer_leave_table:
                pass
            elif event['type'] == enum_table_event.table_event_kick_off:
                pass
            elif event['type'] == enum_table_event.table_event_offline:
                pass
            elif event['type'] == enum_table_event.table_event_reconnect:
                pass
            elif event['type'] == enum_table_event.table_event_game_start:
                Context.Log.info("game start")
            elif event['type'] == enum_table_event.table_event_game_end:
                Context.Log.info("game end")
            elif event['type'] == enum_table_event.table_event_game_info:
                pass
            elif event['type'] == enum_table_event.table_event_user_info:
                pass
            elif event['type'] == enum_table_event.table_event_table_info:
                pass
            elif event['type'] == enum_table_event.table_event_broadcast:
                pass
            elif event['type'] == enum_table_event.table_event_trustee:
                pass
            elif event['type'] == enum_table_event.table_event_cancel_trustee:
                pass

    def on_ntf_token(self, msg):
        tokenType = msg['param']['tokenType']
        tokenUser = msg['param']['tokenUser']
        if tokenType == enum_token_type.token_type_jdz:
            if int(self.http.userId) in tokenUser:
                self.req_jdz()
        elif tokenType == enum_token_type.token_type_qdz:
            if int(self.http.userId) in tokenUser:
                self.req_qdz()
        elif tokenType == enum_token_type.token_type_jb:
            if int(self.http.userId) in tokenUser:
                self.req_jb()
        elif tokenType == enum_token_type.token_type_play:
            # if int(self.http.userId) in tokenUser:
            #     self.req_trustee()
            # self.go_on_run = False
            self.req_force_quit()


def run():
    redis_key = sys.argv[1]
    Context.Log.show_location_prefix(False)
    Context.init_with_redis_key(redis_key)
    params = Context.RedisConfig.get_global_item_json('params')
    http_sdk = params['server']['http.sdk']
    httpClient = HttpClient(sys.argv[2], sys.argv[3], sys.argv[4], http_sdk)
    httpClient.run_as_player()
    tcpClient = PlayerClient(httpClient)
    if len(sys.argv) > 5:
        tcpClient.tableId = int(sys.argv[5])
    if len(sys.argv) > 6:
        tcpClient.seatId = int(sys.argv[6])
    d = defer.Deferred()
    factory = PlayerFactory(d, tcpClient)
    reactor.connectTCP(httpClient.host, int(httpClient.port), factory)
    d.addCallbacks(done, connect_failed)
    Context.Log.info('start reactor.............................')
    reactor.run()

def done(reason):
    Context.Log.debug('connect lost:', reason)
    reactor.stop()

def connect_failed(err):
    if str(err) != str(connectionDone):
        Context.Log.exception(err)
    else:
        Context.Log.info('connect close gracefully')
    reactor.stop()

if __name__ == '__main__':
    run()
