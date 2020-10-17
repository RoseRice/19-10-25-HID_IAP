#ifndef __delay_H
#define __delay_H

#include "stm32f1xx_hal.h"


extern void DelayInit(void);
extern void setDelayUs(uint32_t nus);
extern void setDelayMs(uint16_t nms);


#endif /* __delay_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
