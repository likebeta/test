#!/usr/bin/env python
# -*- coding=utf-8 -*-

# Author: likebeta <ixxoo.me@gmail.com>
# Create: 2015-07-16

import sys
from framework.context import Context
from HttpClient import HttpClient
from twisted.internet import defer
from twisted.internet import reactor

@defer.inlineCallbacks
def create(clients, step):
    for i in xrange(0, len(clients), step):
        print '---------------------------%d -> %d--------------------------' % (i, i+step)
        ds = []
        for c in clients[i:i+step]:
            ds.append(c.run_as_player())
        yield defer.DeferredList(ds, consumeErrors=True)
    reactor.stop()

if __name__ == '__main__':
    redis_key = sys.argv[1]
    Context.Log.show_location_prefix(False)
    Context.init_with_redis_key(redis_key)
    params = Context.RedisConfig.get_global_item_json('params')
    http_sdk = params['server']['http.sdk']

    fname = sys.argv[2]
    with open(fname) as f:
        robot_name = f.readlines()

    clients = []
    for k, name in enumerate(robot_name, 1):
        name = name[:-1]
        if name:
            name = name.decode('utf8')
            httpClient = HttpClient('robot_' + str(k), 'client_robot_1.0', name, http_sdk)
            clients.append(httpClient)

    reactor.callWhenRunning(create, clients, 50)
    reactor.run()
