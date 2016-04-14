#!/usr/bin/env python
# coding: utf-8

def counter(start_at=0):
    count = [start_at]
    def incr():
        count[0] += 1
        return count[0]
    return incr

s = counter(1)
print s()
print s()
print s()

t = counter(9.0)
print t()
print t()
print t()

print s()
