#!/usr/bin/env python
# coding: utf-8
import stackless
import time 
import signal

def recv_tasklet():
    while True:
        print "recv enter %d" % stackless.getruncount()
#        print "recv data: " + channel.receive()
        try:
            print "recv data: " + channel.receive()
        except Exception,e:
            print "recv get cexeption:" + e.message
        print "recv leave %d" % stackless.getruncount()

def send_tasklet():
    while True:
        print "send enter %d" % stackless.getruncount()
#        channel.send("cao")
        channel.send_exception(Exception,'dddd')
        print "send leave %d" % stackless.getruncount()

def common_tasklet():
    while True:
        print "common %d" % stackless.getruncount()
        time.sleep(0.5)
        stackless.schedule()

def channel_cb(channel,tasklet,sending,willblock):
    print 'I am callback'

channel = stackless.channel()
stackless.tasklet(recv_tasklet)()
stackless.tasklet(send_tasklet)()
stackless.tasklet(common_tasklet)()

stackless.set_channel_callback(channel_cb)

stackless.run()
