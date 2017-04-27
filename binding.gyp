{
    'conditions': [
        ['OS=="linux"', {
            'cflags': [
                '-O2'
            ],
            "targets": [
                {
                    "includes": [
                        "auto.gypi"
                    ],
                    "sources": [
                        "src/urg_angle_dist.cc", "src/urg_calculate_xy.cc", "src/urg_connection.c", "src/urg_debug.c", "src/Urg_driver.cpp", "src/urg_ring_buffer.c", "src/urg_sensor.c", "src/urg_serial.c", "src/urg_tcpclient.c", "src/urg_utils.c", "src/urg_serial_utils_linux.c"
                    ],
                    "include_dirs": [
                        "./include/cpp",
                        "./include/c"
                    ]
                }
            ]

        }]
    ],
    "includes": [
        "auto-top.gypi"
    ]
}
