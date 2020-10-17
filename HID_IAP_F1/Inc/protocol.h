#ifndef __protocol_H
#define __protocol_H

#include "stm32f1xx_hal.h"

#define BLK_RCV_BUF_SIZE      (2*1024) // 接受数据块大小
#define FLASH_APP_ADDR        (0x08010000) // APP固件起始地址

extern uint8_t FunTransReady(void);
extern uint8_t FunTransEnd(void);
extern uint8_t FunBlockStart(void);
extern uint8_t FunBlockFinish(void);
extern uint8_t FunFrameData(void);
extern uint8_t FunControl(void);


#endif /* __delay_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
