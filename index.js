var nbind = require('nbind');
const path = require('path');
const math = require('mathjs');

var laserLib = nbind.init().lib;

function getXY(LaserPortName){

    return new Promise((resolve, reject)=>{
        var total_arr = laserLib.urg_calculate_xy(LaserPortName);
        if (total_arr){
            resolve(math.transpose([total_arr.slice(0, 1081), total_arr.slice(1081)]));
        } else {
            reject('Bad laser data');
        }
    })
}

function getAngleDist(LaserPortName){

    return new Promise((resolve, reject)=>{
        var total_arr = laserLib.urg_angle_dist(LaserPortName);
        if (total_arr){
            resolve({
                distance: total_arr.slice(0, 1081),
                angle: total_arr.slice(1081)
            });
        } else {
            reject('Bad laser data');
        }
    })
}

function angleDistToXY(obj){
    var XY = [];
    const {distance, angle} = obj;
    console.assert(distance.length === angle.length)
    for (let i = 0; i < distance.length; i++){
        if (distance[i] !== 0 && angle[i] !== 0){
            //console.log(distance[i])
            XY.push([distance[i] * Math.cos(angle[i]), distance[i] * Math.sin(angle[i])])
        }
    }

    return XY;
}
module.exports = {
    getXY,
    getAngleDist,
    angleDistToXY
}
