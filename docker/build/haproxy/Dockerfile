#
# liblifthttp-automation-haproxy
#
# Based on:
# https://hub.docker.com/_/haproxy/
#
# Documentation:
# https://github.com/jbaldwin/liblifthttp

# Pull base image.
FROM haproxy:latest
COPY haproxy.cfg /usr/local/etc/haproxy/haproxy.cfg
COPY entrypoint.sh /entrypoint.sh

# Define default command.
ENTRYPOINT ["/entrypoint.sh", "/bin/bash"]
