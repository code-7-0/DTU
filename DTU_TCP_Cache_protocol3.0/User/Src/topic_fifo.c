#include "topic_fifo.h"
//	温控板控制指令缓存 
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
		if(pos!=fifo_mqtt_topic._tail)               //栈内没有数，也可以存入                                           
    {
				memcpy(&fifo_mqtt_topic.send_fifo[fifo_mqtt_topic._head],&temper_topic_send,sizeof(TemperMqttTopic)); 
        fifo_mqtt_topic._head = pos; //头指针指向已存数据的下一位
    }
}

//void MqttTopicFiFoPush(TemperMqttTopic *temper_topic_send)
//{
//    uint16_t pos = (fifo_mqtt_topic._head+1)%TOPIC_FIFO_MAX_SIZE;
//		if(pos!=fifo_mqtt_topic._tail)               //栈内没有数，也可以存入                                           
//    {
//			memcpy(&fifo_mqtt_topic.send_fifo[fifo_mqtt_topic._head],temper_topic_send,sizeof(TemperMqttTopic)); 
//        fifo_mqtt_topic._head = pos; //头指针指向已存数据的下一位
//    }
//}

void MqttTopicFiFoPop(TemperMqttTopic* temper_topic_send)
{
    if(fifo_mqtt_topic._tail!=fifo_mqtt_topic._head)        //栈内必须有数据才能取出                                  
    {
			memcpy(temper_topic_send,&fifo_mqtt_topic.send_fifo[fifo_mqtt_topic._tail],sizeof(TemperMqttTopic)); 
			fifo_mqtt_topic._tail = (fifo_mqtt_topic._tail+1)%TOPIC_FIFO_MAX_SIZE;//指向在未被取出数据,%FIFO_MAX_SIZE使fifo_mqtt_topic._tail，fifo_mqtt_topic._head都不会超过FIFO_MAX_SIZE
    }     
}

uint16_t MqttTopicFiFoSize(void)
{
    return ((fifo_mqtt_topic._head+TOPIC_FIFO_MAX_SIZE-fifo_mqtt_topic._tail)%TOPIC_FIFO_MAX_SIZE);
}





