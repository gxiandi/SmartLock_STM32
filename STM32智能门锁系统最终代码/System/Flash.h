#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f10x.h"

// 补充第5张卡的FLASH地址（按你的ADDR1~ADDR4递增，比如间隔8字节）
#define FLASH_ADDR1    0x08007000  // 卡1地址（你的原有定义）
#define FLASH_ADDR2    0x08007008  // 卡2地址
#define FLASH_ADDR3    0x08007010  // 卡3地址
#define FLASH_ADDR4    0x08007018  // 卡4地址
#define FLASH_ADDR5    0x08007020  // 新增：卡5地址（关键修复）

// 声明FLASH函数（关键：补充FLASH_Init声明，消除隐式声明警告）
void FLASH_Init(void);          // 若你未实现该函数，可保留声明（编译不报错）
uint8_t FLASH_R(uint32_t addr); // 读FLASH函数
void FLASH_W(uint32_t addr, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3); // 写FLASH
void FLASH_Clear(uint32_t addr); // 清空FLASH地址

#endif
