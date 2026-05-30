#include "stm32f10x.h"                  // Device header
#include "Delay.h"

void Key_Init(void)
{
	// 开启时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	// PB12(KEY1)、PB14(KEY2) 上拉输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// PA11(KEY3)、PA12(KEY4) 上拉输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

// 按键扫描函数：返回值 1/2/3/4 对应按键，0=无按键
uint8_t Key_Scan(void)
{
	uint8_t keynum = 0;  // ? 修复1：必须初始化为0（无按键）
	
	// ? 修复2：用 else if 优化逻辑，防止冲突
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) == 0)  // KEY1 = PB12
	{
		Delay_ms(20);
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) == 0)
		{
			while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) == 0); // 等待松开
			keynum = 1;
		}
	}
	else if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14) == 0) // KEY2 = PB14
	{
		Delay_ms(20);
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14) == 0)
		{
			while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14) == 0);
			keynum = 2;
		}
	}
	else if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11) == 0) // KEY3 = PA11
	{
		Delay_ms(20);
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11) == 0)
		{
			while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11) == 0);
			keynum = 3;
		}
	}
	else if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_12) == 0) // KEY4 = PA12
	{
		Delay_ms(20);
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_12) == 0)
		{
			while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_12) == 0);
			keynum = 4;
		}
	}
	
	return keynum;
}
