#!/usr/bin/env python
# -*- coding=utf-8 -*-

#  Author: 易思龙 <ixxoo.me@gmail.com>
#  Create: 2014-10-21

print __file__
import bar
print locals()
if hasattr(bar, 'bar_init'):
    bar.bar_init()

if hasattr(bar, 'main'):
    bar.main.bar_main()
