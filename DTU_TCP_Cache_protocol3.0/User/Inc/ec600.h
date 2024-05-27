#ifndef __ec600n_H
#define __ec600n_H 
#include "serial_transceiver.h"
#include "global.h"
void fat_uart_send_str(char *buf); 
extern void fat_tim_proc(void);       //fat ��ʱ��������Ҫ�û������Ķ�ʱ���жϴ�����

extern uint8_t wait_timeout(uint16_t time); // ������ʱ�������ﵽ���ö�ʱֵʱ����1	 
extern uint8_t wait_timeout_nonblocking( uint16_t timeoutval); //�ȴ������

extern uint8_t fat_send_wait_cmdres_blocking(char *cmd, uint16_t timeoutval);    //��������󣬿�����ʱ�����������õĶ�ʱʱ�䵽��󷵻�1���û���ʱ���Խ��ж��������Ӧ������д���
extern uint8_t fat_send_wait_cmdres_nonblocking(char *cmd, uint16_t timeoutval); //��������󣬿�����ʱ�����������õĶ�ʱʱ�䵽������������Ӧ����ʱ����1���û���ʱ���Խ��ж��������Ӧ������д���

extern uint8_t fat_cmdres_keyword_matching(char *res); //������Ӧ�����в��ҹؼ���
	   

void Handle_Ec600n(void);
#endif  
