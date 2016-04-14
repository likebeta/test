#!/usr/bin/env python
# coding: utf-8

def callLaster(delay,callable,*args,**kw):
    print delay
    print callable
    print args
    print kw
    t_args = args
    t_kw = kw
    callable(*t_args,**t_kw)

def my_callable(msg,data):
    print msg
    print data


class Msg(object):
    pass

class Data(object):
    pass

callLaster(5,my_callable,Msg(),Data())
