#ifndef PROCESS_SEQUENCE_H
#define PROCESS_SEQUENCE_H

#include "main.h"
#include <stdint.h>
#include <stdio.h>
#include "motor_move.h"
#include "motor_accel.h"



#define Y_OFFSET_STEPS 0.0f //yarma piston offset
#define D_OFFSET_STEPS 50.0f //delme piston offset
#define K_OFFSET_STEPS 80.0f //kesme piston offset
#define CONTA_LENGTH 1095.0f 
#define PI  3.1415

typedef enum{
            //SYS_IDLE, 
    SYS_START,
    SYS_MOTORMOVE,
    SYS_MOTORWAIT,
    SYS_WAIT_BEFORE_EX,
    SYS_DYC_EXTEND,
    SYS_DYC_PRESS_WAIT,
    SYS_DYC_RETRACT,
    SYS_WAIT_AFTER_RET,

}MovementState_t;



extern UART_HandleTypeDef huart6;

extern uint32_t currentBatchStart;
extern uint32_t totalEvents;
extern uint32_t pressWait;
extern uint32_t axisOffSteps;
extern uint32_t eventIndex;
extern MovementState_t processState;
extern uint8_t triggerMove;
extern uint32_t delayStartTime;
extern float stepPerMM;
extern uint32_t currentAbsPos;

#endif