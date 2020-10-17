#include "delay.h"


static uint8_t  fac_us=0; // us延时倍乘数			   
static uint16_t fac_ms=0; // ms延时倍乘数,在ucos下,代表每个节拍的ms数

/*
** 功能：初始化延迟函数（SYSTICK的时钟固定为HCLK时钟的1/8，SYSCLK:系统时钟）
** 输入：
** 输出：
*/
void DelayInit(void)
{
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8); // 选择外部时钟  HCLK/8
  
  fac_us = SystemCoreClock/8000000; // 为系统时钟的1/8  
  fac_ms = (uint16_t)fac_us*1000; // 每个ms需要的systick时钟数   
}

/*
** 功能：微秒延迟
** 输入：nus[要延时的us数]
** 输出：
*/
void setDelayUs(uint32_t nus)
{		
	uint32_t temp;
  
	SysTick->LOAD = nus * fac_us; // 时间加载
	SysTick->VAL = 0x00; // 清空计数器
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; // 开始倒数
  
	do
	{
		temp = SysTick->CTRL;
	}while((temp&0x01) && !(temp&(1<<16))); // 等待时间到达
  
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; // 关闭计数器
	SysTick->VAL = 0X00;      					 //清空计数器	 
}

/*
** 功能：毫秒延迟（注意nms的范围）
** 输入：nms[要延时的ms数]
**      【SysTick->LOAD为24位寄存器。所以,最大延时为:nms<=0xffffff*8*1000/SYSCLK（SYSCLK单位为Hz,nms单位为ms）；对72M条件下,nms<=1864】
** 输出：
*/
void setDelayMs(uint16_t nms) // 对72M条件下,nms<=1864
{	 		  	  
	uint32_t temp;
  
	SysTick->LOAD=(uint32_t)nms*fac_ms; // 时间加载(SysTick->LOAD为24bit)
  
	SysTick->VAL = 0x00; //清空计数器
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;	//开始倒数
  
	do
	{
		temp = SysTick->CTRL;
	}while((temp&0x01) && !(temp&(1<<16))); //等待时间到达
  
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; // 关闭计数器
	SysTick->VAL = 0X00; // 清空计数器	  	    
}
