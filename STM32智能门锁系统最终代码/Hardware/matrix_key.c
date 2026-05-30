#include "matrix_key.h"
#include "Delay.h"

/**
  * @brief  矩阵按键初始化（列1改为PA2，列2=PB8/列3=PB9/列4=PA0）
  */
void Key_Init1(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能GPIOB和GPIOA时钟（不变）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);
    
    // 行引脚配置（PB3-PB6，上拉输入）—— 不变
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;        
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // ========== 核心修改1：拆分列引脚初始化（PA2/PB8-PB9分属不同端口） ==========
    // 列1(PA2)+列4(PA0) 配置（GPIOA，推挽输出）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_0; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 列2(PB8)+列3(PB9) 配置（GPIOB，推挽输出）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // ========== 核心修改2：初始列置高（PA2/PA0置高，PB8/PB9置高，移除PB7） ==========
    // 初始所有列引脚置高
    GPIO_SetBits(GPIOA, GPIO_Pin_2 | GPIO_Pin_0); // 列1+列4置高
    GPIO_SetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9); // 列2+列3置高
}

/**
  * @brief  非阻塞式矩阵按键扫描（核心修改：列1改为PA2）
  * @return 按键值（按下时返回对应值，无按键返回0）
  */
static uint8_t Key_Scan1(void)
{
    uint8_t key_value = 0;

    // ===== 扫描第1列（列1→PA2）=====
    // ========== 核心修改3：所有列置高（PA2/PA0+PB8/PB9，移除PB7） ==========
    GPIO_SetBits(GPIOA, GPIO_Pin_2 | GPIO_Pin_0); // 列1+列4置高
    GPIO_SetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9); // 列2+列3置高
    GPIO_ResetBits(GPIOA, GPIO_Pin_2); // 核心修改4：第1列置低改为PA2（移除PB7）
    Delay_us(1000); 
    if ((GPIO_ReadInputData(GPIOB) & (GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6)) 
        != (GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6))
    {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3) == 0) key_value = 1;
        else if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0) key_value = 5;
        else if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) == 0) key_value = 9;
        else if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 0) key_value = 13;
    }

    // ===== 扫描第2列（列2→PB8）=====
    // ========== 修改5：列置高同步更新为PA2/PA0+PB8/PB9 ==========
    GPIO_SetBits(GPIOA, GPIO_Pin_2 | GPIO_Pin_0);
    GPIO_SetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9);
    GPIO_ResetBits(GPIOB, GPIO_Pin_8); // 第2列置低（不变）
    Delay_us(1000);
    if (key_value == 0 && (GPIO_ReadInputData(GPIOB) & (GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6)) 
        != (GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6))
    {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3) == 0) key_value = 2;
        else if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0) key_value = 6;
        else if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) == 0) key_value = 10;
        else if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 0) key_value = 14; // 删除键C
    }

    // ===== 扫描第3列（列3→PB9）=====
    // ========== 修改6：列置高同步更新为PA2/PA0+PB8/PB9 ==========
    GPIO_SetBits(GPIOA, GPIO_Pin_2 | GPIO_Pin_0);
    GPIO_SetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9);
    GPIO_ResetBits(GPIOB, GPIO_Pin_9); // 第3列置低（不变）
    Delay_us(1000);
    if (key_value == 0 && (GPIO_ReadInputData(GPIOB) & (GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6)) 
        != (GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6))
    {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3) == 0) key_value = 3;
        else if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0) key_value = 7;
        else if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) == 0) key_value = 11;
        else if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 0) key_value = 15; // 确认键D
    }

    // ===== 扫描第4列（列4→PA0）=====
    // ========== 修改7：列置高同步更新为PA2/PA0+PB8/PB9 ==========
    GPIO_SetBits(GPIOA, GPIO_Pin_2 | GPIO_Pin_0);
    GPIO_SetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9);
    GPIO_ResetBits(GPIOA, GPIO_Pin_0); // 第4列置低（不变）
    Delay_us(1000);
    if (key_value == 0 && (GPIO_ReadInputData(GPIOB) & (GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6)) 
        != (GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6))
    {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3) == 0) key_value = 4;
        else if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0) key_value = 8;
        else if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) == 0) key_value = 12;
        else if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 0) key_value = 16;
    }

    // ========== 修改8：所有列恢复高电平（同步更新为PA2/PA0+PB8/PB9） ==========
    // 所有列恢复高电平
    GPIO_SetBits(GPIOA, GPIO_Pin_2 | GPIO_Pin_0);
    GPIO_SetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9);
    
    return key_value;
}

/**
  * @brief  非阻塞式获取矩阵按键值（对外接口）—— 无修改
  * @return 按键值，无按键返回0
  */
uint8_t Key_GetNum1(void)
{
    static uint8_t last_key = 0; // 记录上一次按键值，用于消抖
    uint8_t current_key = Key_Scan1();
    
    // 消抖处理：只有当前按键按下，且上一次无按键时，才返回有效值
    if (current_key != 0 && last_key == 0)
    {
        last_key = current_key;
        return current_key;
    }
    // 按键松开时，重置last_key
    else if (current_key == 0 && last_key != 0)
    {
        last_key = 0;
    }
    
    return 0; // 非首次按下/无按键时，返回0（避免重复触发）
}
