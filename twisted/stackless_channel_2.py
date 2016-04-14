#!/usr/bin/env python
# coding: utf-8

# desc: http://www.stackless.com/wiki/ChannelExamples

import stackless

class NonblockingChannel(stackless.channel):
    def send(self, value, wait=False):
        if wait or (self.balance < 0):
            # There are tasklets waiting to receive
            return stackless.channel.send(self, value)
        else:
            print 'no one recving, ignore'

    def send_exception(self, exc, value, wait=False):
        if wait or (self.balance < 0):
            # There are tasklets waiting to receive
            return stackless.channel.send_exception(self, exc, value)

    def receive(self, default=None, wait=False):
        if wait or (self.balance > 0):
            # There are tasklets waiting to send
            return stackless.channel.receive(self)
        else:
            print 'no data, return defalut value'
            return default

def testNonblockingChannel():
    print
    print "testNonblockingChannel"
    print "----------------------"

    def recv(ch, name):
        print "Started recv<%s>" % (name,)
        print "recv<%s>: got a message from '%s'" % (name, ch.receive(wait=True))
        ch.send(name)

    ch = NonblockingChannel()
    ch.receive() # nonblocking receive
    ch.send("nonblocking!") # nonblocking send
    print

    for name in "ABCDE":
        task = stackless.tasklet(recv)(ch, name)
        task.run()

    ch.send('host')
    print

testNonblockingChannel()
