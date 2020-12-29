# Phomera

Is a simple tool to take (more advanced) pictures via phone on a raspberry pi. It supports additional settings such as aperture, light and focus that can be attached to other tools (such as a digital aperature) that can manage it (since raspivid/raspistill can't always manage it, e.g. lens adapters). 

To run it, you require gstreamer (on both rpi and pc). I tested it mainly with gstreamer-1.0 1.18.1 mingw-x86_64, but other versions could work. E.g. download https://gstreamer.freedesktop.org/data/pkg/android/1.18.1/ and unzip them into external/android_gstreamer. Next, in Android studio click File > Project Structure and route it to an Android NDK (21.0.6113669 is tested to work).