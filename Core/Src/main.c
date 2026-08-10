        /* USER CODE BEGIN Header */
        /**
         ******************************************************************************
        * @file           : main.c
        * @brief          : Main program body
        ******************************************************************************
        * @attention
        *
        * Copyright (c) 2026 STMicroelectronics.
        * All rights reserved.
        *
        * This software is licensed under terms that can be found in the LICENSE file
        * in the root directory of this software component.
        * If no LICENSE file comes with this software, it is provided AS-IS.
        *
        ******************************************************************************
        */
        /* USER CODE END Header */
        /* Includes ------------------------------------------------------------------*/
        #include "main.h"

        /* Private includes ----------------------------------------------------------*/
        /* USER CODE BEGIN Includes */
        #include <stdio.h>
        #include <stdlib.h>
        #include "motor_move.h"
        #include "motor_accel.h"

        /* USER CODE END Includes */

        /* Private typedef -----------------------------------------------------------*/
        /* USER CODE BEGIN PTD */

    /*
        typedef enum {
            STATE_IDLE,
            STATE_ACCEL,
            STATE_CRUISE,
            STATE_DECEL
        } MotionState_t;

    */

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

        //    SYS_CUT,
        //   SYS_WAIT_AFTER_CUT,

        // SYS_DONE
        }MovementState_t;

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

        int ab;
        /* USER CODE END PTD */

        /* Private define ------------------------------------------------------------*/
        /* USER CODE BEGIN PD */

        #define Y_OFFSET_STEPS 0.0f //yarma piston offset
        #define D_OFFSET_STEPS 50.0f //delme piston offset
        #define K_OFFSET_STEPS 80.0f //kesme piston offset

        #define CONTA_LENGTH 1095.0f 

        #define PI  3.1415
        /* USER CODE END PD */

        /* Private macro -------------------------------------------------------------*/
        /* USER CODE BEGIN PM */

        /* USER CODE END PM */

        /* Private variables ---------------------------------------------------------*/
        TIM_HandleTypeDef htim1;

        UART_HandleTypeDef huart6;

        /* USER CODE BEGIN PV */
        uint32_t eventIndex = 0;
        uint32_t currentAbsPos = 0;

        uint32_t pressWait;

        float radius = 18.93;
        float circumference;
        float stepPerMM;

        volatile MotionState_t state = STATE_IDLE;
        volatile uint32_t stepCounter;
        volatile uint32_t targetSteps;
        volatile uint32_t rampSteps;

        volatile uint32_t cruiseARR = 150;
        volatile uint32_t accelARR = 1200;
        volatile uint32_t currentARR = 0;
        volatile int32_t  n = 0;

        volatile uint8_t motionComplete = 0;

        uint8_t rx_buff[10];

        MovementState_t processState = SYS_START;

        uint8_t triggerMove = 0;

        uint32_t delayStartTime = 0;

        //i am so confused

        TimelineEvent_t rawTimeline[200]; 
        TimelineEvent_t finalTimeline[200]; //200 events for 1 line, which i believe we should never hit that much, im guessing no more than like 60
        uint32_t totalEvents = 0;

        LocalFeature_t recipe[] = {
        {333.0f, 0, 1, 0},
        {762.0f, 0, 1, 0},
        {1095.0f, 0, 0, 1},
        }; //will somehow uh make this editable
        #define RECIPE_SIZE (sizeof(recipe) / sizeof(recipe[0]))
        

        uint32_t axisOffSteps = 0;

        uint32_t currentBatchStart = 0;
        /* USER CODE END PV */

        /* Private function prototypes -----------------------------------------------*/
        void SystemClock_Config(void);
        static void MX_GPIO_Init(void);
        static void MX_TIM1_Init(void);
        static void MX_USART6_UART_Init(void);
        /* USER CODE BEGIN PFP */
        void moveMotor(uint32_t steps,uint32_t targetARR);
        void processSequence(void);
        void CompileTimeline(uint32_t startPiece);

        /* USER CODE END PFP */

        /* Private user code ---------------------------------------------------------*/
        /* USER CODE BEGIN 0 */

        /* USER CODE END 0 */

        /**
         * @brief  The application entry point.
         * @retval int
         */
        int main(void)
        {

        /* USER CODE BEGIN 1 */
        circumference = 2 * PI * radius; //around 0.037mm/step now
        stepPerMM = 3200.0f / circumference;


        /*

        TODO

        SEPERATE INTO DIFFERENT FILES
        IMPLEMENT ARR INPUT
        ADD STEP/MM SCALING

        */


        /* USER CODE END 1 */

        /* MCU Configuration--------------------------------------------------------*/

        /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
        HAL_Init();

        /* USER CODE BEGIN Init */

        /* USER CODE END Init */

        /* Configure the system clock */
        SystemClock_Config();

        /* USER CODE BEGIN SysInit */

        /* USER CODE END SysInit */

        /* Initialize all configured peripherals */
        MX_GPIO_Init();
        MX_TIM1_Init();
        MX_USART6_UART_Init();
        /* USER CODE BEGIN 2 */

        HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, 1); //Set direction to forward
        HAL_GPIO_WritePin(YARMA_GPIO_Port, YARMA_Pin, 1);
        HAL_GPIO_WritePin(KESME_GPIO_Port, KESME_Pin, 1);
        HAL_GPIO_WritePin(DELME_GPIO_Port, DELME_Pin, 1);

        HAL_GPIO_WritePin(KESME_GPIO_Port, KESME_Pin, 0);
        HAL_Delay(1500);
        HAL_GPIO_WritePin(KESME_GPIO_Port, KESME_Pin, 1);
        HAL_Delay(500);

        currentAbsPos = (uint32_t)(K_OFFSET_STEPS * stepPerMM + 0.5f);


        HAL_GPIO_WritePin(GPIOA, LD2_Pin, GPIO_PIN_SET); // Turn on onboard LED
        char *testMsg = "UART is alive!\r\n";
        HAL_UART_Transmit(&huart6, (uint8_t*)testMsg, 16, 1000);

        CompileTimeline(0);

        eventIndex = 0;
        for (uint32_t i = 0; i < totalEvents; i++) {
            if (finalTimeline[i].absoluteSteps > currentAbsPos) {
                eventIndex = i;
                break;
            }
        }

        char msg[64];
        int len = sprintf(msg, "Total Events: %lu\r\n", totalEvents);
        HAL_UART_Transmit(&huart6, (uint8_t*)msg, len, 100);

        triggerMove = 1;
        
        /* USER CODE END 2 */

        /* Infinite loop */
        /* USER CODE BEGIN WHILE */
        while (1)
        {
            processSequence();
            /* USER CODE END WHILE */

            /* USER CODE BEGIN 3 */
        }
        /* USER CODE END 3 */
        }

        /**
         * @brief System Clock Configuration
         * @retval None
         */
        void SystemClock_Config(void)
        {
        RCC_OscInitTypeDef RCC_OscInitStruct = {0};
        RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

        /** Configure the main internal regulator output voltage
         */
        __HAL_RCC_PWR_CLK_ENABLE();
        __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

        /** Initializes the RCC Oscillators according to the specified parameters
         * in the RCC_OscInitTypeDef structure.
         */
        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
        RCC_OscInitStruct.HSIState = RCC_HSI_ON;
        RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
        RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
        RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
        RCC_OscInitStruct.PLL.PLLM = 8;
        RCC_OscInitStruct.PLL.PLLN = 100;
        RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
        RCC_OscInitStruct.PLL.PLLQ = 4;
        RCC_OscInitStruct.PLL.PLLR = 2;
        if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        {
            Error_Handler();
        }

        /** Initializes the CPU, AHB and APB buses clocks
         */
        RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                    |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
        RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
        RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
        RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
        RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

        if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
        {
            Error_Handler();
        }
        }

        /**
         * @brief TIM1 Initialization Function
         * @param None
         * @retval None
         */
        static void MX_TIM1_Init(void)
        {

        /* USER CODE BEGIN TIM1_Init 0 */

        /* USER CODE END TIM1_Init 0 */

        TIM_ClockConfigTypeDef sClockSourceConfig = {0};
        TIM_MasterConfigTypeDef sMasterConfig = {0};
        TIM_OC_InitTypeDef sConfigOC = {0};
        TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

        /* USER CODE BEGIN TIM1_Init 1 */

        /* USER CODE END TIM1_Init 1 */
        htim1.Instance = TIM1;
        htim1.Init.Prescaler = 99;
        htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
        htim1.Init.Period = 1000;
        htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        htim1.Init.RepetitionCounter = 0;
        htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
        if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
        {
            Error_Handler();
        }
        sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
        if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
        {
            Error_Handler();
        }
        if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
        {
            Error_Handler();
        }
        sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
        sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
        if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
        {
            Error_Handler();
        }
        sConfigOC.OCMode = TIM_OCMODE_PWM1;
        sConfigOC.Pulse = 50;
        sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
        sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
        sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
        sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
        sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
        if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
        {
            Error_Handler();
        }
        sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
        sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
        sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
        sBreakDeadTimeConfig.DeadTime = 0;
        sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
        sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
        sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
        if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
        {
            Error_Handler();
        }
        /* USER CODE BEGIN TIM1_Init 2 */

        /* USER CODE END TIM1_Init 2 */
        HAL_TIM_MspPostInit(&htim1);

        }

        /**
         * @brief USART6 Initialization Function
         * @param None
         * @retval None
         */
        static void MX_USART6_UART_Init(void)
        {

        /* USER CODE BEGIN USART6_Init 0 */

        /* USER CODE END USART6_Init 0 */

        /* USER CODE BEGIN USART6_Init 1 */

        /* USER CODE END USART6_Init 1 */
        huart6.Instance = USART6;
        huart6.Init.BaudRate = 115200;
        huart6.Init.WordLength = UART_WORDLENGTH_8B;
        huart6.Init.StopBits = UART_STOPBITS_1;
        huart6.Init.Parity = UART_PARITY_NONE;
        huart6.Init.Mode = UART_MODE_TX_RX;
        huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
        huart6.Init.OverSampling = UART_OVERSAMPLING_16;
        if (HAL_UART_Init(&huart6) != HAL_OK)
        {
            Error_Handler();
        }
        /* USER CODE BEGIN USART6_Init 2 */

        /* USER CODE END USART6_Init 2 */

        }

        /**
         * @brief GPIO Initialization Function
         * @param None
         * @retval None
         */
        static void MX_GPIO_Init(void)
        {
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        /* USER CODE BEGIN MX_GPIO_Init_1 */

        /* USER CODE END MX_GPIO_Init_1 */

        /* GPIO Ports Clock Enable */
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOH_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /*Configure GPIO pin Output Level */
        HAL_GPIO_WritePin(GPIOA, DELME_Pin|YARMA_Pin|KESME_Pin|LD2_Pin
                                |DIR_Pin, GPIO_PIN_RESET);

        /*Configure GPIO pin : B1_Pin */
        GPIO_InitStruct.Pin = B1_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

        /*Configure GPIO pins : DELME_Pin YARMA_Pin KESME_Pin LD2_Pin
                                DIR_Pin */
        GPIO_InitStruct.Pin = DELME_Pin|YARMA_Pin|KESME_Pin|LD2_Pin
                                |DIR_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /*Configure GPIO pins : USART_TX_Pin USART_RX_Pin */
        GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USER CODE BEGIN MX_GPIO_Init_2 */

        /* USER CODE END MX_GPIO_Init_2 */
        }

        /* USER CODE BEGIN 4 */


/*
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

*/


        ////////////////////////////////////////////////////// i like a lil seperation

/*

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

    */

        ///////////////////////////////////////////////////////////////////////////////////////////

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

                    if(finalTimeline[eventIndex].isDelme){
                        pressWait = 1000;
        
                    }
                    

                    if(finalTimeline[eventIndex].isYarma){
                        pressWait = 500;
                        

                    }

                    if(finalTimeline[eventIndex].isKesme){
                        pressWait = 400;  
                        
                    
                    }
                    
                
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

        //you wiiiill neveeer love me agaiin

        void CompileTimeline(uint32_t startPiece){

        uint32_t rawIndex = 0;

        uint32_t pastBuffer = (startPiece >= 5) ? (startPiece - 5) : 0; //if i didn't add this it forgot the immediate next steps at the rollover on the 10th. it should be buffering the last five and the next 15 e.g. 5-25

            for(uint32_t n_buffer = pastBuffer; n_buffer < startPiece + 15; n_buffer++){
                for(uint8_t i = 0; i < RECIPE_SIZE; i++){  //MAKE i HERE CHANGEABLE
                
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




        /* USER CODE END 4 */

        /**
         * @brief  This function is executed in case of error occurrence.
         * @retval None
         */
        void Error_Handler(void)
        {
        /* USER CODE BEGIN Error_Handler_Debug */
        /* User can add his own implementation to report the HAL error return state */
        __disable_irq();
        while (1)
        {
        }
        /* USER CODE END Error_Handler_Debug */
        }
        #ifdef USE_FULL_ASSERT
        /**
         * @brief  Reports the name of the source file and the source line number
         *         where the assert_param error has occurred.
         * @param  file: pointer to the source file name
         * @param  line: assert_param error line source number
         * @retval None
         */
        void assert_failed(uint8_t *file, uint32_t line)
        {
        /* USER CODE BEGIN 6 */
        /* User can add his own implementation to report the file name and line number,
            ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
        /* USER CODE END 6 */
        }
        #endif /* USE_FULL_ASSERT */
