#include "mbed.h"
#include "LightSensor.h"

LightSensor::LightSensor(PinName analogPin) : sensor(analogPin){
    value = 0;
}

void LightSensor::reset_values(){
    min_light = 101;
    max_light = 0;
    mean_light = 0;
    count = 0;
}

void LightSensor::read_light_sensor(){
    value = sensor.read() * 100;       
    sprintf(light_sensor_data, "LIGHT: %.2f%%", value);

    if (mode == 2){
        if (value < min_light)
            min_light = value;
        if (value > max_light)
            max_light = value;
        count++;
        mean_light = (mean_light * (count - 1) + value) / count;
    }

    if (mode == 4){
        if (value < 0.2){
            sprintf(advanced_data, "LIGHT is too low: %.1f%% < 60%%", value);
            alarm = true;
        } else if (value > 1){
            sprintf(advanced_data, "LIGHT is too high: %.1f%% > 100%%", value);
            alarm = true;
        } else {
            sprintf(advanced_data, "LIGHT is normal");
            alarm = false;
        }
    }
}
