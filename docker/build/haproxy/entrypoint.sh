#!/bin/bash

# ignore hostname resolution failures for 'nginx' or '127.0.0.1' testing.
/usr/local/sbin/haproxy -f /usr/local/etc/haproxy/haproxy.cfg -dr

# Keep this container running
tail -f /dev/null
