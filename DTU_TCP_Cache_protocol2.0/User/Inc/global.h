#ifndef __GLOBAL_H
#define __GLOBAL_H
//#include <stdlib.h>
//#include <stdlib.h>
//#include "stdio.h"
//#include "string.h"
#include "main.h"

//extern uint16_t convertStringsToUint16(const char* strArray);
extern uint16_t stringToUint16(const char* str) ;
//调试信息开关
#pragma pack(1)
struct s_debug_onoff
{					
	  uint8_t  Gaspower;  		//燃气壁挂炉调试信息打印
	  uint8_t  EC600N;  			//EC600N调试信息打印
		uint8_t  GasCost;  			//燃气积算信息打印
		uint8_t  MQ_send;  			//发布信息打印
};

		





//struct s_debug_onoff
//{					
//	  uint8_t  Gaspower;  		
//	  uint8_t  EC600N;  			
//		uint8_t  GasCost;  			
//		uint8_t  MQ_send;  			
//};
#pragma pack()
extern struct s_debug_onoff debug_onoff;     //调试信息开关

// 软件版本
#define VER_MAJOR				1
#define VER_MINOR				0//0 
#define VER_REVISION		0//4//3//2//1//0//4//3//2//1//0


//---------- 宏函数定义 -----------------------------------------
#define INC_LIMIT(x,max)	if(x < max) x++
#define DEC_LIMIT(x,min) 	if(x > min) x--

#define UP_LIMIT(x,max)		if(x > max) x = max
#define DN_LIMIT(x,min)		if(x < min) x = min

// 字高低字节交换
#define WORD_SWAP(a) 		a = ((a<<8)&0xFF00) | ((a>>8)&0x00FF)
// 字母小写转大写
#define char_tolower(c)     (char) ((c >= 'a' && c <= 'z') ? (c & ~0x20) : c)  

typedef enum {FALSE = 0, TRUE = !FALSE} bool;


extern uint16_t CRC16(uint8_t *ptr,  uint16_t len);
extern uint64_t timer_cnt;
extern uint32_t outlinetimes;
extern uint8_t update_flag;
extern uint8_t tcp_connect_cnt;
extern uint32_t OTA_store_addr;
extern uint16_t zero_value;
extern uint16_t zero_cnt; 
extern uint8_t qccid_get_flag;
extern uint8_t zero_buf[255];
#endif


