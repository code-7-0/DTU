#include "main.h"
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __debug_H
#define __debug_H
#ifdef __cplusplus
 extern "C" {
#endif

//��������ַ��
#define  Dev_LOCK     	10001   //��������
#define  Dev_TEMP1   		10002   //�����ʿ���ʪ�ȴ�����RS-WS-N01-8
#define  Dev_TEMP2   		10003   //
#define  Dev_BMS    	  10004   //BM1650_BMS
#define  Dev_HCAP    	  10005   //4ͨ�������ɼ��棬ɣ��
#define  Dev_D081FD    	10006   //������

//USART3�жϽ��մ���
extern unsigned int   usart3delay;           //ͨ�ų�ʱʱ�䣬�������5ms,Ϊͨ�ų�ʱ
extern unsigned char  FLAG_uart3;			      //UART2��ʼ�µ�һ֡���ݽ���ǰ��ʹ�ܽ��������־�����ܹ����б���Ϊ1���ڶ�ʱ���������ճ�ʱʱ������
extern unsigned char  Debug_Shandle;	//����֡�����־
	 
	 
//�豸������Ϣ
#pragma pack(1)
struct Device_Manage
{					
	  unsigned char  name[20];  		//�豸����
		unsigned char  factory[20];  	//��������
	  unsigned char  Model[20];  		//�豸�ͺ�
	  unsigned short device_code;   //��Ʒ���룬��ʼ����10000����Э���Ӧ
	  unsigned short Station_NO;    //�豸վ��
	  unsigned short online;        //�豸վ��
	  unsigned short baud_rate;     //�豸������ 
	  unsigned short Parity_bits;   //�豸��żУ��λ
	  unsigned short run_start;     //����״̬
};
#pragma pack()

//������Ϣ����
#pragma pack(1)
struct debug_onoff
{					
	  unsigned char  Gaspower;  		//ȼ���ڹ�¯������Ϣ��ӡ
	  unsigned char  EC600N;  			//EC600N������Ϣ��ӡ
		unsigned char  GasCost;  			//ȼ��������Ϣ��ӡ
		unsigned char  MQ_send;  			//������Ϣ��ӡ
};
#pragma pack()

extern struct Device_Manage Device_Manage; //�豸��Ϣ
extern struct debug_onoff debug_onoff;     //������Ϣ����

void printfArry(unsigned char *arry,unsigned char num); //��ӡ����

extern	void Handle_Debug(void);
#ifdef __cplusplus
}
#endif
#endif /*__ usart_H */
