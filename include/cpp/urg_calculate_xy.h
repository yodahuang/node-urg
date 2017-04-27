#ifndef URG_CALCULATE_XY_H
#define URG_CALCULATE_XY_H

#include "urg_sensor.h"
#include "urg_utils.h"
#include <vector>

using namespace std;

extern vector<double> urg_calculate_xy(const char* device);

extern vector<double> urg_argle_dist(const char* device);

#endif
