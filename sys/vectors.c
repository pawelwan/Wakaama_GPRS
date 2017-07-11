#include "common.h"
//#include "stm32f4x7_eth.h"
//#include "stm32f4x7_eth_bsp.h"
#include "drivers/sd/stm32f4_sdio_sd.h"

void default_handler_c (unsigned int * hardfault_args)
{
  printf ("\n\n[Default Handler]\n");
  while (1);
}


extern xSemaphoreHandle s_xSemaphore;
extern xSemaphoreHandle ETH_link_xSemaphore;


/**
  * @brief  This function handles External line 10 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI9_5_IRQHandler(void)
{
	//xprintf("^");
	xprintf("EXTI9_5_IRQHandler\n");
  #if 0
  
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  
  if(EXTI_GetITStatus(ETH_LINK_EXTI_LINE) != RESET)
  {
  /* Give the semaphore to wakeup LwIP task */
  xSemaphoreGiveFromISR( ETH_link_xSemaphore, &xHigherPriorityTaskWoken ); 
  }
   /* Clear interrupt pending bit */
   EXTI_ClearITPendingBit(ETH_LINK_EXTI_LINE);

    /* Switch tasks if necessary. */	
  if( xHigherPriorityTaskWoken != pdFALSE )
  {
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
  }
  
  #endif
}


/**
  * @brief  This function handles ethernet DMA interrupt request.
  * @param  None
  * @retval None
  */
void ETH_IRQHandler(void)
{
}


void SDIO_IRQHandler(void)
{
	//xprintf("SDIO IRQ!\n");
	SD_ProcessIRQSrc();
}

void SD_SDIO_DMA_IRQHANDLER(void)
{
	//xprintf("SDIO DMA IRQ!\n");
	SD_ProcessDMAIRQ();
}



/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
	xprintf("NMI_Handler");
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
/*void HardFault_Handler(void)
{
  //Go to infinite loop when Hard Fault exception occurs*/
  /*while (1)
  {
  }
}*/


void hard_fault_handler_c (unsigned int * hardfault_args)
{
  printf ("\n\n[Hard fault handler]\n");
  while (1);
}


/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
	xprintf("MemManage_Handler");
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
	xprintf("BusFault_Handler");
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
	xprintf("UsageFault_Handler");
  while (1)
  {
  }
}


/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

