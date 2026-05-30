#ifndef __SYS_H
#define __SYS_H
#include "stm32f10x.h"

// 位带操作核心宏
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x02000000+((addr&0x00FFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))

// GPIO输入寄存器地址映射
#define GPIOA_IDR_Addr    (GPIOA_BASE+0x08)

// PA引脚输入操作（AS608用到PA6状态引脚）
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)

#endif
