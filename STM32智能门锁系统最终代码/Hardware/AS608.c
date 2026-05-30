#include "AS608.h"
#include "stm32f10x_usart.h"
#include <string.h>  // 新增：必须加

uint32_t AS608Addr = 0XFFFFFFFF;
SysPara AS608Para;   // 全局变量
uint16_t ValidN;     // 全局变量

void GPIO_Config_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void RCC_Configuration(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);  
}

void USART_Config_Init(void)
{
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 57600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART1,USART_IT_IDLE,ENABLE);
	USART_Cmd(USART1, ENABLE);
}

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void AS608_Init(void)
{
	RCC_Configuration();
	NVIC_Configuration();
	GPIO_Config_Init();
	USART_Config_Init();
}

static void Com_SendData(uint8_t data)
{
	USART_SendData(USART1, data);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

static void SendHead(void){Com_SendData(0xEF);Com_SendData(0x01);}
static void SendAddr(void)
{
	Com_SendData(AS608Addr>>24);
	Com_SendData(AS608Addr>>16);
	Com_SendData(AS608Addr>>8);
	Com_SendData(AS608Addr);
}
static void SendFlag(uint8_t flag){Com_SendData(flag);}
static void SendLength(int length){Com_SendData(length>>8);Com_SendData(length);}
static void Sendcmd(uint8_t cmd){Com_SendData(cmd);}
static void SendCheck(uint16_t check){Com_SendData(check>>8);Com_SendData(check);}

static uint8_t *JudgeStr(uint16_t waittime)
{
	char *data;
	uint8_t str[8];
	str[0]=0xef;str[1]=0x01;str[2]=AS608Addr>>24;
	str[3]=AS608Addr>>16;str[4]=AS608Addr>>8;
	str[5]=AS608Addr;str[6]=0x07;str[7]='\0';
	
	while(--waittime)
	{
		Delay_ms(1);
		if(RX_len)
		{
			RX_len=0;
			data=(char*)strstr((const char*)&aRxBuffer,(const char*)str); // 修复类型
			if(data)return (uint8_t*)data;	
		}
	}
	return 0;
}

uint8_t GZ_GetImage(void)
{
	uint16_t temp;uint8_t ensure;uint8_t *data;
	SendHead();SendAddr();SendFlag(0x01);SendLength(0x03);Sendcmd(0x01);
	temp=0x01+0x03+0x01;SendCheck(temp);
	data=JudgeStr(2000);
	if(data)ensure=data[9];else ensure=0xff;
	return ensure;
}

uint8_t GZ_GenChar(uint8_t BufferID)
{
	uint16_t temp;uint8_t ensure;uint8_t *data;
	SendHead();SendAddr();SendFlag(0x01);SendLength(0x04);Sendcmd(0x02);
	Com_SendData(BufferID);
	temp=0x01+0x04+0x02+BufferID;SendCheck(temp);
	data=JudgeStr(2000);
	if(data)ensure=data[9];else ensure=0xff;
	return ensure;
}

uint8_t GZ_Match(void)
{
	uint16_t temp;uint8_t ensure;uint8_t *data;
	SendHead();SendAddr();SendFlag(0x01);SendLength(0x03);Sendcmd(0x03);
	temp=0x01+0x03+0x03;SendCheck(temp);
	data=JudgeStr(2000);
	if(data)ensure=data[9];else ensure=0xff;
	return ensure;
}

uint8_t GZ_RegModel(void)
{
	uint16_t temp;uint8_t ensure;uint8_t *data;
	SendHead();SendAddr();SendFlag(0x01);SendLength(0x03);Sendcmd(0x05);
	temp=0x01+0x03+0x05;SendCheck(temp);
	data=JudgeStr(2000);
	if(data)ensure=data[9];else ensure=0xff;
	return ensure;		
}

uint8_t GZ_StoreChar(uint8_t BufferID,uint16_t PageID)
{
	uint16_t temp;uint8_t ensure;uint8_t *data;
	SendHead();SendAddr();SendFlag(0x01);SendLength(0x06);Sendcmd(0x06);
	Com_SendData(BufferID);Com_SendData(PageID>>8);Com_SendData(PageID);
	temp=0x01+0x06+0x06+BufferID+(PageID>>8)+(uint8_t)PageID;SendCheck(temp);
	data=JudgeStr(2000);
	if(data)ensure=data[9];else ensure=0xff;
	return ensure;	
}

uint8_t GZ_HighSpeedSearch(uint8_t BufferID,uint16_t StartPage,uint16_t PageNum,SearchResult *p)
{
	uint16_t temp;uint8_t ensure;uint8_t *data;
	SendHead();SendAddr();SendFlag(0x01);SendLength(0x08);Sendcmd(0x1b);
	Com_SendData(BufferID);Com_SendData(StartPage>>8);Com_SendData(StartPage);
	Com_SendData(PageNum>>8);Com_SendData(PageNum);
	temp=0x01+0x08+0x1b+BufferID+(StartPage>>8)+(uint8_t)StartPage+(PageNum>>8)+(uint8_t)PageNum;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
	{
		ensure=data[9];
		p->pageID=(data[10]<<8)+data[11];
		p->mathscore=(data[12]<<8)+data[13];
	}else ensure=0xff;
	return ensure;
}

uint8_t GZ_ValidTempleteNum(uint16_t *ValidN)
{
	uint16_t temp;uint8_t ensure;uint8_t *data;
	SendHead();SendAddr();SendFlag(0x01);SendLength(0x03);Sendcmd(0x1d);
	temp=0x01+0x03+0x1d;SendCheck(temp);
	data=JudgeStr(2000);
	if(data){ensure=data[9];*ValidN=(data[10]<<8)+data[11];}else ensure=0xff;
	return ensure;
}

uint8_t GZ_ReadSysPara(SysPara *p)
{
	uint16_t temp;uint8_t ensure;uint8_t *data;
	SendHead();SendAddr();SendFlag(0x01);SendLength(0x03);Sendcmd(0x0F);
	temp=0x01+0x03+0x0F;SendCheck(temp);
	data=JudgeStr(1000);
	if(data)
	{
		ensure=data[9];
		p->GZ_max=(data[14]<<8)+data[15];
		p->GZ_level=data[17];
		p->GZ_addr=(data[18]<<24)+(data[19]<<16)+(data[20]<<8)+data[21];
		p->GZ_size=data[23];
		p->GZ_N=data[25];
	}else ensure=0xff;
	return ensure;
}

uint8_t GZ_HandShake(uint32_t *GZ_Addr)
{
	SendHead();SendAddr();Com_SendData(0X01);Com_SendData(0X00);Com_SendData(0X00);	
	Delay_ms(200);
	if(RX_len)
	{
		RX_len=0;
		if(aRxBuffer[0]==0XEF&&aRxBuffer[1]==0X01&&aRxBuffer[6]==0X07)
		{
			*GZ_Addr=(aRxBuffer[2]<<24)+(aRxBuffer[3]<<16)+(aRxBuffer[4]<<8)+aRxBuffer[5];
			return 0;
		}
	}
	return 1;		
}
