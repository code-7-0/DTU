#ifndef __MQTTTOPIC_H
#define __MQTTTOPIC_H

#define		TOPIC_DAT_NUM     40  //

//故障代码
#define ERROR_TEMP1 	0x11
#define ERROR_TEMP2 	0x12
#define ERROR_HIGH 		0x13
#define ERROR_FAST 		0x14
#define ERROR_CRT 		0x15
#define ERROR_FIRE 		0x16
#define ERROR_FAN 		0x17
#define ERROR_PUMP 		0x18
#define ERROR_BURN 		0x19
#define ERROR_GAS 		0x1A 
#define ERROR_PRESS 	0x1B 
#define ERROR_HIGH2 	0x1C
#define ERROR_FREZ 		0x1D
#define ERROR_RUN 		0x1E
#define ERROR_HEAT 		0x1F
#define ERROR_OUT 		0x20
#define ERROR_FAN2 		0x21
#define ERROR_WARM 		0x22
#define ERROR_UART 		0x23
#define ERROR_TEMP 		0x24
#define ERROR_WASH 		0x25 
#define ERROR_GAS2 		0x26 
#define ERROR_WATER 	0x27


#define MQTT_DAT_UP_DELAY 	300    //主动上报周期通常为300秒
#define MQTT_DAT_UP_DELAY_LIVE 	6    //主动上报周期通常为10秒(活跃)


//topic data基本信息
#pragma pack(1)
struct Topic_dat
{					
    unsigned char enable;     //是否发送该命令
	  unsigned int send_flag;  	//待发送标志
	  unsigned int send_cont_cycl;  //发送周期，单位秒
	  unsigned int send_count;  //距离上次发送时间
	   char identifier[25];  		//参数标识符
	   char dat[50];         		//数据
};
#pragma pack()


//				0,0,1000,1,"E1","0",			//20点火失败故障
//				0,0,1000,1,"E2","0",			//21残焰、假火
//				0,0,1000,1,"E3","0",			//22限温器故障(机械限温器断开)
//				0,0,1000,1,"E4","0",			//23火焰4分钟内熄火5次（着火后火焰检测有效后又熄火）
//				0,0,1000,1,"E6","0",			//24温度不上升或传感器脱落故障(点火燃烧持续30s后,采暖传感器和卫浴水传感器温度变化不超过2℃)
//				0,0,1000,1,"E7","0",			//25防冻故障≤3℃
//				0,0,1000,1,"F1","0",			//26风压故障
//				0,0,1000,1,"F2","0",			//27缺水或水压开关故障（水泵启动前检测，检测失败不启动水泵，连续5s检测失败报警。）
//        0,0,1000,1,"F3","0",			//28采暖温度传感器故障（采暖NTC开路、短路或连线松脱、短路）
//				0,0,1000,1,"F4","0",			//29卫浴水温度传感器故障（卫浴水NTC开路、短路或连线松脱、短路)
//				0,0,1000,1,"F5","0",			//30温度传感器超温故障


////topic data基本信息
//#pragma pack(1)
//struct Topic_setting
//{					
//	   char identifier[25];  //参数标识符
//};
//#pragma pack()


void Handle_topic(void);
unsigned char  topic_check_list(void);
void Trigger_MQ_T(unsigned char state); //触发一次发送，3秒后
void Handle_MQTT_live(void);

extern unsigned char topic_state;			//待发送命令直针
extern unsigned char topic_state_last; //已发送命令直针

extern char topic_buf1[256];
extern char topic_buf2[256];		
extern unsigned int  topic_count;			  //命令序号
extern unsigned char topic_buf_ok;			  //拼装命令标识



#endif

