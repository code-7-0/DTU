#include "global.h"

extern UART_HandleTypeDef huart1;
//USART1中断接收处理
unsigned int   usart1delay;           //通信超时时间，定义大于5ms,为通信超时
unsigned char  FLAG_uart1;			      //UART2开始新的一帧数据接收前先使能接收允许标志，接受过程中保持为1，在定时器发生接收超时时被清零
unsigned char  Ec600_Shandle = 0;	    //允许帧处理标志

unsigned char USART1RXD_loop = 0; 			//USART2 接收存储索引
char USART1_RX_BUF[FAT_UART_RX_SIZE] = {0};


//基本处理函数
void fat_uart_send_byte(unsigned char data);
void (*uart_send_byte)(unsigned char data) = fat_uart_send_byte; //定义串口发送一个字节数据函数指针

unsigned short int usDelayCount = 0;	  // 延时计数
unsigned char ucTimeOutStartFlg = 0;	  // 开启定时计数标识，0：定时计数未开启，1，定时计数开启中
unsigned short int usTimeOutCount = 0;	  // 定时时间计数
unsigned short int usTimeOutVal = 0;	  // 定时时间值
unsigned char ucTimeOutValReachedFlg = 0; // 定时计数达到定时值标识


//MQTT业务函数
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//基本处理函数
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**1
 * @description: 发送一个字节数据
 * @param data: 要传输的数据
 * @return None
 */
void fat_uart_send_byte(unsigned char data) {}
	
/**2
 * @description: 发送字符串
 * @param 要传输的字符串
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
 * @description: 注册UART发送数据函数
 * @param uart_send_byte: 发送函数原型
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
 * @description: 清空uart接收缓存并重置相关标志位
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
 * @description: 定时器处理函数
 * @param None
 * @return None
 */
void fat_tim_proc(void)
{
	usDelayCount++;
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
int wait_timeout(unsigned short int timeoutval)
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

/**8
 * @description: 发送命令后，开启定时计数，在设置的定时时间到达后返回1，用户此时可以进行对命令的响应结果进行处理
 * @param cmd：要发送的命令
 * @param timeoutval：定时时间
 * @return 0：未到达定时时间，1：定时时间到达
 */
int fat_send_wait_cmdres_blocking(char *cmd, unsigned short int timeoutval)
{
	/* 发送命令，开启定时计数 */
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
int fat_send_wait_cmdres_nonblocking(char *cmd, unsigned short int timeoutval)
{
	/* 发送命令，开启定时计数 */
	if (ucTimeOutStartFlg == 0)
	{
		if(debug_onoff.MQ_send)   printf("uart send: %s\r",cmd);
		fat_uart_clean();
		fat_uart_send_str((unsigned char *)cmd);
		usTimeOutVal = timeoutval;
		ucTimeOutStartFlg = 1;
	}
	/* 到达设定的定时时间或接收到一帧数据 */
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
 * @description: 延时函数，根据所用定时器时间间隔有关
 * @param val:延时时间 val * 定时器时间间隔
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
* @description: 命令响应数据中查找关键字
* @param <keyword> 要被检索的关键字
* @return 0：未检索到关键字， 1：检索到了关键字
*/
int fat_cmdres_keyword_matching(char *keyword)
{
	/* 命令有响应，在响应数据中查找关键字 */
	if (Ec600_Shandle)
	{
		if(debug_onoff.EC600N)  printf("uart recv: %s",USART1_RX_BUF);
		/* 检索检索到关键词 */
		if (strstr((const char *)(USART1_RX_BUF), keyword) != NULL)
		{
			return 1;
		}
		Ec600_Shandle = 0;
	}
	return 0;
}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//串口接收中断处理函数
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 /*** @brief This function handles USART1 global interrupt / USART1 wake-up interrupt through EXTI line 25.
  */
void USART1_IRQHandler(void)
{
		unsigned char datatemp;

		HAL_UART_IRQHandler(&huart1);

	 if ( (__HAL_UART_GET_FLAG (&huart1, UART_FLAG_RXNE) != RESET) )//接收数据
	 {
				datatemp = huart1.Instance->RDR;//注意这个是RDR，以前用的大部分都是DR;
			if(!FLAG_uart1)	//开始新的一帧诗句接收前先使能接收允许标志，在定时器发生接收超时时被清零
				 {
					 FLAG_uart1 = 1;//帧接收标志
					 USART1RXD_loop = 0;	//复位索引准备接收新的一帧
				 }
				 if(FLAG_uart1)
				 {
						if(USART1RXD_loop < 200)  //防止溢出
							USART1_RX_BUF[USART1RXD_loop]  = datatemp;//接收到的数据存入缓冲器
					usart1delay = delaycount;//更新定时数据
					USART1RXD_loop++;
				 }
	 }		
  __HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE | UART_FLAG_RXFNE);
	__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_RXNE | UART_IT_RXFNE);
	__HAL_UART_GET_IT(&huart1, UART_IT_RXNE | UART_IT_RXFNE);
	 
}

/**
 * @description: UART1发送一个字节函数
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

