#include "main.h"
#include <stdint.h>
#include "motor_accel.h"


        void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
        {
            if (htim->Instance == TIM1) {
                stepCounter++;

                switch (state) {

                    case STATE_ACCEL:
                    {
                        n++;
                        int32_t delta = (int32_t)(((2ULL * currentARR) + ((4 * n) + 1) / 2) / ((4 * n) + 1)); //avr446 algorithm. no point in complex floating point calculations
                        
                        if (delta < 1) {
                            delta = 1; 
                        }

                        if ((currentARR > cruiseARR) && (currentARR > delta) && ((currentARR - delta) >= cruiseARR)) {
                            currentARR -= delta;
                        } else {
                            currentARR = cruiseARR;
                            rampSteps = stepCounter;
                            state = STATE_CRUISE;
                        }

                        if (stepCounter >= (targetSteps / 2)) {
                            rampSteps = stepCounter;
                            state = STATE_DECEL;
                        }
                        break;
                    }

                    case STATE_CRUISE:
                    {
                        if (stepCounter >= (targetSteps - rampSteps)) {
                            state = STATE_DECEL;
                        }
                        break;
                    }

                    case STATE_DECEL:
                    {
                        if (n > 0) {
                            n--;
                            int32_t denom = (4 * n) - 1;
                            if (denom > 0) {
                                int32_t delta = (int32_t)(((2ULL * currentARR) + (denom / 2)) / denom); //reverse avr446
                                if (delta < 1) {
                                    delta = 1;  
                                }
                                currentARR += delta;
                            }
                            if (currentARR > accelARR) {
                                currentARR = accelARR;
                            }
                        } else {
                            currentARR = accelARR; // hold baseline speed
                        }
                        
                            

                        if (stepCounter >= targetSteps) {
                            state = STATE_IDLE;
                            motionComplete = 1;
                            __HAL_TIM_DISABLE_IT(&htim1, TIM_IT_UPDATE);
                            HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1); 
                            __HAL_TIM_MOE_DISABLE(&htim1); //kill 7 billion timers
                            return;
                        }
                        break;
                    }

                    default:
                        break;
                }


                __HAL_TIM_SET_AUTORELOAD(&htim1, currentARR);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, currentARR / 2); //50% duty cycle always!
            }
        }

