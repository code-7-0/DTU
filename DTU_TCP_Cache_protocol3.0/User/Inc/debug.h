#ifndef __debug_H
#define __debug_H
#include "serial_transceiver.h"
#include "flash.h"
#include "Gaspower.h"	 
#include "temper_control.h" 
	 
//�豸������Ϣ
//#pragma pack(1)
//struct Device_Manage
//{					
//	  uint8_t  name[20];  		//�豸����
//		uint8_t  factory[20];  	//��������
//	  uint8_t  Model[20];  		//�豸�ͺ�
//	  uint16_t device_code;   //��Ʒ���룬��ʼ����10000����Э���Ӧ
//	  uint16_t Station_NO;    //�豸վ��
//	  uint16_t online;        //�豸վ��
//	  uint16_t baud_rate;     //�豸������ 
//	  uint16_t Parity_bits;   //�豸��żУ��λ
//	  uint16_t run_start;     //����״̬
//};
//#pragma pack()



//extern struct Device_Manage Device_Manage; //�豸��Ϣ

extern void Handle_Debug(void);
extern uint8_t	temper_rapif_reply_flag;
extern uint8_t	temper_rapif_reply_cnt;
#endif 
