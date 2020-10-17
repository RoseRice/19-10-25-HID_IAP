#ifndef __IAP__H__
#define __IAP__H__

#include "stm32f1xx_hal.h"

#include "stm32f1xx_hal_flash.h"
#include "stm32f1xx_hal_flash_ex.h"

#define APP_IS_OK     (0xA5)

typedef volatile uint32_t  vu32;
typedef  void (*iapFunPtr)(void);				//定义一个函数类型的参数

#define SOFT_VERSION    ("HID_IAP_20191023_01_def")

extern const char * softVersion;

extern uint8_t iapWriteAppBin(uint32_t addr, uint8_t *buf, uint32_t size);
extern void iapLoadApp(uint32_t appxaddr);
extern uint8_t iapLoadAppCheck(uint32_t appxaddr);

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
