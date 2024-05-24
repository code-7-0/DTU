#ifndef __LTE_H
#define __LTE_H

typedef enum
{
    MD_WAIT,        				//��λģ��
	  MD_RESET,        				//��λģ��
    MD_AT_REQ,       				//AT����
	  MD_AT_E0,       				//�رջ���
    MD_WORK_STA_CHK, 				//����״̬���
    MD_CONNETINIT,   				//����������Ϣ��ʼ��
    MD_CONNETED,     				//����ͨ��
		MD_FLIGHTMODE,			   	//����ģʽ
    MD_OK = 0xFE,    				//����
    MD_ERR = 0xFF,   				//�쳣
} MD_RUN_STATE;

//�������̱�ʶ
typedef enum
{
    SUB_MESG_GET = 0,
	  SUB_SEND1,        				//��������
	  SUB_SEND2,        				   //��������
	  SUB_CHECK,        				  //��ѯ����״̬
    SUB_CK_SOCKET,       				//��ѯSocket
    SUB_CK_TOPIC, 					    //����״̬���
	  SUB_WAIT,   								//����������Ϣ��ʼ��
    SUB_CONNETED,     					//����ͨ��
		SUB_FLIGHTMODE,			   			//����ģʽ
    SUB_OK = 0xFE,    					//����
    SUB_ERR = 0xFF,   					//�쳣
} SUB_STATE;


void handle_MQTTConvert(void);//���յ��ƶ������ִ������͸��
void clear_bufer(char *arry,unsigned int dat);
void module_MQTT(void);

#endif
