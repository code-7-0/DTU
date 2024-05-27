#include "ec600.h"

//uint16_t usDelayCount = 0;	  // ��ʱ����
uint8_t ucTimeOutStartFlg = 0;	  // ������ʱ������ʶ��0����ʱ����δ������1����ʱ����������
uint16_t usTimeOutCount = 0;	  // ��ʱʱ�����
uint16_t usTimeOutVal = 0;	  // ��ʱʱ��ֵ
uint8_t ucTimeOutValReachedFlg = 0; // ��ʱ�����ﵽ��ʱֵ��ʶ


//����������
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 
/**2
 * @description: �����ַ���
 * @param Ҫ������ַ���
 * @return None
 */
//void fat_uart_send_str(unsigned char *buf)
//{
//	while (*buf != '\0')
//	{
//		uart_send_byte(*buf++);
//	}
//}
void fat_uart_send_str(char *buf)
{
    if (buf == NULL) return; // Check if the buffer is not NULL
    // �����ַ�������
    uint16_t length = strlen((const char *)buf);
//		Ec600_trans_Complete=1;
//	    HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buf, length);
//		while(((&huart1)->gState) != HAL_UART_STATE_READY); 
	  //Ec600_trans_Complete=0;
	  EC600_Usart_Send((uint8_t*)buf,length); 
}
 
/**6
 * @description: ���uart���ջ��沢������ر�־λ
 * @param None
 * @return None
 */
void fat_uart_clean(void)
{
//	g_tModule.EC600_BufLen = 0;
	g_tModule.EC600_RevFlag = 0;
	memset(g_tModule.EC600_RevBuf, 0, EC600_SIZE_MAX);
//	EC600_Usart_Read((uint8_t *)g_tModule.EC600_RevBuf,EC600_SIZE_MAX);
}

/**7
 * @description: ��ʱ��������
 * @param None
 * @return None
 */
void fat_tim_proc(void)
{
//	usDelayCount++;
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
uint8_t wait_timeout(uint16_t timeoutval)
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
/**11
 * @description: ���������ֻ������ʱ�������ﵽ���ö�ʱֵ����յ�����ʱ����1
 * @param timeoutval:��ʱʱ��
 * @return 0:δ�ﵽ����ʱ��, 1:�Ѵﵽ����ʱ��
 */
uint8_t wait_timeout_nonblocking( uint16_t timeoutval)
{
	/* �������������ʱ���� */
	if (ucTimeOutStartFlg == 0)
	{ 
//		if(mqtt_revflag==0)
			fat_uart_clean();
		usTimeOutVal = timeoutval;
		ucTimeOutStartFlg = 1;
		
	}
	/* �����趨�Ķ�ʱʱ�����յ�һ֡���� */
	if (ucTimeOutValReachedFlg || g_tModule.EC600_RevFlag)
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
uint8_t fat_send_wait_cmdres_blocking(char *cmd, uint16_t timeoutval)
{
	/* �������������ʱ���� */
	if (ucTimeOutStartFlg == 0 /*& Ec600_trans_Complete==1*/)
	{ 
		if(debug_onoff.MQ_send)  
			debug_printf("uart send: %s\r\n",cmd);
//			debug_printf("uart length: %d,context:%s",strlen(cmd),cmd);
		fat_uart_clean();
		fat_uart_send_str(cmd); 
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
uint8_t fat_send_wait_cmdres_nonblocking(char *cmd, uint16_t timeoutval)
{
	/* �������������ʱ���� */
	if (ucTimeOutStartFlg == 0 /*& Ec600_trans_Complete==1*/)
	{
		if(debug_onoff.MQ_send)   debug_printf("uart send: %s\r\n",cmd);
//		debug_printf("uart length: %d,context:%s",strlen(cmd),cmd);
		fat_uart_clean();
		fat_uart_send_str(cmd);
//		Ec600_trans_Complete=0;
		usTimeOutVal = timeoutval;
		ucTimeOutStartFlg = 1;
		
	}
	/* �����趨�Ķ�ʱʱ�����յ�һ֡���� */
	if (ucTimeOutValReachedFlg || g_tModule.EC600_RevFlag)
	{
		ucTimeOutValReachedFlg = 0;
		ucTimeOutStartFlg = 0;
		usTimeOutCount = 0;
		return 1;
	}
	return 0;
}

/**12
* @description: ������Ӧ�����в��ҹؼ���
* @param <keyword> Ҫ�������Ĺؼ���
* @return 0��δ�������ؼ��֣� 1���������˹ؼ���
*/
uint8_t fat_cmdres_keyword_matching(char *keyword)
{
	/* ��������Ӧ������Ӧ�����в��ҹؼ��� */
	if (g_tModule.EC600_RevFlag)
	{
	if(debug_onoff.EC600N)  debug_printf("uart recv : %s\r\n",g_tModule.EC600_RevBuf);
		/* �����������ؼ��� */
//		uint8_t count = 0;
//		char *p = (char *)g_tModule.EC600_RevBuf;
//		while ((p = strstr(p, keyword)) != NULL) 
//		{
//				count++;
//				p++; // �ƶ�ָ����������һ������λ��
//    }
//		if(count==0)
//			g_tModule.EC600_RevFlag = 0;
//		return count; 

		if (strstr((const char *)(g_tModule.EC600_RevBuf), keyword) != NULL)
		{
			return 1;
		}
		g_tModule.EC600_RevFlag = 0;
//		DMA_EC600Usart_Read((uint8_t *)g_tModule.EC600_RevBuf,FAT_UART_RX_SIZE);
	}
	return 0;
}

