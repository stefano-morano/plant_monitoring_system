#ifndef I2CSENSORS_H
#define I2CSENSORS_H

#include "mbed.h"
#include <cstdint>

class I2CSensor {
public:

    DigitalOut light;
    uint16_t clear, red, green, blue;
    float ax, ay, az, humidity, temperature;
    float ax_min = 200, ax_max = -200, ay_min = 200, ay_max = -200, az_min = 200, az_max = -200;
    int dominant_color, mode, count = 0;
    float min_temp = 55, max_temp = 0, mean_temp = 0;
    float min_hum = 76, max_hum = 0, mean_hum = 0;
    char accData[128], RGBData[128], tempData[128], humData[128];
    int counter_dominant[3];
    bool color_alarm = false, temperature_alarm = false, humidity_alarm = false, accellerometer_alarm = false, 
    color_working = false, temperature_working = false, humidity_working = false, accellerometer_working = false, 
    need_flash = false;
    
    I2CSensor(PinName sda, PinName scl, PinName digitalPin);    
    void reset_values();
    void read_i2c();
    void changeLED(int mode);

private:

    I2C i2c;
    float total_acc = 0;

    bool readAccRegs(int addr, uint8_t *data, int len);
    float getAccAxis(uint8_t addr);
    int writeColorRegister(uint8_t reg, uint8_t value);
    
    //RGB sensor functions
    int readColorRegister(uint8_t reg);
    int dominant(int red, int green, int blue);
    float readHumidity();
    float readTemperature();

    // Color sensor functions
    void initializeColorSensor();
    void readColorData(uint16_t &clear, uint16_t &red, uint16_t &green, uint16_t &blue);

     // Accelerometer functions
    bool initializeAccelerometer();
    float calculateTotalAcceleration(float x, float y, float z);

};

#endif
