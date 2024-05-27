#ifndef __GLOBAL_H
#define __GLOBAL_H
//#include <stdlib.h>
//#include <stdlib.h>
//#include "stdio.h"
//#include "string.h"
#include "main.h"

//extern uint16_t convertStringsToUint16(const char* strArray);
extern uint16_t stringToUint16(const char* str) ;
//������Ϣ����
#pragma pack(1)
struct s_debug_onoff
{					
	  uint8_t  Gaspower;  		//ȼ���ڹ�¯������Ϣ��ӡ
	  uint8_t  EC600N;  			//EC600N������Ϣ��ӡ
		uint8_t  GasCost;  			//ȼ��������Ϣ��ӡ
		uint8_t  MQ_send;  			//������Ϣ��ӡ
};

		





//struct s_debug_onoff
//{					
//	  uint8_t  Gaspower;  		
//	  uint8_t  EC600N;  			
//		uint8_t  GasCost;  			
//		uint8_t  MQ_send;  			
//};
#pragma pack()
extern struct s_debug_onoff debug_onoff;     //������Ϣ����

// ����汾
#define VER_MAJOR				1
#define VER_MINOR				0//0 
#define VER_REVISION		0//4//3//2//1//0//4//3//2//1//0


//---------- �꺯������ -----------------------------------------
#define INC_LIMIT(x,max)	if(x < max) x++
#define DEC_LIMIT(x,min) 	if(x > min) x--

#define UP_LIMIT(x,max)		if(x > max) x = max
#define DN_LIMIT(x,min)		if(x < min) x = min

// �ָߵ��ֽڽ���
#define WORD_SWAP(a) 		a = ((a<<8)&0xFF00) | ((a>>8)&0x00FF)
// ��ĸСдת��д
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


