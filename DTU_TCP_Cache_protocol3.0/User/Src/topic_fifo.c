#include "topic_fifo.h"
//	�¿ذ����ָ��� 
FifoTopic fifo_mqtt_topic = {0,0,0};    

void MqttTopicFiFoReset(void)
{
    fifo_mqtt_topic._head = fifo_mqtt_topic._tail = 0;
}

void MqttTopicFiFoPush(uint8_t command,uint8_t* id)
{
		TemperMqttTopic temper_topic_send={0,0};
		temper_topic_send.command=command;
		if(id!=NULL)
			memcpy(temper_topic_send.sub_id,id,4);
		
    uint16_t pos = (fifo_mqtt_topic._head+1)%TOPIC_FIFO_MAX_SIZE;
		if(pos!=fifo_mqtt_topic._tail)               //ջ��û������Ҳ���Դ���                                           
    {
				memcpy(&fifo_mqtt_topic.send_fifo[fifo_mqtt_topic._head],&temper_topic_send,sizeof(TemperMqttTopic)); 
        fifo_mqtt_topic._head = pos; //ͷָ��ָ���Ѵ����ݵ���һλ
    }
}

//void MqttTopicFiFoPush(TemperMqttTopic *temper_topic_send)
//{
//    uint16_t pos = (fifo_mqtt_topic._head+1)%TOPIC_FIFO_MAX_SIZE;
//		if(pos!=fifo_mqtt_topic._tail)               //ջ��û������Ҳ���Դ���                                           
//    {
//			memcpy(&fifo_mqtt_topic.send_fifo[fifo_mqtt_topic._head],temper_topic_send,sizeof(TemperMqttTopic)); 
//        fifo_mqtt_topic._head = pos; //ͷָ��ָ���Ѵ����ݵ���һλ
//    }
//}

void MqttTopicFiFoPop(TemperMqttTopic* temper_topic_send)
{
    if(fifo_mqtt_topic._tail!=fifo_mqtt_topic._head)        //ջ�ڱ��������ݲ���ȡ��                                  
    {
			memcpy(temper_topic_send,&fifo_mqtt_topic.send_fifo[fifo_mqtt_topic._tail],sizeof(TemperMqttTopic)); 
			fifo_mqtt_topic._tail = (fifo_mqtt_topic._tail+1)%TOPIC_FIFO_MAX_SIZE;//ָ����δ��ȡ������,%FIFO_MAX_SIZEʹfifo_mqtt_topic._tail��fifo_mqtt_topic._head�����ᳬ��FIFO_MAX_SIZE
    }     
}

uint16_t MqttTopicFiFoSize(void)
{
    return ((fifo_mqtt_topic._head+TOPIC_FIFO_MAX_SIZE-fifo_mqtt_topic._tail)%TOPIC_FIFO_MAX_SIZE);
}





