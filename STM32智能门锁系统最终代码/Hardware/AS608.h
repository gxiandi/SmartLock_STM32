#ifndef __AS608_H
#define __AS608_H
#include "sys.h"
#include "Delay.h"
#include <string.h>

#define RXBUFF_SIZE	200
extern uint8_t aRxBuffer[RXBUFF_SIZE];
extern uint8_t RX_len;
extern uint8_t rx_index;

#define CharBuffer1 0x01
#define CharBuffer2 0x02

// 1. 先定义结构体（必须放前面！）
typedef struct  
{
	uint16_t pageID;
	uint16_t mathscore;
}SearchResult;

typedef struct
{
	uint16_t GZ_max;
	uint8_t  GZ_level;
	uint32_t GZ_addr;
	uint8_t  GZ_size;
	uint8_t  GZ_N;
}SysPara;

// 2. 再声明外部变量（修复顺序！）
extern uint32_t AS608Addr;
extern uint16_t ValidN;
extern SysPara AS608Para;

// 函数声明
void AS608_Init(void);
uint8_t GZ_GetImage(void);
uint8_t GZ_GenChar(uint8_t BufferID);
uint8_t GZ_Match(void);
uint8_t GZ_RegModel(void);
uint8_t GZ_StoreChar(uint8_t BufferID,uint16_t PageID);
uint8_t GZ_HighSpeedSearch(uint8_t BufferID,uint16_t StartPage,uint16_t PageNum,SearchResult *p);
uint8_t GZ_ValidTempleteNum(uint16_t *ValidN);
uint8_t GZ_ReadSysPara(SysPara *p);
uint8_t GZ_HandShake(uint32_t *GZ_Addr);

#endif
