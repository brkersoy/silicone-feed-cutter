#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include "main.h"
#include <stdint.h>

extern volatile uint32_t cruiseARR;
extern volatile uint32_t accelARR; //i don't know if this is necessary, but i'm afraid to break it
extern volatile uint32_t currentARR;
extern float radius;

#define Y_OFFSET_STEPS 0.0f //yarma piston offset
#define D_OFFSET_STEPS 50.0f //delme piston offset
#define K_OFFSET_STEPS 80.0f //kesme piston offset

#define CONTA_LENGTH 1095.0f 

#define PI  3.1415

typedef struct { //for ONE item only
            float localOffset; 
            uint8_t isD;
            uint8_t isY;
            uint8_t isC;
        } LocalFeature_t;

typedef struct { //recipe of the locations, this is what defines the gasket
    uint16_t count;
    LocalFeature_t features[100];
} Recipe_t;

extern Recipe_t recipe;

#endif