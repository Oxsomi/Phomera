#!/bin/sh
raspivid -n -rot 270 -t 0 -w 1440 -h 1080 -fps 30 -b 6000000 -o - | gst-launch-1.0 -e -vvvv fdsrc ! h264parse ! rtph264pay pt=96 config-interval=5 ! udpsink host=$1 port=5000