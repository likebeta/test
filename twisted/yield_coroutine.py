#!/usr/bin/env python
# coding: utf-8
import time

def coroutine(func):
    def ret():
        f = func()
        f.next()
        return f
    return ret

@coroutine
def consumer():
    print "Wait to getting a task"
    while 1:
        n = (yield)
        print "Got %s" % n
                                                               
def producer():
    c = consumer()
    while 1:
        time.sleep(1)
        print "Send a task to consumer"
        c.send("task")

if __name__ == "__main__":
    producer()
