#ifndef __ec600n_H
#define __ec600n_H 
#include "serial_transceiver.h"
#include "global.h"
void fat_uart_send_str(char *buf); 
extern void fat_tim_proc(void);       //fat 定时器处理，需要用户开启的定时器中断处理中

extern uint8_t wait_timeout(uint16_t time); // 开启定时计数，达到设置定时值时返回1	 
extern uint8_t wait_timeout_nonblocking( uint16_t timeoutval); //等待命令到达

extern uint8_t fat_send_wait_cmdres_blocking(char *cmd, uint16_t timeoutval);    //发送命令后，开启定时计数，在设置的定时时间到达后返回1，用户此时可以进行对命令的响应结果进行处理
extern uint8_t fat_send_wait_cmdres_nonblocking(char *cmd, uint16_t timeoutval); //发送命令后，开启定时计数，在设置的定时时间到达后或命令有响应数据时返回1，用户此时可以进行对命令的响应结果进行处理

extern uint8_t fat_cmdres_keyword_matching(char *res); //命令响应数据中查找关键字
	   

void Handle_Ec600n(void);
#endif  
