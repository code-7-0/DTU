#ifndef __TOPIC_H
#define __TOPIC_H
#include "Gaspower.h"
#include "flash.h"
//#include "lte_demo.h" 
 
#define MQTT_DAT_UPLOAD_DELAY 	10    //�����ϱ�����ͨ��Ϊ5��
#define MQTT_DAT_UPLOAD_DELAY_LIVE 	4    //�����ϱ�����ͨ��Ϊ2��(��Ծ)


//topic data������Ϣ
#pragma pack(1)
struct Topic_dat 
{			 
//	  uint8_t send_flag;  			//�����ͱ�־
	  uint16_t send_count_cycl;  //�������ڣ���λ�� 
		uint16_t send_count;  		//���ͼ�ʱ
		char identifier[10];  		//������ʶ��
//		char dat[50];         		//����
};
//topic data������Ϣ
#pragma pack(1)
struct Topic_err
{			 
//	  uint8_t send_data_flag;  	//�����ͱ�־ 
		char identifier[3];  		//������ʶ��
//		uint8_t dat;
		char dat[2];         		//����
};
#pragma pack()


#define ERROR_TIMES 4
typedef struct {
//    uint8_t erro_code;  // ���ϴ���
		unsigned char erro_code;
    unsigned char count;      // ���ϼ���
		unsigned char start_flag;  // ��ʼ������־
    unsigned char refresh_flag;  // ���ͱ�־
} FaultInfo;

void Handle_topic(void);

extern uint8_t topic_send_wait;			//����������ֱ��
extern uint8_t topic_send_last; //�ѷ�������ֱ��

extern char 	message_buf[256];
extern char 	topic_buf[256];		
extern uint32_t  topic_count;			  //�������
extern uint8_t topic_send_ok;			  //ƴװ�����ʶ
extern uint8_t topic_update_flag ;
extern char   QCCID[21]; 

#endif

