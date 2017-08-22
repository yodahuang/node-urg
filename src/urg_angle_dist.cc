#include "urg_calculate_xy.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace std;

vector<double> urg_angle_dist(const char* device)
{
    urg_t urg;
    long *data;
    long max_distance;
    long min_distance;
    long time_stamp;
    int i;
    int n;

    const double M_TO_MM = 1000;
    const int MAX_TRUSTABLE = 30;

    vector<double> arr_dist;
    vector<double> arr_angle;

    if (urg_open(&urg, URG_SERIAL, device, 115200) < 0) {
        printf("urg_open: %s, %d: %s\n",
            URG_SERIAL, 115200, urg_error(&urg));
        return arr_dist;
    }

    data = (long *)malloc(urg_max_data_size(&urg) * sizeof(data[0]));
    if (!data) {
        perror("urg_max_index()");
        return arr_dist;
    }

    // Gets measurement data
    urg_start_measurement(&urg, URG_DISTANCE, 1, 0);
    n = urg_get_distance(&urg, data, &time_stamp);
    if (n < 0) {
        printf("urg_get_distance: %s\n", urg_error(&urg));
        urg_close(&urg);
        return arr_dist;
    }

    // Outputs X-Y coordinates
    urg_distance_min_max(&urg, &min_distance, &max_distance);
    for (i = 0; i < n; ++i) {
        long distance = data[i];
        double radian;

        if ((distance < min_distance) || (distance > max_distance) || (distance/M_TO_MM > MAX_TRUSTABLE)) {
            continue;
        }

        radian = urg_index2rad(&urg, i);
        arr_dist.push_back(distance / M_TO_MM);
        arr_angle.push_back(radian);
    }

    // Disconnects
    free(data);
    urg_close(&urg);

#if defined(URG_MSC)
    getchar();
#endif
    // Combine the two vectors for better communictaion between C++ and Node.js
    vector<double> arr_total;
    arr_total.reserve( arr_dist.size()*2 );
    arr_total.insert(arr_total.end(), arr_dist.begin(), arr_dist.end());
    arr_total.insert(arr_total.end(), arr_angle.begin(), arr_angle.end());
    return arr_total;
}

#include "nbind/nbind.h"

NBIND_GLOBAL() {
  function(urg_angle_dist);
}
