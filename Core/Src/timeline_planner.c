#include "main.h" 
#include "timeline_planner.h"
#include <stdint.h>





        LocalFeature_t recipe[] = {
        {333.0f, 0, 1, 0},
        {762.0f, 0, 1, 0},
        {1095.0f, 0, 0, 1},
        }; //will somehow uh make this editable


const uint16_t recipeSize = (sizeof(recipe) / sizeof(recipe[0]));
        
TimelineEvent_t rawTimeline[400];
TimelineEvent_t finalTimeline[400];

 
 void CompileTimeline(uint32_t startPiece){

        uint32_t rawIndex = 0;

        uint32_t pastBuffer = 0;
        if (startPiece >= 5) {
            pastBuffer = startPiece - 5;
        } else {
            pastBuffer = 0;
        }//if i didn't add this it forgot the immediate next steps at the rollover on the 10th. it should be buffering the last five and the next 15 e.g. 5-25

            for(uint32_t n_buffer = pastBuffer; n_buffer < startPiece + 15; n_buffer++){
                for(uint8_t i = 0; i < recipeSize; i++){  
                
                float toolOffset = 0;

                if(recipe[i].isD){
                toolOffset = D_OFFSET_STEPS;
                }
                else if(recipe[i].isY){
                toolOffset = Y_OFFSET_STEPS;
                }
                else if(recipe[i].isC){
                toolOffset = K_OFFSET_STEPS;  
                }
                
                float absMM = (n_buffer * CONTA_LENGTH) + toolOffset + recipe[i].localOffset;

                uint32_t absSteps = (uint32_t)(absMM * stepPerMM + 0.5f);

                rawTimeline[rawIndex].absoluteSteps = absSteps;
                rawTimeline[rawIndex].isDelme = recipe[i].isD;
                rawTimeline[rawIndex].isYarma = recipe[i].isY;
                rawTimeline[rawIndex].isKesme = recipe[i].isC; //rawTimeline is the timeline before its sorted by steps from the starting point x = 0

                rawIndex++;

                }

            }

            for(uint32_t i = 0; i < rawIndex - 1; i++){ //simple bubble sort to sort rawTimeline. couldnt figure put qsort
                for(uint32_t k = 0; k < rawIndex - i -1; k++){

                if(rawTimeline[k].absoluteSteps > rawTimeline[k + 1].absoluteSteps){
                    TimelineEvent_t temp = rawTimeline[k];
                    rawTimeline[k] = rawTimeline[k + 1];
                    rawTimeline[k + 1] = temp;  
                }
                }


            

            

            }

            totalEvents = 0; //merge if multiple events happen at the same time  THIS PART MIGHT BE BROKEN!
            for (uint32_t j = 0; j < rawIndex; j++) {
                if (totalEvents > 0 && rawTimeline[j].absoluteSteps == finalTimeline[totalEvents - 1].absoluteSteps) {
        
                    finalTimeline[totalEvents - 1].isDelme |= rawTimeline[j].isDelme;
                    finalTimeline[totalEvents - 1].isYarma |= rawTimeline[j].isYarma;
                    finalTimeline[totalEvents - 1].isKesme |= rawTimeline[j].isKesme;
                } else {

                    finalTimeline[totalEvents] = rawTimeline[j];
                    totalEvents++;
                }
            

            }
            //finalTimeline is the final array to use!


        }
