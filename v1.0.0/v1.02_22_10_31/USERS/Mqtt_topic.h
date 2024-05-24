#ifndef __MQTTTOPIC_H
#define __MQTTTOPIC_H

#define		TOPIC_DAT_NUM     40  //

//���ϴ���
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


#define MQTT_DAT_UP_DELAY 	300    //�����ϱ�����ͨ��Ϊ300��
#define MQTT_DAT_UP_DELAY_LIVE 	6    //�����ϱ�����ͨ��Ϊ10��(��Ծ)


//topic data������Ϣ
#pragma pack(1)
struct Topic_dat
{					
    unsigned char enable;     //�Ƿ��͸�����
	  unsigned int send_flag;  	//�����ͱ�־
	  unsigned int send_cont_cycl;  //�������ڣ���λ��
	  unsigned int send_count;  //�����ϴη���ʱ��
	   char identifier[25];  		//������ʶ��
	   char dat[50];         		//����
};
#pragma pack()


//				0,0,1000,1,"E1","0",			//20���ʧ�ܹ���
//				0,0,1000,1,"E2","0",			//21���桢�ٻ�
//				0,0,1000,1,"E3","0",			//22����������(��е�������Ͽ�)
//				0,0,1000,1,"E4","0",			//23����4������Ϩ��5�Σ��Ż���������Ч����Ϩ��
//				0,0,1000,1,"E6","0",			//24�¶Ȳ������򴫸����������(���ȼ�ճ���30s��,��ů����������ԡˮ�������¶ȱ仯������2��)
//				0,0,1000,1,"E7","0",			//25�������ϡ�3��
//				0,0,1000,1,"F1","0",			//26��ѹ����
//				0,0,1000,1,"F2","0",			//27ȱˮ��ˮѹ���ع��ϣ�ˮ������ǰ��⣬���ʧ�ܲ�����ˮ�ã�����5s���ʧ�ܱ�������
//        0,0,1000,1,"F3","0",			//28��ů�¶ȴ��������ϣ���ůNTC��·����·���������ѡ���·��
//				0,0,1000,1,"F4","0",			//29��ԡˮ�¶ȴ��������ϣ���ԡˮNTC��·����·���������ѡ���·)
//				0,0,1000,1,"F5","0",			//30�¶ȴ��������¹���


////topic data������Ϣ
//#pragma pack(1)
//struct Topic_setting
//{					
//	   char identifier[25];  //������ʶ��
//};
//#pragma pack()


void Handle_topic(void);
unsigned char  topic_check_list(void);
void Trigger_MQ_T(unsigned char state); //����һ�η��ͣ�3���
void Handle_MQTT_live(void);

extern unsigned char topic_state;			//����������ֱ��
extern unsigned char topic_state_last; //�ѷ�������ֱ��

extern char topic_buf1[256];
extern char topic_buf2[256];		
extern unsigned int  topic_count;			  //�������
extern unsigned char topic_buf_ok;			  //ƴװ�����ʶ



#endif

