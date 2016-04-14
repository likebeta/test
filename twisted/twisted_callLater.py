#!/usr/bin/env python
# coding: utf-8
from twisted.internet import reactor
import time

def print_log():
    print 'print log'
    reactor.stop()

def loop():
    print 'begin loop'
    reactor.callLater(0.0,print_log)
    i = 10
    while i > 0:
        print 'in loop'
        time.sleep(0.5)
        i -= 1

reactor.callLater(0.5,loop)
reactor.run()

    
