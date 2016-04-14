#!/usr/bin/env python
# coding: utf-8

from twisted.internet import reactor

class Timer(object):
    TIMER_ONCE = 1
    TIMER_LOOP = 2

    def __init__(self):
        self._type = None

    def setTimeout(self, seconds, callback, *args, **kw):
        self._type = Timer.TIMER_ONCE
        self._seconds = seconds
        self._callback = callback
        self._timer = reactor.callLater(seconds, self.__callback, *args, **kw)
        return True

    def setInterval(self, seconds, callback, *args, **kw):
        self._type = Timer.TIMER_LOOP
        self._seconds = seconds
        self._callback = callback
        self._timer = reactor.callLater(seconds, self.__callback, *args, **kw)
        return True

    def getTimerType(self):
        return self._type

    def getLeftTime(self):
        if self._type is None:
            return None
        else:
            return self._timer.getTime()

    def getInterval(self):
        if self._type is None:
            return None
        return self._seconds

    def delay(self,seconds):
        try:
            return self._timer.delay(seconds)
        except:
            return None

    def reset(self,seconds):
        try:
            self._timer.reset(seconds)
            self._seconds = seconds
            return True
        except:
            return False

    def cancel(self):
        try:
            self._timer.cancel()
            self._type = None
            return True
        except:
            return False

    def IsActive(self):
        try:
            return self._timer.active()
        except:
            return False

    def __callback(self, *args, **kw):
        self._callback(*args, **kw)
        if self._type == Timer.TIMER_LOOP:
            self._timer = reactor.callLater(self._seconds, self.__callback, *args, **kw)
        else:
            self._type = None

if __name__ == '__main__':
    def once_test(name, age):
        print 'in once_test:', name, age

    def loop_test(name, age):
        print 'in loop_test:', name, age

    def stop_run():
        print 'stop timer loop....'
        t1.cancel()
        reactor.stop()

    t1 = Timer()
    t2 = Timer()

    t1.setInterval(0.2, loop_test, 'yisilong', 24)
    t2.setTimeout(0.5, once_test, 'liyan', 23)
    reactor.callLater(6, stop_run)
    reactor.run()
