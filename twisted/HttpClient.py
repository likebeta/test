#!/usr/bin/env python
# -*- coding=utf-8 -*-

# Author: likebeta <ixxoo.me@gmail.com>
# Create: 2015-07-15

import json
import urllib
from twisted.web import client
from twisted.internet import defer
from twisted.internet import reactor
from framework.context import Context


def get_page(url, query=None, postdata=None, method='POST', headers=None, cookies=None, timeout=6):
    if not headers:
        headers = {'Content-type': 'application/x-www-form-urlencoded'}

    if isinstance(url, unicode):
        url = str(url)

    if query:
        url += '?' + urllib.urlencode(query)

    if postdata:
        postdata = urllib.urlencode(postdata)

    d = client.getPage(url, method=method, headers=headers, postdata=postdata, cookies=cookies, timeout=timeout)
    return d


class HttpClient(object):

    def __init__(self, devId, clientId, devName, baseUrl):
        self.devId = devId
        self.clientId = clientId
        self.devName = devName
        self.base_url = baseUrl
        self.has_login = False
        self.cookies = {}

    @defer.inlineCallbacks
    def login(self):
        params = {'deviceId': self.devId, 'devName': self.devName, 'clientId': self.clientId}
        url = self.base_url + '/v1/user/loginByGuest'
        msg = {'param': params, 'cmd': 0}
        Context.Log.info("login req: %s, %s" % (url, msg))
        try:
            res = yield get_page(url, postdata=msg, cookies=self.cookies)
            s = json.loads(res)
            Context.Log.info("login ack: %s, %s" % (url, s))
            if 'param' in s:
                for k, v in s['param'].iteritems():
                    setattr(self, k, v)
                self.has_login = True
        except Exception, e:
            print type(e), e

    def run_as_player(self):
        return self.login()

if __name__ == '__main__':
    import sys
    redis_key = sys.argv[1]
    Context.Log.show_location_prefix(False)
    Context.init_with_redis_key(redis_key)
    params = Context.RedisConfig.get_global_item_json('params')
    http_sdk = params['server']['http.sdk']
    player = HttpClient("devId", "clientId", "devName", http_sdk)

    @defer.inlineCallbacks
    def test():
        yield player.run_as_player()
        reactor.stop()

    reactor.callWhenRunning(test)
    reactor.run()
