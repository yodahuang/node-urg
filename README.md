## Urg library for node.js

This is a ported JS library of the [original URG library](https://sourceforge.net/p/urgnetwork/wiki/Home/) written in C/C++, aiming to read data from Hokuyo URG-Series sensors, which are accurate distance measurement sensors.This port is implemented using [nbind](https://www.npmjs.com/package/nbind).

Currently, the library only implement the basic functions of reading lidar data.

### Note

The library only works on linux and Mac since the building file for windows is not written.

Currently the library only supports UTM-30LX, since I hardcoded number `1081` in the code (which I should not). Will fix this when have time. (You are welcome to contribute too!)

After I fix these things I'll publish this on `npm`

### API

- `getXY(laserPortName)`
    - `laserPortName` is a string of the port e.g. '/dev/ttyS2'
    - return a 2d array in the following form : [[x0, y0], [x1, y1]...]

- `urg_angle_dist(laserPortName)`
    - return an object of {distance: [...], angle: [...]}
