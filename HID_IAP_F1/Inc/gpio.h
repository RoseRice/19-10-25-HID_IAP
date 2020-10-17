/**
  ******************************************************************************
  * File Name          : gpio.h
  * Description        : This file contains all the functions prototypes for 
  *                      the gpio  
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __gpio_H
#define __gpio_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

#define LED1_OFF      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET)
#define LED2_OFF      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET)
#define LED1_ON       HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET)
#define LED2_ON       HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET)
#define USB_DE_ON     HAL_GPIO_WritePin(USB_DE_GPIO_Port, USB_DE_Pin, GPIO_PIN_RESET)
#define USB_DE_OFF    HAL_GPIO_WritePin(USB_DE_GPIO_Port, USB_DE_Pin, GPIO_PIN_SET)

#define KEY1          HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4) // 读取按键1
#define KEY2          HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3) // 读取按键2
#define WK_UP         HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) // 读取按键WK_UP

#define KEY_NONE      (0)	// 没有按键按下
#define KEY1_PRES     (1)	// KEY1按下后返回值
#define KEY2_PRES     (2)	// KEY2按下后返回值
#define WKUP_PRES     (3)	// WKUP按下后返回值

typedef enum
{
  KEY_MODE_ONCE = 0,                // 不支持连续按；按下后必须松开才能第二次触发，否则不再响应这个按键，防止多次触发
  KEY_MODE_HOLD = !KEY_MODE_ONCE    // 支持连续按；如果一直按着，会一直返回这个按键的键值，方便长按检测
  
} KEY_MODE_ENUM;

/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */

extern uint8_t getKeyValue(KEY_MODE_ENUM mode);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ pinoutConfig_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
