#include "global.h"


extern UART_HandleTypeDef huart2;


unsigned char USART2RXD_loop = 0; 			//USART2 接收存储索引
unsigned char USART2_RX_BUF[200];  			//接收缓冲
unsigned char USART2_TX_BUF[200] = "0123456789";				//USART2 送缓冲

//USART2中断接收处理
unsigned int   usart2delay;           //通信超时时间，定义大于5ms,为通信超时
unsigned char  FLAG_uart2;			      //UART2开始新的一帧数据接收前先使能接收允许标志，接受过程中保持为1，在定时器发生接收超时时被清零
unsigned char  En_RS485_Shandle = 0;	//允许帧处理标志

/**
  * @brief This function handles USART2 global interrupt / USART2 wake-up interrupt through EXTI line 26.
  */
void USART2_IRQHandler(void)
{
		unsigned char datatemp;
		HAL_UART_IRQHandler(&huart2);
		if ((__HAL_UART_GET_FLAG (&huart2, UART_FLAG_RXNE) != RESET))//接收数据
		 {
					datatemp = huart2.Instance->RDR;//注意这个是RDR，以前用的大部分都是DR;
				if(!FLAG_uart2)	//开始新的一帧诗句接收前先使能接收允许标志，在定时器发生接收超时时被清零
					 {
						 FLAG_uart2 = 1;//帧接收标志
						 USART2RXD_loop = 0;	//复位索引准备接收新的一帧
					 }
					 if(FLAG_uart2)
					 {
							if(USART2RXD_loop < 200)  //防止溢出
								USART2_RX_BUF[USART2RXD_loop]  = datatemp;//接收到的数据存入缓冲器
						usart2delay = delaycount;//更新定时数据
						USART2RXD_loop++;
					 }
		 }		
//__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE | UART_FLAG_RXFNE);
	__HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_RXNE | UART_IT_RXFNE);
	__HAL_UART_GET_IT(&huart2, UART_IT_RXNE | UART_IT_RXFNE);
	 //HAL_UART_Receive_IT(&huart2,USART2_RX_BUF, 1);
}

//**********************************************************
//*	功能：USART2发送一包数据
//*	参数：buf 发送缓冲区首地址
//*		  len 待发送的字节数
//**********************************************************
void RS485_Send_Data(uint8_t *buf,uint8_t len)
{

	HAL_GPIO_WritePin(GPIOA, RS485_EN2_Pin,GPIO_PIN_SET);
	Delay_sys(10);
  HAL_UART_Transmit(&huart2, buf,len, 0xffff);
	Delay_sys(10);
	HAL_GPIO_WritePin(GPIOA, RS485_EN2_Pin,GPIO_PIN_RESET); //RS485换向引脚
} 


void Handle_Modbus(void)
{
   if(En_RS485_Shandle!=0) 
		{
		    En_RS485_Shandle = 0;
			  RS485_Send_Data(USART2_RX_BUF,30);
		}
}
	
	
	

