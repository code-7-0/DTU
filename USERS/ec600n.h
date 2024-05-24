
#include "main.h"
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ec600n_H
#define __ec600n_H
#ifdef __cplusplus
 extern "C" {
#endif
#define FAT_UART_RX_SIZE 512    //fat 串口接收缓存大小	 
#define FAT_TIMER_VAL  1     //fat 定时器间隔   

//USART1中断接收处理
extern unsigned int   usart1delay;           //通信超时时间，定义大于5ms,为通信超时
extern unsigned char  FLAG_uart1;			      //UART2开始新的一帧数据接收前先使能接收允许标志，接受过程中保持为1，在定时器发生接收超时时被清零
extern unsigned char  Ec600_Shandle;	//允许帧处理标志
	 

extern char USART1_RX_BUF[FAT_UART_RX_SIZE];
void fat_tim_proc(void);                                                 //fat 定时器处理，需要用户开启的定时器中断处理中
	 
void reg_fat_uart_send_byte(void (*uart_send_byte)(unsigned char data)); //注册 串口发送一个字节函数
	 
int wait_timeout(unsigned short int time); // 开启定时计数，达到设置定时值时返回1	 
	 
int fat_send_wait_cmdres_blocking(char *cmd, unsigned short int timeoutval);    //发送命令后，开启定时计数，在设置的定时时间到达后返回1，用户此时可以进行对命令的响应结果进行处理
int fat_send_wait_cmdres_nonblocking(char *cmd, unsigned short int timeoutval); //发送命令后，开启定时计数，在设置的定时时间到达后或命令有响应数据时返回1，用户此时可以进行对命令的响应结果进行处理

int fat_cmdres_keyword_matching(char *res); //命令响应数据中查找关键字
	 
	 
	 
void Handle_Ec600n(void);
void uart1_send_byte(uint8_t data);
#ifdef __cplusplus
}
#endif
#endif /*__ usart_H */
