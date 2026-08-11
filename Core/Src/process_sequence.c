#include "main.h"
#include <stdint.h>
#include "process_sequence.h"
#include "timeline_planner.h"


        void processSequence(){


            switch(processState){
                case SYS_START:
                    if(triggerMove == 1 && state == STATE_IDLE){
                        
        
                        processState = SYS_MOTORMOVE;
                    }
                    break;

                case SYS_MOTORMOVE:
                {
                    HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, 1);
                    uint32_t targetAbs = finalTimeline[eventIndex].absoluteSteps + axisOffSteps;
                    uint32_t stepsToMove = targetAbs - currentAbsPos;
                    currentAbsPos = targetAbs;

                    float targetMM = (float)targetAbs / stepPerMM;
                    float moveMM = (float)stepsToMove / stepPerMM;
        
                // extracts whole and fractional parts
                    uint32_t moveWhole = (uint32_t)moveMM;
                    uint32_t moveFrac  = (uint32_t)((moveMM - moveWhole) * 100); // .XX
        
                    uint32_t targetWhole = (uint32_t)targetMM;
                    uint32_t targetFrac  = (uint32_t)((targetMM - targetWhole) * 100);

                    char debugBuf[96];
                    int size = sprintf(debugBuf, "Moving %lu steps (%lu.%02lu mm) -> Target: %lu steps (%lu.%02lu mm)\r\n", 
                        stepsToMove, moveWhole, moveFrac, 
                        targetAbs, targetWhole, targetFrac);

                    HAL_UART_Transmit(&huart6, (uint8_t*)debugBuf, size, 100);

                    if(stepsToMove != 0){
                        moveMotor(stepsToMove, cruiseARR);
                    }
                    processState = SYS_MOTORWAIT;
                } //
                    break;

                case SYS_MOTORWAIT:

                    if(state == STATE_IDLE){
                        processState = SYS_WAIT_BEFORE_EX;
                        delayStartTime = HAL_GetTick();
                        
                    }
                    break;

                case SYS_WAIT_BEFORE_EX:
                    if((HAL_GetTick() - delayStartTime) >= 200){
                        processState = SYS_DYC_EXTEND;
                    }
                    break;

                case SYS_DYC_EXTEND:
                    {
                    pressWait = 0;
                    char toolMsg[64];
                    int tSize = sprintf(toolMsg, "Tools Firing -> D:%d Y:%d C:%d\r\n", 
                                        finalTimeline[eventIndex].isDelme, 
                                        finalTimeline[eventIndex].isYarma, 
                                        finalTimeline[eventIndex].isKesme);
                    HAL_UART_Transmit(&huart6, (uint8_t*)toolMsg, tSize, 100);

                
                    if(finalTimeline[eventIndex].isDelme){
                        HAL_GPIO_WritePin(DELME_GPIO_Port, DELME_Pin, 0);
        
                    }
                    

                    if(finalTimeline[eventIndex].isYarma){
                        HAL_GPIO_WritePin(YARMA_GPIO_Port, YARMA_Pin, 0);

                    }

                    if(finalTimeline[eventIndex].isKesme){
                        HAL_GPIO_WritePin(KESME_GPIO_Port, KESME_Pin, 0);
                        
                    
                    }

                    processState = SYS_DYC_PRESS_WAIT;
                    delayStartTime = HAL_GetTick();

                }
                    break;

                case SYS_DYC_PRESS_WAIT:

                    if(finalTimeline[eventIndex].isDelme && 1000 > pressWait) pressWait = 1000;
                    if(finalTimeline[eventIndex].isYarma && 500 > pressWait)  pressWait = 500;
                    if(finalTimeline[eventIndex].isKesme && 400 > pressWait)  pressWait = 400;
                
                    if((HAL_GetTick() - delayStartTime) >= pressWait){
                        processState = SYS_DYC_RETRACT;
                    }
                    break;
                
                case SYS_DYC_RETRACT:

                    if(finalTimeline[eventIndex].isDelme){
                        HAL_GPIO_WritePin(DELME_GPIO_Port, DELME_Pin, 1);

                    }

                    if(finalTimeline[eventIndex].isYarma){
                        HAL_GPIO_WritePin(YARMA_GPIO_Port, YARMA_Pin, 1);

                    }

                    if(finalTimeline[eventIndex].isKesme){
                        HAL_GPIO_WritePin(KESME_GPIO_Port, KESME_Pin, 1);

                    }

                    processState = SYS_WAIT_AFTER_RET;
                    delayStartTime = HAL_GetTick();

                    break;

                case SYS_WAIT_AFTER_RET:

                    if((HAL_GetTick() - delayStartTime) >= 200){
                        eventIndex++;
                    uint32_t batchLimitSteps = (uint32_t)(((currentBatchStart + 10) * CONTA_LENGTH) * stepPerMM);
                    if (finalTimeline[eventIndex].absoluteSteps >= batchLimitSteps){
                            
                            currentBatchStart += 10;           // Slide the window forward 10 pieces
                            CompileTimeline(currentBatchStart); // recompiling the next 10

                            // find where it was left off
                            for(uint32_t i = 0; i < totalEvents; i++){ //check the absolute step of the event on the timeline and compare to current position- if it's lower it happened, if it's higher it hasn't and will be queued up
                                if(finalTimeline[i].absoluteSteps > currentAbsPos){
                                    eventIndex = i;
                                    break;
                                }
                            }
                            processState = SYS_MOTORMOVE;
                            HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, 1);
                        } else {
                            processState = SYS_MOTORMOVE;
                            HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, 1);
                        }
                    }
                    break;


                /*
                Cut branch  
                might not be needed anymore?
                

                case SYS_WAIT_AFTER_CUT:

                    if((HAL_GetTick() - delayStartTime) >= 1000){
                        triggerMove = 0;
                        processState = SYS_START;
                    }
                    
                */

            }
        
        }
