#ifndef GPSLightMoistureSensors_H
#define GPSLightMoistureSensors_H

#include "GPS.h"
#include "LightSensor.h"
#include "MoistureSensor.h"

class GPSLightMoistureSensors {
public:
    GPS gps;
    LightSensor light;
    MoistureSensor moisture;

    int mode;

    GPSLightMoistureSensors(PinName GPSTxPin, PinName GPSRxPin, int GPSbaudRateGPS, 
                            PinName ligthAnalogPin,
                            PinName moistureAnalogPin);

    void read_sensors_data();
    void change_mode(int new_mode);
};

#endif