var nbind = require('nbind');
var lib = nbind.init().lib;

var laser = lib.urg_calculate_xy('/dev/ttyS3');

console.log(laser)
