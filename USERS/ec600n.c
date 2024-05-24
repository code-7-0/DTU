#include "global.h"

extern UART_HandleTypeDef huart1;
//USART1�жϽ��մ���
unsigned int   usart1delay;           //ͨ�ų�ʱʱ�䣬�������5ms,Ϊͨ�ų�ʱ
unsigned char  FLAG_uart1;			      //UART2��ʼ�µ�һ֡���ݽ���ǰ��ʹ�ܽ��������־�����ܹ����б���Ϊ1���ڶ�ʱ���������ճ�ʱʱ������
unsigned char  Ec600_Shandle = 0;	    //����֡�����־

unsigned char USART1RXD_loop = 0; 			//USART2 ���մ洢����
char USART1_RX_BUF[FAT_UART_RX_SIZE] = {0};


//����������
void fat_uart_send_byte(unsigned char data);
void (*uart_send_byte)(unsigned char data) = fat_uart_send_byte; //���崮�ڷ���һ���ֽ����ݺ���ָ��

unsigned short int usDelayCount = 0;	  // ��ʱ����
unsigned char ucTimeOutStartFlg = 0;	  // ������ʱ������ʶ��0����ʱ����δ������1����ʱ����������
unsigned short int usTimeOutCount = 0;	  // ��ʱʱ�����
unsigned short int usTimeOutVal = 0;	  // ��ʱʱ��ֵ
unsigned char ucTimeOutValReachedFlg = 0; // ��ʱ�����ﵽ��ʱֵ��ʶ


//MQTTҵ����
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//����������
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**1
 * @description: ����һ���ֽ�����
 * @param data: Ҫ���������
 * @return None
 */
void fat_uart_send_byte(unsigned char data) {}
	
/**2
 * @description: �����ַ���
 * @param Ҫ������ַ���
 * @return None
 */
void fat_uart_send_str(unsigned char *buf)
{
	while (*buf != '\0')
	{
		uart_send_byte(*buf++);
	}
}
/**4
 * @description: ע��UART�������ݺ���
 * @param uart_send_byte: ���ͺ���ԭ��
 * @return None
 */
void reg_fat_uart_send_byte(void (*_uart_send_byte)(unsigned char data))
{
	if (_uart_send_byte != NULL)
	{
		uart_send_byte = _uart_send_byte;
	}
}
/**6
 * @description: ���uart���ջ��沢������ر�־λ
 * @param None
 * @return None
 */
void fat_uart_clean(void)
{
	USART1RXD_loop = 0;
	Ec600_Shandle = 0;
	memset(USART1_RX_BUF, 0, FAT_UART_RX_SIZE);
}

/**7
 * @description: ��ʱ��������
 * @param None
 * @return None
 */
void fat_tim_proc(void)
{
	usDelayCount++;
	/* ������ʱ���� */
	if (ucTimeOutStartFlg)
	{
		usTimeOutCount++;
		if (usTimeOutCount > usTimeOutVal)
		{
			ucTimeOutValReachedFlg = 1;
			usTimeOutCount=0;
		}
	}
}

/**11
 * @description: ������ʱ�������ﵽ���ö�ʱֵʱ����1
 * @param timeoutval:��ʱʱ��
 * @return 0:δ�ﵽ����ʱ��, 1:�Ѵﵽ����ʱ��
 */
int wait_timeout(unsigned short int timeoutval)
{
	/* ��ʼ��ʱ������ */
	if (ucTimeOutStartFlg == 0)
	{
		usTimeOutVal = timeoutval;
		ucTimeOutStartFlg = 1;
	}
	/* ��ʱʱ�䵽�� */
	if (ucTimeOutValReachedFlg)
	{
		ucTimeOutValReachedFlg = 0;
		ucTimeOutStartFlg = 0;
		usTimeOutCount = 0;
		return 1;
	}
	return 0;
}

/**8
 * @description: ��������󣬿�����ʱ�����������õĶ�ʱʱ�䵽��󷵻�1���û���ʱ���Խ��ж��������Ӧ������д���
 * @param cmd��Ҫ���͵�����
 * @param timeoutval����ʱʱ��
 * @return 0��δ���ﶨʱʱ�䣬1����ʱʱ�䵽��
 */
int fat_send_wait_cmdres_blocking(char *cmd, unsigned short int timeoutval)
{
	/* �������������ʱ���� */
	if (ucTimeOutStartFlg == 0)
	{
		//usTimeOutCount = 1;
		if(debug_onoff.MQ_send)  //printf("uart send: %s\r",cmd);
			printf("uart length: %d,context:%s",strlen(cmd),cmd);
		fat_uart_clean();
		fat_uart_send_str((unsigned char *)cmd);
		usTimeOutVal = timeoutval;
		ucTimeOutStartFlg = 1;
	}

	/* �����趨�Ķ�ʱʱ�� */
	if (ucTimeOutValReachedFlg)
	{
		ucTimeOutValReachedFlg = 0;
		ucTimeOutStartFlg = 0;
		usTimeOutCount = 0;
		return 1;
	}
	return 0;
}



/**9
 * @description: ��������󣬿�����ʱ�����������õĶ�ʱʱ�䵽������������Ӧ����ʱ����1���û���ʱ���Խ��ж��������Ӧ������д���
 * @param cmd: Ҫ���͵�����
 * @param timeoutval: ��ʱʱ��
 * @return 0:δ�ﵽ��ʱʱ�䣬1����ʱʱ�䵽�����������Ӧ����
 */
int fat_send_wait_cmdres_nonblocking(char *cmd, unsigned short int timeoutval)
{
	/* �������������ʱ���� */
	if (ucTimeOutStartFlg == 0)
	{
		if(debug_onoff.MQ_send)   printf("uart send: %s\r",cmd);
		fat_uart_clean();
		fat_uart_send_str((unsigned char *)cmd);
		usTimeOutVal = timeoutval;
		ucTimeOutStartFlg = 1;
	}
	/* �����趨�Ķ�ʱʱ�����յ�һ֡���� */
	if (ucTimeOutValReachedFlg || Ec600_Shandle)
	{
		ucTimeOutValReachedFlg = 0;
		ucTimeOutStartFlg = 0;
		usTimeOutCount = 0;
		return 1;
	}
	return 0;
}
/**10
 * @description: ��ʱ�������������ö�ʱ��ʱ�����й�
 * @param val:��ʱʱ�� val * ��ʱ��ʱ����
 * @return None
 */
void fat_delay(unsigned short int val)
{
	usDelayCount = 0;
	while (usDelayCount < val)
	{
	}
}




/**12
* @description: ������Ӧ�����в��ҹؼ���
* @param <keyword> Ҫ�������Ĺؼ���
* @return 0��δ�������ؼ��֣� 1���������˹ؼ���
*/
int fat_cmdres_keyword_matching(char *keyword)
{
	/* ��������Ӧ������Ӧ�����в��ҹؼ��� */
	if (Ec600_Shandle)
	{
		if(debug_onoff.EC600N)  printf("uart recv: %s",USART1_RX_BUF);
		/* �����������ؼ��� */
		if (strstr((const char *)(USART1_RX_BUF), keyword) != NULL)
		{
			return 1;
		}
		Ec600_Shandle = 0;
	}
	return 0;
}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//���ڽ����жϴ�����
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 /*** @brief This function handles USART1 global interrupt / USART1 wake-up interrupt through EXTI line 25.
  */
void USART1_IRQHandler(void)
{
		unsigned char datatemp;

		HAL_UART_IRQHandler(&huart1);

	 if ( (__HAL_UART_GET_FLAG (&huart1, UART_FLAG_RXNE) != RESET) )//��������
	 {
				datatemp = huart1.Instance->RDR;//ע�������RDR����ǰ�õĴ󲿷ֶ���DR;
			if(!FLAG_uart1)	//��ʼ�µ�һ֡ʫ�����ǰ��ʹ�ܽ��������־���ڶ�ʱ���������ճ�ʱʱ������
				 {
					 FLAG_uart1 = 1;//֡���ձ�־
					 USART1RXD_loop = 0;	//��λ����׼�������µ�һ֡
				 }
				 if(FLAG_uart1)
				 {
						if(USART1RXD_loop < 200)  //��ֹ���
							USART1_RX_BUF[USART1RXD_loop]  = datatemp;//���յ������ݴ��뻺����
					usart1delay = delaycount;//���¶�ʱ����
					USART1RXD_loop++;
				 }
	 }		
  __HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE | UART_FLAG_RXFNE);
	__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_RXNE | UART_IT_RXFNE);
	__HAL_UART_GET_IT(&huart1, UART_IT_RXNE | UART_IT_RXFNE);
	 
}

/**
 * @description: UART1����һ���ֽں���
 * @param None
 * @return None
 */
void uart1_send_byte(uint8_t data)
{
	HAL_UART_Transmit(&huart1, &data,1, 0xffff);

}


void Handle_Ec600n(void)
{
//    if(Ec600_Shandle)
//		{
//				Ec600_Shandle = 0;
//		    printf("%s", USART1_RX_BUF);
//		}
}

