#!/usr/bin/env python
# coding: utf-8

# desc: http://www.stackless.com/wiki/Channels

import stackless
from exceptions import StopIteration

def SendingSequence(channel, sequence):
    print "sending"
    channel.send_sequence(sequence)
    channel.send_exception(StopIteration)

def ReceivingSequence(channel):
    for item in channel:
        print "receiving"
        print item
    print "done receiving"

ch = stackless.channel()

task2 = stackless.tasklet(ReceivingSequence)(ch)
task = stackless.tasklet(SendingSequence)(ch, ['a','b','c'])

stackless.run()
