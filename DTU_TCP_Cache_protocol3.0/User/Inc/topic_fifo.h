#ifndef __TOPIC_FIFO_H
#define __TOPIC_FIFO_H
#include "main.h"
#include "string.h"

#define TOPIC_FIFO_MAX_SIZE		5 
enum{
		NETIN=0,//开始入网
		//NETOUT,//停止入网
		ADD,//添加子设备
		DELETE,//删除
		STATUS,//网络状态
		//CONTROL,//设置
		ONLINE,//网络状态
		IDLIST//子设备列表
		//UPDATE //OTA升级
};

//命令参数
typedef struct{                                                                 
    uint8_t command;                                                         
    uint8_t sub_id[4];                                       
}TemperMqttTopic; 
 
typedef struct{                                                                 
    uint16_t _head;                                                       
    uint16_t _tail;                                                     
    TemperMqttTopic send_fifo[TOPIC_FIFO_MAX_SIZE];                                       
}FifoTopic; 

 

extern void MqttTopicFiFoPush(uint8_t command,uint8_t* id);
extern void MqttTopicFiFoPop(TemperMqttTopic* temper_topic_send);
extern uint16_t MqttTopicFiFoSize(void); 
extern FifoTopic fifo_mqtt_topic;   
 
#endif  

