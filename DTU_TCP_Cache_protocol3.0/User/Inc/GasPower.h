#ifndef __Gaspower_H
#define __Gaspower_H 
#include "tim.h"
//#include "debug.h"
#include "gpio.h" 
#include "gas_fifo.h"
#include "global.h"
#include "stdio.h"
#include "serial_transceiver.h"
//��״̬-���͡����ա��ȴ�������
typedef enum{
    BUS_NULL=0,        //����
	  BUS_REV,           //����æ
		BUS_SEND,        	//����æ
  	BUS_WAIT          //�ȴ�
}   BUS_STATES;

//���������ֽ�״̬-������+11���ֽڣ�������ΪУ���룩+������
typedef enum{
	  BYTE_SEND_STATE_0 = 0,   //������
		BYTE_SEND_STATE_1,
		BYTE_SEND_STATE_2,
		BYTE_SEND_STATE_3,
		BYTE_SEND_STATE_4,
		BYTE_SEND_STATE_5,
		BYTE_SEND_STATE_6,
		BYTE_SEND_STATE_7,
		BYTE_SEND_STATE_8,
		BYTE_SEND_STATE_9,
		BYTE_SEND_STATE_10,
	  BYTE_SEND_STATE_11,
    BYTE_SEND_STATE_OFF = 0XFE,
} BYTE_STATES;


//�ڹ�¯��������
typedef enum
{
    GAS_SET_RSSI=10,        			//����״̬
	  GAS_SET_STATE=21,        			//״̬��ѯ
		GAS_SET_ONOFF,        				//0:�ػ� 1:����
	  GAS_SET_MODE,									//1:�ļ�ģʽ2������ģʽ
		GAS_SET_BATHROOM_TEMP,				//30-75�ֱ��Ӧ35��-60��
		GAS_SET_HEATINGTEMP,					//30-80�ֱ��Ӧ30��-xx��
		GAS_SET_RST,									//1:��λ����
		GAS_SET_HOT_WATER_CIRC,							//0:�ر�1������
		GAS_SET_LOCK,									//0:�ر�1������
		GAS_SET_ECO,									//0:ECO����1��ECO�ر�
	  GAS_SET_NULL,									//
		GAS_SET_TIME_CALIB						//����0~59�ֱ��Ӧ0~59����,Сʱ0~24�ֱ��Ӧ0~24Сʱ
} GASPOWER_COMMAND;


//�豸������Ϣ ���ո�ʽ
#pragma pack(1)
typedef struct 
{					
	  uint8_t  	tree;  									//����ͷ
		uint8_t  	length;  								//���ݳ���
	  uint16_t  index;  								//ָ�����
	  uint8_t 	Command_code;   				//ָ����
	  uint8_t 	on_off;    							//����ģʽ   ////////////////
	  uint8_t 	load_state;     				//�豸վ��
	  uint8_t 	Dev_switch;     				//�豸����״̬
		uint8_t 	run_state;      				//����״̬
		uint8_t 	ECO; 										//ECO״̬
		uint8_t 	Set_Bathroomtemp;				//��ԡ�趨�¶�
		uint8_t 	Set_Heating_temp;				//��ů�趨�¶�
		uint8_t   Hot_water_flow; 				//��ˮ����     0.1 L/MIN
		uint8_t 	Bathroomtemp; 					//��ԡ�¶�	
		uint8_t 	Heating_temp;						//��ů�¶�
		uint8_t 	Outdoor_temp;						//�����¶�
		uint8_t 	Indoor_temp;						//�����¶�
		uint8_t 	Water_temp;							//����ˮˮ��
		uint8_t 	Hydraulic;							//ˮѹ
		uint8_t 	Firepower;      				//����
		uint8_t 	ERRO_CODE;      				//���ϴ���    ////////////////
		uint8_t 	Protect_LEVEl;					//�����ȼ�
		uint8_t 	Retain1; 								//Ԥ��
		uint8_t 	Retain2;		
		uint16_t 	Check_sum; 							//У���	
		char ALL_DATA[50];
}S_GasPower_Manage;
#pragma pack()

//char   Gaspower_Ascii[50];            //����ƴ��Ϊ�ַ��� on_off-Protect_LEVEl;


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

extern uint16_t  Gaspower_index;      //�������к�
extern uint8_t   siglewire_wait_cnt;//���͵ȴ���ʱ����
extern S_GasPower_Manage  GasPower_Manage;
//extern char   Gaspower_Ascii[50]; 
extern uint8_t 	error_code_pre; //֮ǰ�Ĵ������


extern uint64_t CostofGas;			//ȼ������ 0.001 M3
extern uint64_t CostofWater; 	//ˮ������ ��λ0.1L
extern uint64_t CostofElect;  	//�������� ��λWH
extern uint8_t IntegrationFlag; //���ֱ�ʶ
extern uint8_t IntegrationDelay; //���ֵ���ʱ


//����������ֵ
extern void Gaspower_Command(uint8_t command,uint16_t dat,uint16_t masg_index);
extern void  Handle_Gaspower (void); //���������
extern void Handle_GasCost(void);//ȼ����������
extern void GasPower_Rapid_Reply(void);
#endif /*__ usart_H */
