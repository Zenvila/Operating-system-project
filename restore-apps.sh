#!/bin/bash
FILE="/var/tmp/kexec-session/running_apps.txt"
if [[ -f $FILE ]]; then
while read -r app; do
if command -v "$app" &>/dev/null; then
nohup "$app" &>/dev/null &
echo "Started $app"
fi
done < "$FILE"
else
echo "No app list found!"
fi
