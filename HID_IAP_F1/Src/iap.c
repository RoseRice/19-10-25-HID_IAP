#include "iap.h"

const char * softVersion = "HID_IAP_20191023_01";


/*
** ��flashд������
*/
uint8_t iapWriteAppBin(uint32_t addr, uint8_t *buf, uint32_t size)
{
  uint8_t ret = 0;
  uint16_t w_data = 0;
  uint32_t cnt = 0;
  
  HAL_FLASH_Unlock(); // ����
  for(cnt=0; cnt<size; cnt+=2)
  {
    w_data = buf[cnt] + (buf[cnt+1]<<8);
    ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr+cnt, w_data); // д������
    if(ret != HAL_OK) break;
  }
  HAL_FLASH_Lock(); // ����
  
  return ret;
}


/*
** ����ջ����ַ
** addr:ջ����ַ
*/
__asm void MSR_MSP(uint32_t addr) 
{
	MSR MSP, r0 			//set Main Stack value
	BX r14
}

/*
** ��ת��Ӧ�ó����
** appxaddr:�û�������ʼ��ַ
*/
void iapLoadApp(uint32_t appxaddr)
{
  iapFunPtr jump2app;
  
	if(((*(vu32 *)appxaddr)&0x2FF00000)==0x20000000)	// ���ջ����ַ�Ƿ�Ϸ�.
	{
		jump2app=(iapFunPtr)*(vu32 *)(appxaddr+4); // �û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		
		MSR_MSP(*(vu32 *)appxaddr); // ��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
		jump2app(); // ��ת��APP
	}
}

uint8_t iapLoadAppCheck(uint32_t appxaddr)
{
  uint8_t ret = 0;
  
	if(((*(vu32 *)appxaddr)&0x2FF00000)==0x20000000)	// ���ջ����ַ�Ƿ�Ϸ�.
	{
		ret = APP_IS_OK; // ����������
	}
  else
  {
    ret = 1;
  }
  return ret;
}
