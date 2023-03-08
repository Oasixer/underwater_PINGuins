#ifndef MS5839_H
#define MS5839_H

//Datasheet here: https://www.mouser.ca/datasheet/2/418/8/ENG_DS_MS5839_02BA_A4-1955832.pdf

#include <stdint.h>
#include <stdbool.h>
#include <Wire.h>
//#include <SoftWire.h>

// Chip Deffs
#define I2C_ADDRESS   0x76
#define RESET_COMMAND 0x1E
#define READ_COMMAND 0x00
#define PROM_READ_COMMAND 0xA0 //for adress 0
#define SCL_PIN A13
#define SDA_PIN A12


//Physics Deffs
#define WATER_DENSITY 997 // kg/m3

#define PRESS_ZERO 99972 //Pa


typedef enum {
    PRESS = 0,
    TEMP = 1
} measure_t;

typedef enum {
    RES_256 = 0,
    RES_512 = 1,
    RES_1024 = 2,
    RES_2048 = 3,
    RES_4096 = 4,
    RES_8192 = 5    
} resolution_t;

static struct MS5839_t{
  //Calibration constants (READ THE DATASHEET!)(coefs are 1 less here to avoid starting indexing at 1)
  uint16_t C[7];
  uint32_t D1; //digital pressure value
  uint32_t D2; //digital pressure value
  int32_t  dT; //diff from ref temp
  int32_t temp; //1st order compensated tempurature value
  int32_t press; //1st order compensated pressure value

} MS5839_struct;

void MS5839_init(void);

void MS5839_start_conversion(measure_t type, resolution_t resolution);

int32_t MS5839_read_conversion(void); 

//WARNING! SLOW! will take ~20ms to run!
int32_t MS5839_read_temp(void);

//WARNING! SLOW! will take ~20ms to run! USES LAST MEASURED TEMP
int32_t MS5839_read_press(void);

//uses previous press measurement! in mm
int32_t MS5839_calc_depth(void);

// returns depth in mm
int32_t get_depth_mm_50ms(void);






#endif