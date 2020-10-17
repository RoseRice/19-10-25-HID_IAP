#include "delay.h"


static uint8_t  fac_us=0; // us��ʱ������			   
static uint16_t fac_ms=0; // ms��ʱ������,��ucos��,����ÿ�����ĵ�ms��

/*
** ���ܣ���ʼ���ӳٺ�����SYSTICK��ʱ�ӹ̶�ΪHCLKʱ�ӵ�1/8��SYSCLK:ϵͳʱ�ӣ�
** ���룺
** �����
*/
void DelayInit(void)
{
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8); // ѡ���ⲿʱ��  HCLK/8
  
  fac_us = SystemCoreClock/8000000; // Ϊϵͳʱ�ӵ�1/8  
  fac_ms = (uint16_t)fac_us*1000; // ÿ��ms��Ҫ��systickʱ����   
}

/*
** ���ܣ�΢���ӳ�
** ���룺nus[Ҫ��ʱ��us��]
** �����
*/
void setDelayUs(uint32_t nus)
{		
	uint32_t temp;
  
	SysTick->LOAD = nus * fac_us; // ʱ�����
	SysTick->VAL = 0x00; // ��ռ�����
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; // ��ʼ����
  
	do
	{
		temp = SysTick->CTRL;
	}while((temp&0x01) && !(temp&(1<<16))); // �ȴ�ʱ�䵽��
  
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; // �رռ�����
	SysTick->VAL = 0X00;      					 //��ռ�����	 
}

/*
** ���ܣ������ӳ٣�ע��nms�ķ�Χ��
** ���룺nms[Ҫ��ʱ��ms��]
**      ��SysTick->LOADΪ24λ�Ĵ���������,�����ʱΪ:nms<=0xffffff*8*1000/SYSCLK��SYSCLK��λΪHz,nms��λΪms������72M������,nms<=1864��
** �����
*/
void setDelayMs(uint16_t nms) // ��72M������,nms<=1864
{	 		  	  
	uint32_t temp;
  
	SysTick->LOAD=(uint32_t)nms*fac_ms; // ʱ�����(SysTick->LOADΪ24bit)
  
	SysTick->VAL = 0x00; //��ռ�����
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;	//��ʼ����
  
	do
	{
		temp = SysTick->CTRL;
	}while((temp&0x01) && !(temp&(1<<16))); //�ȴ�ʱ�䵽��
  
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; // �رռ�����
	SysTick->VAL = 0X00; // ��ռ�����	  	    
}
