#include "global.h"


extern UART_HandleTypeDef huart2;


unsigned char USART2RXD_loop = 0; 			//USART2 ���մ洢����
unsigned char USART2_RX_BUF[200];  			//���ջ���
unsigned char USART2_TX_BUF[200] = "0123456789";				//USART2 �ͻ���

//USART2�жϽ��մ���
unsigned int   usart2delay;           //ͨ�ų�ʱʱ�䣬�������5ms,Ϊͨ�ų�ʱ
unsigned char  FLAG_uart2;			      //UART2��ʼ�µ�һ֡���ݽ���ǰ��ʹ�ܽ��������־�����ܹ����б���Ϊ1���ڶ�ʱ���������ճ�ʱʱ������
unsigned char  En_RS485_Shandle = 0;	//����֡�����־

/**
  * @brief This function handles USART2 global interrupt / USART2 wake-up interrupt through EXTI line 26.
  */
void USART2_IRQHandler(void)
{
		unsigned char datatemp;
		HAL_UART_IRQHandler(&huart2);
		if ((__HAL_UART_GET_FLAG (&huart2, UART_FLAG_RXNE) != RESET))//��������
		 {
					datatemp = huart2.Instance->RDR;//ע�������RDR����ǰ�õĴ󲿷ֶ���DR;
				if(!FLAG_uart2)	//��ʼ�µ�һ֡ʫ�����ǰ��ʹ�ܽ��������־���ڶ�ʱ���������ճ�ʱʱ������
					 {
						 FLAG_uart2 = 1;//֡���ձ�־
						 USART2RXD_loop = 0;	//��λ����׼�������µ�һ֡
					 }
					 if(FLAG_uart2)
					 {
							if(USART2RXD_loop < 200)  //��ֹ���
								USART2_RX_BUF[USART2RXD_loop]  = datatemp;//���յ������ݴ��뻺����
						usart2delay = delaycount;//���¶�ʱ����
						USART2RXD_loop++;
					 }
		 }		
//__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE | UART_FLAG_RXFNE);
	__HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_RXNE | UART_IT_RXFNE);
	__HAL_UART_GET_IT(&huart2, UART_IT_RXNE | UART_IT_RXFNE);
	 //HAL_UART_Receive_IT(&huart2,USART2_RX_BUF, 1);
}

//**********************************************************
//*	���ܣ�USART2����һ������
//*	������buf ���ͻ������׵�ַ
//*		  len �����͵��ֽ���
//**********************************************************
void RS485_Send_Data(uint8_t *buf,uint8_t len)
{

	HAL_GPIO_WritePin(GPIOA, RS485_EN2_Pin,GPIO_PIN_SET);
	Delay_sys(10);
  HAL_UART_Transmit(&huart2, buf,len, 0xffff);
	Delay_sys(10);
	HAL_GPIO_WritePin(GPIOA, RS485_EN2_Pin,GPIO_PIN_RESET); //RS485��������
} 


void Handle_Modbus(void)
{
   if(En_RS485_Shandle!=0) 
		{
		    En_RS485_Shandle = 0;
			  RS485_Send_Data(USART2_RX_BUF,30);
		}
}
	
	
	

