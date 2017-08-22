## Urg library for node.js

This is a ported JS library of the [original URG library](https://sourceforge.net/p/urgnetwork/wiki/Home/) written in C/C++, aiming to read data from Hokuyo URG-Series sensors, which are accurate distance measurement sensors.This port is implemented using [nbind](https://www.npmjs.com/package/nbind).

Currently, the library only implement the basic functions of reading lidar data and is **only work on Linux and Mac**.

I only test this on UTM-30LX. More testing appreciated.

### Future work

- Write build files files for Windows so the library can be used on Windows.

- Implement more advanced functions.

### API

- `getXY(laserPortName)`
    - `laserPortName` is a string of the port e.g. '/dev/ttyS2'
    - return a promise, when success, will contain a 2d array in the following form : `[[x0, y0], [x1, y1]...]`

- `urg_angle_dist(laserPortName)`
    - return a promise, when success, will contain an object of `{distance: [...], angle: [...]}`
