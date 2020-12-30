#!/bin/sh
raspivid -n -rot 270 -t 0 -w 1080 -h 810 -fps 24 -o - | gst-launch-1.0 -e fdsrc ! h264parse ! rtph264pay pt=96 ! udpsink host=$1 port=5000 sync=false