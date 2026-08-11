#ifndef TIMELINE_PLANNER_H
#define TIMELINE_PLANNER_H

#include "main.h"
#include <stdint.h>
#include <stdio.h>
#include "motor_move.h"
#include "motor_accel.h"
#include "process_sequence.h"

#define Y_OFFSET_STEPS 0.0f //yarma piston offset
#define D_OFFSET_STEPS 50.0f //delme piston offset
#define K_OFFSET_STEPS 80.0f //kesme piston offset

#define CONTA_LENGTH 1095.0f 

#define PI  3.1415

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


extern TimelineEvent_t rawTimeline[400]; 
extern TimelineEvent_t finalTimeline[400];
extern const uint16_t recipeSize;

extern LocalFeature_t recipe[];



void CompileTimeline(uint32_t startPiece);

        
#endif