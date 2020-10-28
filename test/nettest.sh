#!/usr/bin/expect -f

set ip [lindex $argv 0]
set port [lindex $argv 1]

set timeout 5
spawn telnet $ip $port
send "<s>"