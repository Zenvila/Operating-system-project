#!/bin/bash
mkdir -p /var/tmp/kexec-session
ps -eo comm | sort | uniq > /var/tmp/kexec-session/running_apps.txt
