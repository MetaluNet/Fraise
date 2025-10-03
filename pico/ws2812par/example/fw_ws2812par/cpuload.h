// CPU load measuring utility
#pragma once

#include "fraise.h"
#include <string>

class CpuLoad {
private:
    absolute_time_t sensor_time;
    absolute_time_t reset_time;
    unsigned int count_us = 0;
    std::string name;
    unsigned int sensor_count = 0;
public:
    CpuLoad(std::string n): name(n) {}
    void start() {
        sensor_time = get_absolute_time();
    }
    void stop() {
        count_us += absolute_time_diff_us(sensor_time, get_absolute_time());
        sensor_count++;
    }
    float get_load() {
        float load = (100.0 * count_us) / absolute_time_diff_us(reset_time, get_absolute_time());
        fraise_printf("l load %s %f avgus %d\n", name.c_str(), load, count_us/sensor_count);
        return load;
    }
    void reset() {
        count_us = 0;
        sensor_count = 0;
        reset_time = get_absolute_time();
    }
};

