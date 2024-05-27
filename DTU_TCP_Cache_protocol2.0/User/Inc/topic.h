#ifndef __TOPIC_H
#define __TOPIC_H
#include "Gaspower.h"
#include "flash.h"
//#include "lte_demo.h" 
 
#define MQTT_DAT_UPLOAD_DELAY 	10    //主动上报周期通常为5秒
#define MQTT_DAT_UPLOAD_DELAY_LIVE 	4    //主动上报周期通常为2秒(活跃)


//topic data基本信息
#pragma pack(1)
struct Topic_dat 
{			 
//	  uint8_t send_flag;  			//待发送标志
	  uint16_t send_count_cycl;  //发送周期，单位秒 
		uint16_t send_count;  		//发送计时
		char identifier[10];  		//参数标识符
//		char dat[50];         		//数据
};
//topic data基本信息
#pragma pack(1)
struct Topic_err
{			 
//	  uint8_t send_data_flag;  	//待发送标志 
		char identifier[3];  		//参数标识符
//		uint8_t dat;
		char dat[2];         		//数据
};
#pragma pack()


#define ERROR_TIMES 4
typedef struct {
//    uint8_t erro_code;  // 故障代码
		unsigned char erro_code;
    unsigned char count;      // 故障计数
		unsigned char start_flag;  // 开始计数标志
    unsigned char refresh_flag;  // 发送标志
} FaultInfo;

void Handle_topic(void);

extern uint8_t topic_send_wait;			//待发送命令直针
extern uint8_t topic_send_last; //已发送命令直针

extern char 	message_buf[256];
extern char 	topic_buf[256];		
extern uint32_t  topic_count;			  //命令序号
extern uint8_t topic_send_ok;			  //拼装命令标识
extern uint8_t topic_update_flag ;
extern char   QCCID[21]; 

#endif

