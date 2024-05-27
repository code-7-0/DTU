#ifndef __Gaspower_H
#define __Gaspower_H 
#include "tim.h"
//#include "debug.h"
#include "gpio.h" 
#include "gas_fifo.h"
#include "global.h"
#include "stdio.h"
#include "serial_transceiver.h"
//总状态-发送、接收、等待、空闲
typedef enum{
    BUS_NULL=0,        //空闲
	  BUS_REV,           //接收忙
		BUS_SEND,        	//发送忙
  	BUS_WAIT          //等待
}   BUS_STATES;

//发送命令字节状态-引导码+11个字节（后两个为校验码）+结束码
typedef enum{
	  BYTE_SEND_STATE_0 = 0,   //引导码
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


//壁挂炉控制命令
typedef enum
{
    GAS_SET_RSSI=10,        			//连接状态
	  GAS_SET_STATE=21,        			//状态查询
		GAS_SET_ONOFF,        				//0:关机 1:开机
	  GAS_SET_MODE,									//1:夏季模式2：冬季模式
		GAS_SET_BATHROOM_TEMP,				//30-75分别对应35℃-60℃
		GAS_SET_HEATINGTEMP,					//30-80分别对应30℃-xx℃
		GAS_SET_RST,									//1:复位请求
		GAS_SET_HOT_WATER_CIRC,							//0:关闭1：开启
		GAS_SET_LOCK,									//0:关闭1：开启
		GAS_SET_ECO,									//0:ECO开启1：ECO关闭
	  GAS_SET_NULL,									//
		GAS_SET_TIME_CALIB						//分钟0~59分别对应0~59分钟,小时0~24分别对应0~24小时
} GASPOWER_COMMAND;


//设备基本信息 接收格式
#pragma pack(1)
typedef struct 
{					
	  uint8_t  	tree;  									//引导头
		uint8_t  	length;  								//数据长度
	  uint16_t  index;  								//指令序号
	  uint8_t 	Command_code;   				//指令码
	  uint8_t 	on_off;    							//开关模式   ////////////////
	  uint8_t 	load_state;     				//设备站号
	  uint8_t 	Dev_switch;     				//设备开关状态
		uint8_t 	run_state;      				//运行状态
		uint8_t 	ECO; 										//ECO状态
		uint8_t 	Set_Bathroomtemp;				//卫浴设定温度
		uint8_t 	Set_Heating_temp;				//采暖设定温度
		uint8_t   Hot_water_flow; 				//热水流量     0.1 L/MIN
		uint8_t 	Bathroomtemp; 					//卫浴温度	
		uint8_t 	Heating_temp;						//采暖温度
		uint8_t 	Outdoor_temp;						//室外温度
		uint8_t 	Indoor_temp;						//室内温度
		uint8_t 	Water_temp;							//自来水水温
		uint8_t 	Hydraulic;							//水压
		uint8_t 	Firepower;      				//火力
		uint8_t 	ERRO_CODE;      				//故障代码    ////////////////
		uint8_t 	Protect_LEVEl;					//保护等级
		uint8_t 	Retain1; 								//预留
		uint8_t 	Retain2;		
		uint16_t 	Check_sum; 							//校验和	
		char ALL_DATA[50];
}S_GasPower_Manage;
#pragma pack()

//char   Gaspower_Ascii[50];            //数据拼接为字符串 on_off-Protect_LEVEl;


//故障代码
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

extern uint16_t  Gaspower_index;      //命令序列号
extern uint8_t   siglewire_wait_cnt;//发送等待延时计数
extern S_GasPower_Manage  GasPower_Manage;
//extern char   Gaspower_Ascii[50]; 
extern uint8_t 	error_code_pre; //之前的错误代码


extern uint64_t CostofGas;			//燃气用量 0.001 M3
extern uint64_t CostofWater; 	//水量积算 单位0.1L
extern uint64_t CostofElect;  	//电量积算 单位WH
extern uint8_t IntegrationFlag; //积分标识
extern uint8_t IntegrationDelay; //积分倒计时


//控制命令填值
extern void Gaspower_Command(uint8_t command,uint16_t dat,uint16_t masg_index);
extern void  Handle_Gaspower (void); //主程序调用
extern void Handle_GasCost(void);//燃气用量估算
extern void GasPower_Rapid_Reply(void);
#endif /*__ usart_H */
