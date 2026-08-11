#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include "main.h"
#include <stdint.h>

extern volatile uint32_t cruiseARR;
extern volatile uint32_t accelARR; //i don't know if this is necessary, but i'm afraid to break it
extern volatile uint32_t currentARR;
extern volatile int32_t  n; //worst variable naming of all time? what does he even do?

typedef struct { //absolute total steps
            uint32_t absoluteSteps;
            uint8_t  isDelme;
            uint8_t  isYarma;
            uint8_t  isKesme;
        } TimelineEvent_t;

        typedef struct { //for ONE item only
            float localOffset; 
            uint8_t isD;
            uint8_t isY;
            uint8_t isC;
        } LocalFeature_t;

#define MAX_RECIPE_FEATURES 50

typedef struct {
    uint16_t count;
    LocalFeature_t features[MAX_RECIPE_FEATURES];
} Recipe_t;

extern uint16_t recipeSize;

extern Recipe_t activeRecipe;
extern LocalFeature_t recipe[];















#endif