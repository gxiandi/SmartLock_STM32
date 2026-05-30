#ifndef __MATRIX_KEY_H
#define __MATRIX_KEY_H

#include "stm32f10x.h"

// 矩阵按键引脚定义（最终版：列1=PA2，列2=PB8，列3=PB9，列4=PA0；行=PB3-PB6）
#define KEY_ROW1_PIN    GPIO_Pin_3
#define KEY_ROW2_PIN    GPIO_Pin_4
#define KEY_ROW3_PIN    GPIO_Pin_5
#define KEY_ROW4_PIN    GPIO_Pin_6
#define KEY_ROW_PORT    GPIOB

// ========== 核心修改：列1从PB7改为PA2 ==========
#define KEY_COL1_PIN    GPIO_Pin_2  // 列1 → PA2（原PB7，解决引脚冲突）
#define KEY_COL2_PIN    GPIO_Pin_8  // 列2 → PB8
#define KEY_COL3_PIN    GPIO_Pin_9  // 列3 → PB9
#define KEY_COL4_PIN    GPIO_Pin_0  // 列4 → PA0
#define KEY_COL1_PORT   GPIOA       // 列1端口改为GPIOA
#define KEY_COL23_PORT  GPIOB       // 列2/3端口为GPIOB
#define KEY_COL4_PORT   GPIOA       // 列4端口为GPIOA

// 按键编号定义（不变，与功能完全匹配）
#define KEY_NONE        0
#define KEY_1           1
#define KEY_2           2
#define KEY_3           3
#define KEY_4           4
#define KEY_5           5
#define KEY_6           6
#define KEY_7           7
#define KEY_8           8
#define KEY_9           9
#define KEY_0           10
#define KEY_DEL         14  // 删除键（行4列2，PB6+PB8）
#define KEY_CONFIRM     15  // 确认键（行4列3，PB6+PB9）
#define KEY_EXIT        16  // 退出键（行4列4，PB6+PA0）

// 函数声明（不变）
void Key_Init1(void);
uint8_t Key_GetNum1(void);

#endif
