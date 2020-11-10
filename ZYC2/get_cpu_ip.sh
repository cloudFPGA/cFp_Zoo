#/bin/sh

ip -4 addr show tun0 | grep -oP '(?<=inet\s)\d+(\.\d+){3}'
