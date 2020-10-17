/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "delay.h"
#include "usbd_custom_hid_if.h"
#include "protocol.h"
#include "iap.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

static FUN_HAND_STU FunHand[] = 
{
  { 0x01, FunTransReady },    // 准备开始数据传输
  { 0x02, FunTransEnd },      // 数据传输完成
  { 0x10, FunBlockStart },    // 启动数据块传输
  { 0x11, FunBlockFinish },   // 数据块传输结束
  { 0x20, FunFrameData },     // 帧传输
  { 0x80, FunControl }        // 控制命令
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
  
  uint8_t loop = 0;
  uint32_t delay_cnt = 0;
  
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
  /* USER CODE BEGIN 2 */
  
  DelayInit();
  
  if(KEY_NONE == getKeyValue(KEY_MODE_HOLD))
  {
    //执行FLASH APP代码
    if(((*(vu32 *)(FLASH_APP_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
    {	 
      iapLoadApp(FLASH_APP_ADDR);
    }
    else
    {
      // 指示灯变换频率地双闪
      while(1)
      {
        if(500 < delay_cnt)  delay_cnt = 0;
        
        delay_cnt += (delay_cnt>>4) + 1;
        
        LED1_ON;
        LED2_OFF;
        setDelayUs(delay_cnt*delay_cnt);
        LED1_OFF;
        LED2_ON;
        setDelayUs(delay_cnt*delay_cnt);
      }
    }
  }
  else
  {
    USB_DE_ON; // 启用USB设备
    MX_USB_DEVICE_Init();
    memset(&RcvUSB, 0x00, sizeof(USB_RCV_STU));
    memset(&SndUSB, 0x00, sizeof(USB_SND_STU));
  }
  
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if(0 == (delay_cnt%40000))
    {
      LED1_OFF;
    }
    
    if(0 == (delay_cnt%80000))
    {
      LED1_ON;
    }
    delay_cnt++;
    
    
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    if(TRUE_t == RcvUSB.hasData)
    {
      RcvUSB.hasData = FALSE_t; // 每帧数据仅解析一次
      
      LED2_ON;
      
      // 解析命令码，并执行处理函数
      for(loop=0; loop<numof(FunHand); loop++)
      {
        if(RcvUSB.buf[0] == FunHand[loop].cmdCode) // 命令码匹配
        {
          FunHand[loop].fun(); // 命令处理函数
          
          if(TRUE_t == SndUSB.hasData)
          {
            SndUSB.hasData = FALSE_t; // 只用发送一次数据
            
            USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, SndUSB.buf, sizeof(SndUSB.buf));
            memset(SndUSB.buf, 0x00, sizeof(SndUSB.buf));
          }
          break; // 命令解析完成，退出循环
        }
      }
      
      memset(RcvUSB.buf, 0x00, sizeof(RcvUSB.buf)); // 接收数据处理完成，缓存清空
      
      LED2_OFF;
    }
    
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
