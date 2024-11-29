#include "GPSLightMoistureSensors.h"
#include "I2CSensor.h"
#include "mbed.h"
#include <cstdio>

#define TEST 1      //0,0,1
#define NORMAL 2    //0,1,0
#define ADVANCED 4  //1,0,0
#define WHITE 0     //0,0,0
#define YELLOW 1    //0,0,1
#define PURPLE 2    //0,1,0
#define RED 3       //0,1,1
#define CYAN 4      //1,0,0
#define GREEN 5     //1,0,1
#define BLUE 6      //1,1,0
#define OFF 7       //1,1,1

#define GPS_TX_PIN PA_9
#define GPS_RX_PIN PA_10
#define LIGHT_PIN PA_0
#define MOISTURE_PIN PA_4
#define I2C_SDA PB_9
#define I2C_SCL PB_8
#define LIGHT_RGB_PIN PB_12

#define TEST_DELAY 2s
#define NORMAL_DELAY 10s
#define OUTPUT_DELAY 30s
#define ADVANCED_DELAY 3s
#define COLOR_DELAY 500ms

InterruptIn button(PB_2);
BusOut led_board(LED1, LED2, LED3);
BusOut led_rgb(PB_13, PB_14, PB_15);
BufferedSerial pc(USBTX, USBRX, 115200);
Ticker ticker, summarize_ticker, color_ticker;

GPSLightMoistureSensors gpsLightMoisture(GPS_TX_PIN, GPS_RX_PIN, 9600, LIGHT_PIN, MOISTURE_PIN);
I2CSensor I2CSensor(I2C_SDA, I2C_SCL, LIGHT_RGB_PIN);
Thread i2cThread(osPriorityNormal, 1024), gpsLightMoistureThread(osPriorityNormal, 1024);

bool button_pressed = false, print_test = false, print_summarize = false, advanced_mode = false, color_seq = false;
int mode, max_dominant, alarm[6], size = 0, counter;

void onClick(void) { button_pressed = true; }
void print_func(void) { print_test = true; }
void print_summ(void) { print_summarize = true; }
void advanced_func(void) { advanced_mode = true; }
void color_sequence(void) { color_seq = true; }

void reset_sensors(){
    gpsLightMoisture.moisture.reset_values();
    gpsLightMoisture.light.reset_values();
    I2CSensor.reset_values();
}

void change_mode(int mode){
    gpsLightMoisture.change_mode(mode);
    I2CSensor.mode = mode;
    ticker.detach();
    summarize_ticker.detach();
    color_ticker.detach();
    counter = 0;
    led_board = mode;
    led_rgb = OFF;
    i2cThread.flags_set(0x1);
    gpsLightMoistureThread.flags_set(0x1);
}

int calculate_max_dominant(){
    int index = 0, max = I2CSensor.counter_dominant[0];
    if (I2CSensor.counter_dominant[1] > max){
        max = I2CSensor.counter_dominant[1];
        index = 1;
    }
    if (I2CSensor.counter_dominant[2] > max){
        max = I2CSensor.counter_dominant[2];
        index = 2;
    }
    return index;
}

void add_queue(int color){
    for (int x = 0; x<size; x++)
        if (alarm[x] == color)
            return;
    alarm[size] = color;
    size++;
}

void remove_queue(int color){
    for (int x = 0; x<size; x++)
        if (alarm[x] == color){
            for (int i = x; i < size - 1; i++) {
                alarm[i] = alarm[i + 1];
            } 
            size--;
            return;      
        }
}

int main(){
    button.mode(PullUp);
    button.fall(onClick);

    mode = TEST;
    led_board = TEST;
    led_rgb = OFF;
    counter = 0;

    gpsLightMoistureThread.start(callback(&gpsLightMoisture, &GPSLightMoistureSensors::read_sensors_data));
    gpsLightMoistureThread.flags_set(0x1);
    i2cThread.start(callback(&I2CSensor, &I2CSensor::read_i2c));
    i2cThread.flags_set(0x1);

    ticker.attach(print_func, TEST_DELAY);

    printf("+-------- TEST MODE --------+\n\n");
    while (true) {
        if (button_pressed) {               
            button_pressed = false;
            mode++;
            if (mode == 4) mode = 1;
            switch(mode){
                case 1: 
                    change_mode(TEST);
                    printf("+-------- TEST MODE --------+\n\n");
                    ticker.attach(print_func, TEST_DELAY);
                    break;
                case 2: 
                    change_mode(NORMAL);
                    printf("+-------- NORMAL MODE --------+\n\n");
                    reset_sensors();
                    ticker.attach(print_func, NORMAL_DELAY);
                    summarize_ticker.attach(print_summ, OUTPUT_DELAY);
                    break;
                case 3: 
                    change_mode(ADVANCED);
                    printf("+-------- ADVANCED MODE --------+\n\n");
                    ticker.attach(advanced_func, ADVANCED_DELAY);
                    break;
            }
        }

        if (print_test){
            print_test = false;
            printf("%s\n", gpsLightMoisture.moisture.moisture_sensor_data);
            printf("%s\n", gpsLightMoisture.light.light_sensor_data);
            printf("%s\n", gpsLightMoisture.gps.gps_data);
            printf("%s", I2CSensor.RGBData);
            if (I2CSensor.color_working){
                switch (I2CSensor.dominant_color){
                    case 1: 
                        printf("RED\n");
                        (mode == TEST) ? led_rgb = RED : led_rgb = OFF;
                        break;
                    case 2: 
                        printf("GREEN\n");
                        (mode == TEST) ? led_rgb = GREEN : led_rgb = OFF;
                        break;
                    case 3: 
                        printf("BLUE\n");
                        (mode == TEST) ? led_rgb = BLUE : led_rgb = OFF;
                        break;
                }
            } else {
                printf("\n");
                led_rgb = OFF;
            }
            
            printf("%s\n", I2CSensor.accData);
            printf("%s, \t", I2CSensor.tempData);
            printf("%s\n", I2CSensor.humData);
            printf("\n-------------------------------------------------------------------------------------------------\n\n");

            I2CSensor.need_flash = (gpsLightMoisture.light.value < 0.2);

            if (mode == NORMAL){                        
                if (!I2CSensor.temperature_working || I2CSensor.temperature < -10 || I2CSensor.temperature > 50)
                    led_rgb = RED;
                if (!I2CSensor.humidity_working || I2CSensor.humidity < 25 || I2CSensor.humidity > 75)
                    led_rgb = WHITE;
                if (gpsLightMoisture.light.value < 0 || gpsLightMoisture.light.value > 100)
                    led_rgb = CYAN;
                if (gpsLightMoisture.moisture.value < 0 || gpsLightMoisture.moisture.value > 100)
                    led_rgb = YELLOW;
                if (!I2CSensor.color_working)           //it's the only one that can't have wrong parameters
                    led_rgb = PURPLE;
                if (I2CSensor.accellerometer_alarm || !I2CSensor.accellerometer_working)
                    led_rgb = BLUE;
            }

            /* if there is a change in modes waking threads through flags 
                as sleep_for method would still keep them sleeping when mode changes */
            i2cThread.flags_set(0x1);
            gpsLightMoistureThread.flags_set(0x1);
        }

        if (print_summarize){
            print_summarize = false;
            max_dominant = calculate_max_dominant();

            printf("\n\n+-------- VALUES REPORT --------+\n\n");
            printf("LIGHT SENSOR: min = %.1f%%, max = %.1f%%, mean = %.1f%%\n", gpsLightMoisture.light.min_light, gpsLightMoisture.light.max_light, gpsLightMoisture.light.mean_light);
            printf("SOIL AND MOISTURE SENSOR: min = %.1f%%, max = %.1f%%, mean = %.1f%%\n", gpsLightMoisture.moisture.min_soil, gpsLightMoisture.moisture.max_soil, gpsLightMoisture.moisture.mean_soil);
            printf("TEMPERATURE SENSOR: min = %.1f°C, max = %.1f°C, mean = %.1f°C\n", I2CSensor.min_temp, I2CSensor.max_temp, I2CSensor.mean_temp);
            printf("HUMIDITY SENSOR: min = %.1f%%, max = %.1f%%, mean = %.1f%%\n", I2CSensor.min_hum, I2CSensor.max_hum, I2CSensor.mean_hum);
            printf("MOST READ DOMINANT COLOR: ");
            switch (max_dominant){
                case 0: 
                    printf("RED\n");
                    break;
                case 1: 
                    printf("GREEN\n");
                    break;
                case 2: 
                    printf("BLUE\n");
                    break;    
            }
            printf("ACCELLEROMETER X-AXIS: max = %.2f m/s², min = %.2f m/s²\n", I2CSensor.ax_max, I2CSensor.ax_min);
            printf("ACCELLEROMETER Y-AXIS: max = %.2f m/s², min = %.2f m/s²\n", I2CSensor.ay_max, I2CSensor.ay_min);
            printf("ACCELLEROMETER Z-AXIS: max = %.2f m/s², min = %.2f m/s²\n", I2CSensor.az_max, I2CSensor.az_min);
            printf("\n-------------------------------------------------------------------------------------------------\n\n");
        }

        if (advanced_mode){
            advanced_mode = false;

            (gpsLightMoisture.light.alarm) ? add_queue(BLUE) : remove_queue(BLUE);
            (gpsLightMoisture.moisture.alarm) ? add_queue(YELLOW) : remove_queue(YELLOW);
            (I2CSensor.temperature_alarm) ? add_queue(RED) : remove_queue(RED);
            (I2CSensor.humidity_alarm) ? add_queue(GREEN) : remove_queue(GREEN);
            (I2CSensor.color_alarm) ? add_queue(PURPLE) : remove_queue(PURPLE);
            (I2CSensor.accellerometer_alarm) ? add_queue(WHITE) : remove_queue(WHITE);

            printf("%s\n", gpsLightMoisture.light.advanced_data);
            printf("%s\n", gpsLightMoisture.moisture.advanced_data);
            printf("%s\n", I2CSensor.tempData);
            printf("%s\n", I2CSensor.humData);
            printf("%s\n", I2CSensor.RGBData);
            printf("%s\n", I2CSensor.accData);
            printf("\n-------------------------------------------------------------------------------------------------\n\n");

            I2CSensor.need_flash = (gpsLightMoisture.light.value < 0.2);

            if (size > 0)
                color_ticker.attach(color_sequence, COLOR_DELAY);

            i2cThread.flags_set(0x1);
            gpsLightMoistureThread.flags_set(0x1);
        }

        if (color_seq){
            color_seq = false;
            if (counter == size){
                color_ticker.detach();
                counter = 0;
                led_rgb = OFF;
            } else  { 
                led_rgb = alarm[counter];
                counter ++;
            }
        }
    }
}

