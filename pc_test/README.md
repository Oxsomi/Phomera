# Win10 -> PI test

The pi has the HQ camera module attached and streams to a Win10 machine. If you download obs + the obs extension called "obs-gstreamer", you can use it as a virtual webcam.
It assumes you are using a pi HQ module, it runs at 4:3 (1440x1080@30), since that's the physical sensor size, cropping can be done too (easiest is through OBS).
Stills can be done at 12.3 MP (4056x3040), however if you use a rpi0w, it might be difficult to reach this at any interactive framerates (it can take around 10 per second).