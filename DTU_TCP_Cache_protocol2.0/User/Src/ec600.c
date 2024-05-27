#include "ec600.h"

//uint16_t usDelayCount = 0;	  // 延时计数
uint8_t ucTimeOutStartFlg = 0;	  // 开启定时计数标识，0：定时计数未开启，1，定时计数开启中
uint16_t usTimeOutCount = 0;	  // 定时时间计数
uint16_t usTimeOutVal = 0;	  // 定时时间值
uint8_t ucTimeOutValReachedFlg = 0; // 定时计数达到定时值标识


//基本处理函数
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 
/**2
 * @description: 发送字符串
 * @param 要传输的字符串
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
    // 计算字符串长度
    uint16_t length = strlen((const char *)buf);
//		Ec600_trans_Complete=1;
//	    HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buf, length);
//		while(((&huart1)->gState) != HAL_UART_STATE_READY); 
	  //Ec600_trans_Complete=0;
	  EC600_Usart_Send((uint8_t*)buf,length); 
}
 
/**6
 * @description: 清空uart接收缓存并重置相关标志位
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
 * @description: 定时器处理函数
 * @param None
 * @return None
 */
void fat_tim_proc(void)
{
//	usDelayCount++;
	/* 开启定时计数 */
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
 * @description: 开启定时计数，达到设置定时值时返回1
 * @param timeoutval:定时时间
 * @return 0:未达到设置时间, 1:已达到设置时间
 */
uint8_t wait_timeout(uint16_t timeoutval)
{
	/* 开始定时器计数 */
	if (ucTimeOutStartFlg == 0)
	{
		usTimeOutVal = timeoutval;
		ucTimeOutStartFlg = 1;
	}
	/* 定时时间到达 */
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
 * @description: 不发送命令，只开启定时计数，达到设置定时值或接收到数据时返回1
 * @param timeoutval:定时时间
 * @return 0:未达到设置时间, 1:已达到设置时间
 */
uint8_t wait_timeout_nonblocking( uint16_t timeoutval)
{
	/* 发送命令，开启定时计数 */
	if (ucTimeOutStartFlg == 0)
	{ 
//		if(mqtt_revflag==0)
			fat_uart_clean();
		usTimeOutVal = timeoutval;
		ucTimeOutStartFlg = 1;
		
	}
	/* 到达设定的定时时间或接收到一帧数据 */
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
 * @description: 发送命令后，开启定时计数，在设置的定时时间到达后返回1，用户此时可以进行对命令的响应结果进行处理
 * @param cmd：要发送的命令
 * @param timeoutval：定时时间
 * @return 0：未到达定时时间，1：定时时间到达
 */
uint8_t fat_send_wait_cmdres_blocking(char *cmd, uint16_t timeoutval)
{
	/* 发送命令，开启定时计数 */
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

	/* 到达设定的定时时间 */
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
 * @description: 发送命令后，开启定时计数，在设置的定时时间到达后或命令有响应数据时返回1，用户此时可以进行对命令的响应结果进行处理
 * @param cmd: 要发送的命令
 * @param timeoutval: 定时时间
 * @return 0:未达到定时时间，1：定时时间到达或命令有响应数据
 */
uint8_t fat_send_wait_cmdres_nonblocking(char *cmd, uint16_t timeoutval)
{
	/* 发送命令，开启定时计数 */
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
	/* 到达设定的定时时间或接收到一帧数据 */
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
* @description: 命令响应数据中查找关键字
* @param <keyword> 要被检索的关键字
* @return 0：未检索到关键字， 1：检索到了关键字
*/
uint8_t fat_cmdres_keyword_matching(char *keyword)
{
	/* 命令有响应，在响应数据中查找关键字 */
	if (g_tModule.EC600_RevFlag)
	{
	if(debug_onoff.EC600N)  debug_printf("uart recv : %s\r\n",g_tModule.EC600_RevBuf);
		/* 检索检索到关键词 */
//		uint8_t count = 0;
//		char *p = (char *)g_tModule.EC600_RevBuf;
//		while ((p = strstr(p, keyword)) != NULL) 
//		{
//				count++;
//				p++; // 移动指针以搜索下一个出现位置
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

