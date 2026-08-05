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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    STATE_IDLE,
    STATE_ACCEL,
    STATE_CRUISE,
    STATE_DECEL
} MotionState_t;

typedef enum{
    SYS_IDLE, 
    SYS_START,
    SYS_MOTORMOVE,
    SYS_MOTORWAIT,
    SYS_WAIT_BEFORE_EX,
    SYS_DYC_EXTEND,
    SYS_DYC_PRESS_WAIT,
    SYS_DYC_RETRACT,
    SYS_WAIT_AFTER_RET,

//    SYS_CUT,
    SYS_WAIT_AFTER_CUT,

    SYS_DONE
}MovementState_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
volatile MotionState_t state = STATE_IDLE;
volatile uint32_t stepCounter;
volatile uint32_t targetSteps;
volatile uint32_t rampSteps;

volatile uint32_t cruiseARR = 300;
volatile uint32_t accelARR = 2000;
volatile uint32_t currentARR = 0;
volatile int32_t  n = 0;

volatile uint8_t motionComplete = 0;

uint8_t delAmount;
uint8_t yarAmount;
uint8_t dycAmount;

uint8_t rx_buff[10];

MovementState_t processState = SYS_START;

uint8_t triggerMove = 0;

uint32_t delayStartTime = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
void moveMotor(uint32_t steps,uint32_t targetARR);

void processSequence(char *dycOrder, uint32_t *stepOrder, uint32_t totalActions, uint32_t moveArr);
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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, 1); //Set direction to forward

  delAmount = 2;
  yarAmount = 1;
  dycAmount = delAmount + yarAmount;

  char *dycOrder = malloc(dycAmount * sizeof(char) + 1);
  uint32_t *stepOrder = malloc(dycAmount * sizeof(uint32_t) + 1);

  if (dycOrder == NULL || stepOrder == NULL) {
    
}

  dycOrder[0] = 'y';
  dycOrder[1] = 'd';
  dycOrder[2] = 'y';
  dycOrder[3] = 'c';


  stepOrder[0] = 2000;
  stepOrder[1] = 3000;
  stepOrder[2] = 1000;
  stepOrder[3] = 3000;


  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    
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
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */



void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1) {
        stepCounter++;

        switch (state) {

            case STATE_ACCEL:
            {
                n++;
                int32_t delta = (int32_t)(((2ULL * currentARR) + ((4 * n) + 1) / 2) / ((4 * n) + 1));
                
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
                        int32_t delta = (int32_t)(((2ULL * currentARR) + (denom / 2)) / denom);
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


////////////////////////////////////////////////////// i like a lil seperation



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

///////////////////////////////////////////////////////////////////////////////////////////

void processSequence(char *dycOrder, uint32_t *stepOrder, uint32_t totalActions, uint32_t moveArr){

    static uint8_t i = 0;


    switch(processState){
        case SYS_START:
            if(triggerMove == 1 && state == STATE_IDLE){
                i = 0;
                processState = SYS_MOTORMOVE;
            }
            break;

        case SYS_MOTORMOVE:
            moveMotor(stepOrder[i], moveArr);
            processState = SYS_MOTORWAIT;
            
            break;

        case SYS_MOTORWAIT:

            if(state == STATE_IDLE){
                processState = SYS_WAIT_BEFORE_EX;
                delayStartTime = HAL_GetTick();
                
            }
            break;

        case SYS_WAIT_BEFORE_EX:
            if((HAL_GetTick() - delayStartTime) >= 500){
                processState = SYS_DYC_EXTEND;
            }
            break;

        case SYS_DYC_EXTEND:
            if(dycOrder[i] == 'd'){
                HAL_GPIO_WritePin(DELME_GPIO_Port, DELME_Pin, 1);
                processState = SYS_DYC_PRESS_WAIT;
                delayStartTime = HAL_GetTick();
            }
            

            else if(dycOrder[i] == 'y'){
                HAL_GPIO_WritePin(YARMA_GPIO_Port, YARMA_Pin, 1);
                processState = SYS_DYC_PRESS_WAIT;
                delayStartTime = HAL_GetTick();
            }

            else if(dycOrder[i] == 'c'){
                HAL_GPIO_WritePin(KESME_GPIO_Port, KESME_Pin, 1);
                processState = SYS_DYC_PRESS_WAIT;
                delayStartTime = HAL_GetTick();
              
            }
            break;

          case SYS_DYC_PRESS_WAIT:

             if((HAL_GetTick() - delayStartTime) >= 1000){
                 processState = SYS_DYC_RETRACT;
             }
             break;
          
          case SYS_DYC_RETRACT:

              if(dycOrder[i] == 'd'){
                  HAL_GPIO_WritePin(DELME_GPIO_Port, DELME_Pin, 0);
                  processState = SYS_WAIT_AFTER_RET;
                  delayStartTime = HAL_GetTick();
              }

              else if(dycOrder[i] == 'y'){
                  HAL_GPIO_WritePin(YARMA_GPIO_Port, YARMA_Pin, 0);
                  processState = SYS_WAIT_AFTER_RET;
                  delayStartTime = HAL_GetTick();
              }

              else if(dycOrder[i] == 'c'){
                  HAL_GPIO_WritePin(KESME_GPIO_Port, KESME_Pin, 0);
                  processState = SYS_WAIT_AFTER_CUT;
                  delayStartTime = HAL_GetTick();
              }

              break;

          case SYS_WAIT_AFTER_RET:

              if((HAL_GetTick() - delayStartTime) >= 500){
                  i++;

              if (i >= totalActions) {
                  i = 0;
                  triggerMove = 0; 
                  processState = SYS_START;
              } else {
                  processState = SYS_MOTORMOVE;
              }               

              }
              break;


          /*
          Cut branch
          */

          case SYS_WAIT_AFTER_CUT:

            if((HAL_GetTick() - delayStartTime) >= 1000){
                 i = 0;
                 triggerMove = 0;
                 processState = SYS_START;
             }
              


    }
  
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
