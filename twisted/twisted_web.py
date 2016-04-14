#!/usr/bin/env python
# -*- coding=utf-8 -*-

#  Author: 易思龙 <ixxoo.me@gmail.com>
#  Create: 2014-07-24

import json
import traceback
from twisted.web import http

import get_user_info

gdata = {}

class MyRequestHandler(http.Request):
    pages = {
        '/api/v1/get_user_info': get_user_info.run,
    }
    def process(self):
        if self.pages.has_key(self.path):
            try:
                data = self.pages[self.path](self.path, self.args, gdata)
            except:
                traceback.print_exc()
                data = {'ret': 1, 'desc': 'system error'}
                data = json.dumps(data)
        else:
            self.setResponseCode(http.NOT_FOUND)
            data = {'ret': 2, 'desc': self.path + ' not implement'}
            data = json.dumps(data)
        self.write(data)
        self.finish()

class MyHttp(http.HTTPChannel):
    requestFactory=MyRequestHandler

class MyHttpFactory(http.HTTPFactory):
    protocol=MyHttp

if __name__=="__main__":
    import sys
    if len(sys.argv) < 3:
        print 'Usage %s listen_port (online|test)' % sys.argv[0]
        sys.exit(-1)
    if sys.argv[2] not in ['online', 'test']:
        print 'Usage %s listen_port (online|test)' % sys.argv[0]
        sys.exit(-2)

    gdata['mode'] = sys.argv[2]
    from twisted.internet import reactor
    reactor.listenTCP(int(sys.argv[1]),MyHttpFactory())
    reactor.run()

