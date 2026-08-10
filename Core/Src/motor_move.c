
#include "main.h"
#include <stdint.h>
#include <stdlib.h>
#include "motor_move.h"


void moveMotor(uint32_t steps, uint32_t targetARR){

            if(state != STATE_IDLE){
            return;
            }

            targetSteps = steps;
            motionComplete = 0;
            stepCounter = 0;
            n = 0;

            cruiseARR = targetARR; 
            accelARR  = cruiseARR * 6;
            currentARR = accelARR;
            rampSteps = targetSteps / 2;


            __HAL_TIM_SET_AUTORELOAD(&htim1, currentARR);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, currentARR / 2); //%50 duty cycle always
            __HAL_TIM_SET_COUNTER(&htim1, 0);

        
            __HAL_TIM_ENABLE_OCxPRELOAD(&htim1, TIM_CHANNEL_1); //output compare preload
            __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);

            state = STATE_ACCEL;

            //start pwm
            __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_UPDATE);
            HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
            __HAL_TIM_MOE_ENABLE(&htim1);
            

        }