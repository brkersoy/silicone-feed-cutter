#ifndef TIMELINE_PLANNER_H
#define TIMELINE_PLANNER_H

#include "main.h"
#include <stdint.h>
#include <stdio.h>
#include "motor_move.h"
#include "motor_accel.h"
#include "process_sequence.h"
#include "sd_manager.h"

#define Y_OFFSET_STEPS 0.0f //yarma piston offset
#define D_OFFSET_STEPS 50.0f //delme piston offset
#define K_OFFSET_STEPS 80.0f //kesme piston offset

#define CONTA_LENGTH 1095.0f 

#define PI  3.1415


extern TimelineEvent_t rawTimeline[400]; 
extern TimelineEvent_t finalTimeline[400];


void CompileTimeline(uint32_t startPiece);

        
#endif