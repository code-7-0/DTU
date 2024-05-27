#ifndef __TOPIC_FIFO_H
#define __TOPIC_FIFO_H
#include "main.h"
#include "string.h"

#define TOPIC_FIFO_MAX_SIZE		5 
enum{
		NETIN=0,//��ʼ����
		//NETOUT,//ֹͣ����
		ADD,//������豸
		DELETE,//ɾ��
		STATUS,//����״̬
		//CONTROL,//����
		ONLINE,//����״̬
		IDLIST//���豸�б�
		//UPDATE //OTA����
};

//�������
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

