#!/usr/bin/env python
# coding: utf-8

from twisted.internet import defer,reactor
import time

def calc_square(x):
    d = defer.Deferred()
    reactor.callLater(1,d.callback,x * x)
    return d

def print_result(x):
    print x
    time.sleep(3)
    reactor.stop()

if __name__ == '__main__':
    d = calc_square(9)
    d.addCallback(print_result)
    reactor.run()


