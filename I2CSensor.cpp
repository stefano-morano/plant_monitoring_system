#include "I2CSensor.h"
#include "mbed.h"

// Accelerometer address
#define ACC_I2C_ADDRESS 0x1D << 1
#define ACC_CTRL_REG 0x2A
#define REG_OUT_X_MSB 0x01
#define REG_OUT_Y_MSB 0x03
#define REG_OUT_Z_MSB 0x05
#define UINT14_MAX 16383

// Color sensor definitions
#define TCS34725_ADDRESS 0x29 << 1  //register address for the rgb sensor
#define TCS34725_ENABLE 0x00        //register address to enable the rgb sensor
#define TCS34725_ENABLE_PON 0x01
#define TCS34725_ENABLE_AEN 0x02
#define TCS34725_ATIME 0x01
#define TCS34725_CONTROL 0x0F
#define TCS34725_COMMAND_BIT 0x80
#define TCS34725_CDATAL 0x14        //register for clear data
#define TCS34725_RDATAL 0x16        //register for red data
#define TCS34725_GDATAL 0x18        //register for green data
#define TCS34725_BDATAL 0x1A        //register for blue data
#define MAX_COLOR_VALUE 65535

// Temperature and Humidity sensor definitions
#define SI7021_ADDRESS 0x40 << 1
#define CMD_MEASURE_HUMIDITY 0xF5
#define CMD_MEASURE_TEMPERATURE 0xF3
#define THRESHOLD 1.0

I2CSensor::I2CSensor(PinName sda, PinName scl, PinName digitalPin) : i2c(sda, scl), light(digitalPin) {}

// --- Accelerometer Functions ---
bool I2CSensor::initializeAccelerometer() {
    uint8_t data[2] = {ACC_CTRL_REG, 0x01};
    return i2c.write(ACC_I2C_ADDRESS, (char *)data, 2) == 0;
}

bool I2CSensor::readAccRegs(int addr, uint8_t *data, int len) {
    char t[1] = {(char) addr};
    if (i2c.write(ACC_I2C_ADDRESS, t, 1, true) != 0)
        return false;
    return i2c.read(ACC_I2C_ADDRESS, (char *) data, len) == 0;
}

float I2CSensor::getAccAxis(uint8_t addr) {
    int16_t axisValue;
    uint8_t res[2];
    if (!readAccRegs(addr, res, 2)){
        accellerometer_alarm = true;
        accellerometer_working = false;
        return 0.0;
    } accellerometer_alarm = false;
    accellerometer_working = true;
    axisValue = (res[0] << 6) | (res[1] >> 2);
    if (axisValue > UINT14_MAX/2)
        axisValue -= UINT14_MAX;
    return float(axisValue) / 4096.0;
}

// --- Color Sensor Functions ---
void I2CSensor::initializeColorSensor() {
    if (writeColorRegister(TCS34725_ENABLE, TCS34725_ENABLE_PON) < 0){
        color_working = false;
        color_alarm = true;
        return;
    }
    ThisThread::sleep_for(3ms);
    if (writeColorRegister(TCS34725_ENABLE, TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN) < 0 || 
    writeColorRegister(TCS34725_ATIME, 0xFF) < 0 || writeColorRegister(TCS34725_CONTROL, 0x02) < 0){
        color_working = false;
        color_alarm = true;
        return;
    }
    color_working = true;
    color_alarm = false;
}

int I2CSensor::writeColorRegister(uint8_t reg, uint8_t value) {
    char data[2] = {(char)(TCS34725_COMMAND_BIT | reg), (char)value}; 
    return i2c.write(TCS34725_ADDRESS, data, 2);
}

int I2CSensor::readColorRegister(uint8_t reg) {
    char cmd = (TCS34725_COMMAND_BIT | reg);
    char data[2];
    if (i2c.write(TCS34725_ADDRESS, &cmd, 1) != 0)
        return -1;
    if (i2c.read(TCS34725_ADDRESS, data, 2) != 0)
        return -1;
    return (data[1] << 8) | data[0];
}

void I2CSensor::readColorData(uint16_t &clear, uint16_t &red, uint16_t &green, uint16_t &blue) {
    if (need_flash){
        light.write(1);
        ThisThread::sleep_for(20ms);
    }

    clear = readColorRegister(TCS34725_CDATAL);
    red = readColorRegister(TCS34725_RDATAL);
    green = readColorRegister(TCS34725_GDATAL);
    blue = readColorRegister(TCS34725_BDATAL);

   if (clear >= MAX_COLOR_VALUE && red >= MAX_COLOR_VALUE && 
        green >= MAX_COLOR_VALUE && blue >= MAX_COLOR_VALUE){
       color_working = false;
       color_alarm = true;
    } else {
        color_working = true;
        color_alarm = false;
    }
    
    if (need_flash){
        ThisThread::sleep_for(20ms);
        light.write(0);
    }
}

// --- Temperature and Humidity Sensor Functions ---
float I2CSensor::readHumidity() {
    char cmd[1] = { CMD_MEASURE_HUMIDITY };
    char data[2] = { 0 };
    if (i2c.write(SI7021_ADDRESS, cmd, 1) != 0){
        humidity_alarm = true;
        humidity_working = false;
        return 0.0;
    }
    ThisThread::sleep_for(20ms);
    if (i2c.read(SI7021_ADDRESS, data, 2) != 0){
        humidity_alarm = true;
        humidity_working = false;
        return 0.0;
    }
    humidity_alarm = false;
    humidity_working = true;
    int humidity_raw = (data[0] << 8) | data[1];
    return ((125.0 * humidity_raw) / 65536) - 6.0;
}

float I2CSensor::readTemperature() {
    char cmd[1] = { CMD_MEASURE_TEMPERATURE };
    char data[2] = { 0 };
    if (i2c.write(SI7021_ADDRESS, cmd, 1) != 0){
        temperature_alarm = true;
        temperature_working = false;
        return 0.0;
    }
    ThisThread::sleep_for(20ms);
    if (i2c.read(SI7021_ADDRESS, data, 2) != 0){
        temperature_alarm = true;
        temperature_working = false;
        return 0.0;
    }
    temperature_alarm = false;
    temperature_working = true;
    int temperature_raw = (data[0] << 8) | data[1];
    return ((175.72 * temperature_raw) / 65536) - 46.85;
}

int I2CSensor::dominant(int red, int green, int blue){
    int max = red, color = 1;
    if (green > max) {
        max = green;
        color = 2;
    }
    if (blue > max) {
        max = blue;
        color = 3;
    }
    if (mode == 2){
        counter_dominant[color-1]++;
    }
    return color;
}

void I2CSensor::reset_values(){
    min_temp = 55; 
    max_temp = 0; 
    mean_temp = 0;
    min_hum = 76;
    max_hum = 0;
    mean_hum = 0;
    count = 0;
    for (int x = 0; x<3 ; x++)
        counter_dominant[x] = 0;
    ax_min = 200;
    ax_max = -200;
    ay_min = 200;
    ay_max = -200;
    az_min = 200;
    az_max = -200;
}

float I2CSensor::calculateTotalAcceleration(float x, float y, float z) {
    return sqrt(x * x + y * y + z * z);
}

void I2CSensor::read_i2c() {
    
    while (true) {

        uint32_t flags = ThisThread::flags_wait_any(0x1, true);

        light.write(0);

        if (!initializeAccelerometer()){
            // alarm variables used for advanced mode
            accellerometer_alarm = true;
            accellerometer_working = false;
        } else {
            //Read accelerometer values
            accellerometer_alarm = false;
            accellerometer_working = true;
            ax = getAccAxis(REG_OUT_X_MSB);
            ay = getAccAxis(REG_OUT_Y_MSB);
            az = getAccAxis(REG_OUT_Z_MSB);
        }

        if (!accellerometer_working){
            ax = 0;
            ay = 0;
            az = 0;
            sprintf(accData, "Accellerometer data not avaiable");
        } else if (mode != 4)
            sprintf(accData, "ACCELLEROMETERS: X_axis = %.2f m/s², Y_axis = %.2f m/s², Z_axis = %.2f m/s²", ax * 9.8, ay * 9.8, az * 9.8);

        // Read color sensor values
        initializeColorSensor();

        if (color_working) readColorData(clear, red, green, blue);

        if (mode!=4){
            if (color_working){
                sprintf(RGBData, "COLOR SENSOR: Clear = %d, Red = %d, Green = %d, Blue = %d -- Dominant color: ", clear, red, green, blue);
                dominant_color = dominant(red, green, blue);
            } else sprintf(RGBData, "Error reading RGB sensor values");
        }
        
        
        // Read temperature and humidity
        humidity = readHumidity();
        temperature = readTemperature();

        if (!temperature_working)
            sprintf(tempData, "TEMPERATURE sensor not avaiable");  
        else if (mode != 4)
            sprintf(tempData, "TEMP/HUM: Temperature = %.1f°C", temperature);

        if (!humidity_working)
            sprintf(humData, "HUMIDITY sensor not avaiable");
        else if (mode!=4)
            sprintf(humData, "Relative Humidity = %.1f%%", humidity);

        if (mode == 2){
            if (humidity < min_hum)
                min_hum = humidity;
            if (humidity > max_hum)
                max_hum = humidity;
            if (temperature < min_temp)
                min_temp = temperature;
            if (temperature > max_temp)
                max_temp = temperature;
            if (ax < ax_min)
                ax_min = ax;
            if (ax > ax_max)
                ax_max = ax;
            if (ay < ay_min)
                ay_min = ay;
            if (ay > ay_max)
                ay_max = ay;
            if (az < az_min)
                az_min = az;
            if (az > az_max)
                az_max = az;
            count++;
            mean_temp = (mean_temp * (count - 1) + temperature) / count;
            mean_hum = (mean_hum * (count - 1) + humidity) / count;
        }

        if (mode == 4){

            if (temperature_working){
                if (temperature < 18){
                    sprintf(tempData, "TEMPERATURE is too low: %.1f%% < 18%%", temperature);
                    temperature_alarm = true;
                } else if (temperature > 25){
                    sprintf(tempData, "TEMPERATURE is too high: %.1f%% > 25%%", temperature);
                    temperature_alarm = true;
                } else {
                    sprintf(tempData, "TEMPERATURE is normal");
                    temperature_alarm = false;
                }
            }

            if (humidity_working){
                if (humidity < 25){
                    sprintf(humData, "AIR HUMIDITY is too low: %.1f%% < 25%%", humidity);
                    humidity_alarm = true;
                } else if (humidity > 75){
                    sprintf(humData, "AIR HUMIDITY is too high: %.1f%% > 75%%", humidity);
                    humidity_alarm = true;
                } else {
                    sprintf(humData, "AIR HUMIDITY is normal");
                    humidity_alarm = false;
                }
            }

            if (color_working){
                if (red < green && blue < 40){
                    sprintf(RGBData, "Leaf color is normal");
                    color_alarm = false;
                } else if (red < green && blue < 80){
                    sprintf(RGBData, "The leaf is becoming yellow: don't add more water and check the light");
                    color_alarm = true;
                } else if (red > green && blue < 30) {
                    sprintf(RGBData, "The leaf is becoming brown: check humidity, water and high temperature");
                    color_alarm = true;
                } else {
                    sprintf(RGBData, "Wrong position of the RGB sensor");
                    color_alarm = true;
                }
            } else sprintf(RGBData, "RGB sensor not avaiable");

            if (accellerometer_working){

                total_acc = calculateTotalAcceleration(ax, ay, az);

                if (total_acc > THRESHOLD) {
                    sprintf(accData, "The plant is moving too fast");
                    accellerometer_alarm = true;
                } else {
                    sprintf(accData, "The plant is stable");
                    accellerometer_alarm = false;
                }
            }
        }
    }
}