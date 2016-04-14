#!/usr/bin/env python
# coding: utf-8

# desc: http://www.stackless.com/wiki/ChannelExamples

import stackless
import pickle

def aCallable(name):
    print "  aCallable<%s>: Before schedule()" % name
    stackless.schedule()
    print "  aCallable<%s>: After schedule()" % name

tasks = []
for name in "ABCDE":
    tasks.append(stackless.tasklet(aCallable)(name))

print "Schedule 1:"
stackless.schedule()

print
print "Pickling..."
pickledTasks = pickle.dumps(tasks)

print
print "Schedule 2:"
stackless.schedule()

unpickledTasks = pickle.loads(pickledTasks)
for task in unpickledTasks:
    task.insert()
print
print "Schedule Unpickled Tasks:"
stackless.schedule()
