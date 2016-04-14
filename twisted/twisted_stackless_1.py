#!/usr/bin/env python
# coding: utf-8

from twisted.internet import reactor
import stackless
import sys
import twisted_timer as timer

count_callback = 0

def callback():
    global count_callback
    count_callback += 1
    print 'callback: %d' % count_callback

def tasklet():
    count_tasklet = 0
    while True:
        count_tasklet += 1
        print 'tasklet: %d' % count_tasklet
        stackless.schedule()
        if not reactor.running:
            print 'reactor stop, exit tasklet'
            break


if __name__  == '__main__':
    second = 3.0 if len(sys.argv) < 2.0 else float(sys.argv[1])
    t1 = timer.Timer()
    t1.setInterval(1.0,callback)
    t2 = timer.Timer()
    t2.setInterval(second,stackless.schedule)

    stackless.tasklet(reactor.run)()
    stackless.tasklet(tasklet)()
    stackless.run()

