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


void moveMotor(uint32_t steps,uint32_t targetARR);

#endif