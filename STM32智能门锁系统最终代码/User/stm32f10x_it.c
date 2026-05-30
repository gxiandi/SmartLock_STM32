/**
  ******************************************************************************
  * @file    stm32f10x_it.c
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   智能门锁 + AS608指纹 中断服务函数
  ******************************************************************************
  */
#include "stm32f10x_it.h"
#include "AS608.h"

// AS608全局接收变量
uint8_t rx_index = 0;
uint8_t rec_over = 0;
uint8_t aRxBuffer[RXBUFF_SIZE];
uint8_t RX_len;

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/
void NMI_Handler(void) {}
void HardFault_Handler(void) { while(1); }
void MemManage_Handler(void) { while(1); }
void BusFault_Handler(void) { while(1); }
void UsageFault_Handler(void) { while(1); }
void SVC_Handler(void) {}
void DebugMon_Handler(void) {}
void PendSV_Handler(void) {}

// 空函数：你的延时无需SysTick中断，删除无用代码
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/******************************************************************************/
void USART1_IRQHandler(void)
{
  uint8_t temp;
  
  if(USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)
  {
    USART_ReceiveData(USART1); 
  }   
  
  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
  {
    temp = USART_ReceiveData(USART1);
    if(rx_index >= RXBUFF_SIZE)
    {
      rx_index = 0;
      return;
    }
    aRxBuffer[rx_index++] = temp;
  }
  
  if(USART_GetFlagStatus(USART1, USART_FLAG_IDLE) != RESET)
  {
    USART_ReceiveData(USART1);
    RX_len = rx_index;
    rx_index = 0;
  }
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
