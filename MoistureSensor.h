#ifndef MoistureSensor_H
#define MoistureSensor_H

#include "mbed.h"

class MoistureSensor {
public:
    float value;
    char moisture_sensor_data[256], advanced_data[56];
    float min_soil = 101, max_soil = 0, mean_soil = 0;
    int count = 0, mode;
    bool alarm = false;
    AnalogIn sensor;
   
    MoistureSensor(PinName analogPin);
    void read_moisture_sensor();
    void reset_values();
};

#endif