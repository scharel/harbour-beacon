# harbour-beacon
A Philips Hue app for SailfishOS

## Current state

**The app is in active development and not ready for an everyday use in the current state!**

## Overview

This app includes its own QT/C++ implementation of the [Hue API V2](https://developers.meethue.com/develop/hue-api-v2/).

### 3rd party software

 - https://github.com/mjansson/mdns.git
 - https://github.com/llewelld/shine/tree/sailfish/plugin/Hue
 
### Getting started

Clone this git repository:

```
git clone --recurse-submodules https://github.com/scharel/harbour-beacon.git
```

Open the file `harbour-beacon.pro` with the [Sailfish SDK](https://docs.sailfishos.org/Tools/Sailfish_SDK/).

Now you can build the code and install it to your device or in the simulator.
