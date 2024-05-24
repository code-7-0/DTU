#include "main.h"
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __Gaspower_H
#define __Gaspower_H
#ifdef __cplusplus
 extern "C" {
#endif

	 
//�豸������Ϣ
#pragma pack(1)
struct GASwall_Manage
{					
	  unsigned char  	tree;  									//����ͷ
		unsigned char  	length;  								//���ݳ���
	  unsigned short  index;  								//ָ�����
	  unsigned char 	Command_code;   				//ָ����
	  unsigned char 	on_off;    							//����ģʽ   ////////////////
	  unsigned char 	load_state;     				//�豸վ��
	  unsigned char 	Dev_switch;     				//�豸����״̬
		unsigned char 	run_state;      				//����״̬
		unsigned char 	ECO; 										//ECO״̬
		unsigned char 	Set_Bathroomtemp;				//��ԡ�趨�¶�
		unsigned char 	Sst_Heating_temp;				//��ů�趨�¶�
		unsigned char   Hot_water_flow; 				//��ˮ����     0.1 L/MIN
		unsigned char 	Bathroomtemp; 					//��ԡ�¶�	
		unsigned char 	Heating_temp;						//��ů�¶�
		unsigned char 	Outdoor_temp;						//�����¶�
		unsigned char 	Indoor_temp;						//�����¶�
		unsigned char 	Water_temp;							//����ˮˮ��
		unsigned char 	Hydraulic;							//ˮѹ
		unsigned char 	Firepower;      				//����
		unsigned char 	ERRO_CODE;      				//���ϴ���    ////////////////
		unsigned char 	Protect_LEVEl;					//�����ȼ�
		unsigned char 	Retain1; 								//Ԥ��
		unsigned char 	Retain2;		
		unsigned short 	Check_sum; 							//У���	
		         char   All_DAT[50];            //����ƴ��Ϊ�ַ��� on_off-Protect_LEVEl;
};
#pragma pack()

//�ڹ�¯��������

typedef enum
{
    GAS_SET_RSSI=10,        			//����״̬
	  GAS_SET_STATE=21,        			//״̬��ѯ
		GAS_SET_ONOFF,        				//0:�ػ� 1:����
	  GAS_SET_MODE,									//1:�ļ�ģʽ2������ģʽ
		GAS_SET_Bathroom_TEMP,				//30-75�ֱ��Ӧ35��-60��
		GAS_SET_HEATINGTEMP,					//30-80�ֱ��Ӧ30��-xx��
		GAS_SET_RST,									//1:��λ����
		GAS_SET_HOT_W_R,							//0:�ر�1������
		GAS_SET_LOCK,									//0:�ر�1������
		GAS_SET_ECO,									//0:ECO����1��ECO�ر�
	  GAS_SET_NULL,									//
		GAS_SET_TIME_CALIB						//����0~59�ֱ��Ӧ0~59����,Сʱ0~24�ֱ��Ӧ0~24Сʱ
} GASPOWER_COMMAND;


//����������ֵ
extern void Gaspowercontrol(unsigned char command,unsigned int dat,unsigned int masg_index);
extern unsigned int  Gaspower_index;      //�������к�
extern unsigned char Gaspsend_delaycount; //���ͼ�ʱ
extern unsigned char Gasprev_delaycount;  //���ռ�ʱ
extern unsigned int  send_waittime_cunt;//���͵ȴ���ʱ����


extern unsigned char EXTI4_15_Single_Wire_Pin_flag; //���߷����ж�

extern unsigned long CostofGas;			//ȼ������ 0.001 M3
extern unsigned long CostofWater; 	//ˮ������ ��λ0.1L
extern unsigned long CostofElect;  	//�������� ��λWH
extern unsigned char IntegrationFlag; //���ֱ�ʶ
extern unsigned char IntegrationDelay; //���ֵ���ʱ

extern struct GASwall_Manage  GASwall_Manage[2];
void  Handle_Gaspowercontrol(void); //���������

void Handle_GasCost(void);//ȼ����������

#ifdef __cplusplus
}
#endif
#endif /*__ usart_H */
