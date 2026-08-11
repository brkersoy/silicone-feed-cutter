#ifndef TIMELINE_PLANNER_H
#define TIMELINE_PLANNER_H

#include "main.h"
#include <stdint.h>
#include <stdio.h>
#include "motor_move.h"
#include "motor_accel.h"
#include "process_sequence.h"
#include "sd_manager.h"

typedef struct { //absolute total steps
            uint32_t absoluteSteps;
            uint8_t  isDelme;
            uint8_t  isYarma;
            uint8_t  isKesme;
        } TimelineEvent_t;


extern TimelineEvent_t rawTimeline[400]; 
extern TimelineEvent_t finalTimeline[400];


void CompileTimeline(uint32_t startPiece);

        
#endif