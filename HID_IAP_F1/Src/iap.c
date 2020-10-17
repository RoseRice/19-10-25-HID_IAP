#include "iap.h"

const char * softVersion = "HID_IAP_20191023_01";


/*
** 向flash写入数据
*/
uint8_t iapWriteAppBin(uint32_t addr, uint8_t *buf, uint32_t size)
{
  uint8_t ret = 0;
  uint16_t w_data = 0;
  uint32_t cnt = 0;
  
  HAL_FLASH_Unlock(); // 解锁
  for(cnt=0; cnt<size; cnt+=2)
  {
    w_data = buf[cnt] + (buf[cnt+1]<<8);
    ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr+cnt, w_data); // 写入数据
    if(ret != HAL_OK) break;
  }
  HAL_FLASH_Lock(); // 上锁
  
  return ret;
}


/*
** 设置栈顶地址
** addr:栈顶地址
*/
__asm void MSR_MSP(uint32_t addr) 
{
	MSR MSP, r0 			//set Main Stack value
	BX r14
}

/*
** 跳转到应用程序段
** appxaddr:用户代码起始地址
*/
void iapLoadApp(uint32_t appxaddr)
{
  iapFunPtr jump2app;
  
	if(((*(vu32 *)appxaddr)&0x2FF00000)==0x20000000)	// 检查栈顶地址是否合法.
	{
		jump2app=(iapFunPtr)*(vu32 *)(appxaddr+4); // 用户代码区第二个字为程序开始地址(复位地址)		
		MSR_MSP(*(vu32 *)appxaddr); // 初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		jump2app(); // 跳转到APP
	}
}

uint8_t iapLoadAppCheck(uint32_t appxaddr)
{
  uint8_t ret = 0;
  
	if(((*(vu32 *)appxaddr)&0x2FF00000)==0x20000000)	// 检查栈顶地址是否合法.
	{
		ret = APP_IS_OK; // 可正常启动
	}
  else
  {
    ret = 1;
  }
  return ret;
}
