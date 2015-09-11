#!/usr/bin/env bash
# -*- coding=utf-8 -*-

#  Author: 易思龙 <ixxoo.me@gmail.com>
#  Create: 2015-09-10

g++ main.cpp urlencode.cpp httpClient.cpp mongoose.c -W -Wall -DNS_ENABLE_SSL -lssl -lcrypto -pthread --std=c++11
