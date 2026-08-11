#ifndef MOTOR_MOVE_H
#define MOTOR_MOVE_H

#include <stdint.h>
#include "main.h"

typedef enum {
            STATE_IDLE,
            STATE_ACCEL,
            STATE_CRUISE,
            STATE_DECEL
        } MotionState_t;

        
extern TIM_HandleTypeDef htim1;
extern volatile MotionState_t state; 
extern volatile uint32_t targetSteps;
extern volatile uint32_t stepCounter;
extern volatile uint8_t motionComplete;
extern volatile int32_t n;
extern volatile uint32_t cruiseARR;
extern volatile uint32_t accelARR;
extern volatile uint32_t currentARR;
extern volatile uint32_t rampSteps;

void moveMotor(uint32_t steps,uint32_t targetARR);

#endif