
#include "main.h"
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ec600n_H
#define __ec600n_H
#ifdef __cplusplus
 extern "C" {
#endif
#define FAT_UART_RX_SIZE 512    //fat ���ڽ��ջ����С	 
#define FAT_TIMER_VAL  1     //fat ��ʱ�����   

//USART1�жϽ��մ���
extern unsigned int   usart1delay;           //ͨ�ų�ʱʱ�䣬�������5ms,Ϊͨ�ų�ʱ
extern unsigned char  FLAG_uart1;			      //UART2��ʼ�µ�һ֡���ݽ���ǰ��ʹ�ܽ��������־�����ܹ����б���Ϊ1���ڶ�ʱ���������ճ�ʱʱ������
extern unsigned char  Ec600_Shandle;	//����֡�����־
	 

extern char USART1_RX_BUF[FAT_UART_RX_SIZE];
void fat_tim_proc(void);                                                 //fat ��ʱ��������Ҫ�û������Ķ�ʱ���жϴ�����
	 
void reg_fat_uart_send_byte(void (*uart_send_byte)(unsigned char data)); //ע�� ���ڷ���һ���ֽں���
	 
int wait_timeout(unsigned short int time); // ������ʱ�������ﵽ���ö�ʱֵʱ����1	 
	 
int fat_send_wait_cmdres_blocking(char *cmd, unsigned short int timeoutval);    //��������󣬿�����ʱ�����������õĶ�ʱʱ�䵽��󷵻�1���û���ʱ���Խ��ж��������Ӧ������д���
int fat_send_wait_cmdres_nonblocking(char *cmd, unsigned short int timeoutval); //��������󣬿�����ʱ�����������õĶ�ʱʱ�䵽������������Ӧ����ʱ����1���û���ʱ���Խ��ж��������Ӧ������д���

int fat_cmdres_keyword_matching(char *res); //������Ӧ�����в��ҹؼ���
	 
	 
	 
void Handle_Ec600n(void);
void uart1_send_byte(uint8_t data);
#ifdef __cplusplus
}
#endif
#endif /*__ usart_H */
