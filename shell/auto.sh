#!/usr/bin/expect

spawn ssh username@host
expect "password: "
send "passwd\r"
interact
