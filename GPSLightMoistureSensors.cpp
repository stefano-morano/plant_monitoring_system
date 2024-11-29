#include "GPSLightMoistureSensors.h"

GPSLightMoistureSensors::GPSLightMoistureSensors(PinName GPSTxPin, PinName GPSRxPin, int GPSbaudRateGPS, 
                                                PinName ligthAnalogPin,
                                                PinName moistureAnalogPin) : gps(GPSTxPin, GPSRxPin,GPSbaudRateGPS),
                                                                            light(ligthAnalogPin),
                                                                            moisture(moistureAnalogPin)
{}

void GPSLightMoistureSensors::read_sensors_data() {
    while (true) {
        uint32_t flags = ThisThread::flags_wait_any(0x1, true);
        if (mode != 4)
            gps.read_gps();
        moisture.read_moisture_sensor();
        light.read_light_sensor();
    } 
}

void GPSLightMoistureSensors::change_mode(int new_mode) {
    mode = new_mode;
    moisture.mode = new_mode;
    light.mode = new_mode;
}

