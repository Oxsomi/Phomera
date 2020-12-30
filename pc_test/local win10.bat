@echo off
rem This line allows us to setup a UDP server on this system to listen to the other system
gst-launch-1.0 -e -v udpsrc port=5000 ! application/x-rtp, media=video, clock-rate=90000, encoding-name=H264 ! decodebin ! autovideosink sync=false