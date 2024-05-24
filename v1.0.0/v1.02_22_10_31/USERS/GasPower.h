#include "main.h"
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __Gaspower_H
#define __Gaspower_H
#ifdef __cplusplus
 extern "C" {
#endif

	 
//设备基本信息
#pragma pack(1)
struct GASwall_Manage
{					
	  unsigned char  	tree;  									//引导头
		unsigned char  	length;  								//数据长度
	  unsigned short  index;  								//指令序号
	  unsigned char 	Command_code;   				//指令码
	  unsigned char 	on_off;    							//开关模式   ////////////////
	  unsigned char 	load_state;     				//设备站号
	  unsigned char 	Dev_switch;     				//设备开关状态
		unsigned char 	run_state;      				//运行状态
		unsigned char 	ECO; 										//ECO状态
		unsigned char 	Set_Bathroomtemp;				//卫浴设定温度
		unsigned char 	Sst_Heating_temp;				//采暖设定温度
		unsigned char   Hot_water_flow; 				//热水流量     0.1 L/MIN
		unsigned char 	Bathroomtemp; 					//卫浴温度	
		unsigned char 	Heating_temp;						//采暖温度
		unsigned char 	Outdoor_temp;						//室外温度
		unsigned char 	Indoor_temp;						//室内温度
		unsigned char 	Water_temp;							//自来水水温
		unsigned char 	Hydraulic;							//水压
		unsigned char 	Firepower;      				//火力
		unsigned char 	ERRO_CODE;      				//故障代码    ////////////////
		unsigned char 	Protect_LEVEl;					//保护等级
		unsigned char 	Retain1; 								//预留
		unsigned char 	Retain2;		
		unsigned short 	Check_sum; 							//校验和	
		         char   All_DAT[50];            //数据拼接为字符串 on_off-Protect_LEVEl;
};
#pragma pack()

//壁挂炉控制命令

typedef enum
{
    GAS_SET_RSSI=10,        			//连接状态
	  GAS_SET_STATE=21,        			//状态查询
		GAS_SET_ONOFF,        				//0:关机 1:开机
	  GAS_SET_MODE,									//1:夏季模式2：冬季模式
		GAS_SET_Bathroom_TEMP,				//30-75分别对应35℃-60℃
		GAS_SET_HEATINGTEMP,					//30-80分别对应30℃-xx℃
		GAS_SET_RST,									//1:复位请求
		GAS_SET_HOT_W_R,							//0:关闭1：开启
		GAS_SET_LOCK,									//0:关闭1：开启
		GAS_SET_ECO,									//0:ECO开启1：ECO关闭
	  GAS_SET_NULL,									//
		GAS_SET_TIME_CALIB						//分钟0~59分别对应0~59分钟,小时0~24分别对应0~24小时
} GASPOWER_COMMAND;


//控制命令填值
extern void Gaspowercontrol(unsigned char command,unsigned int dat,unsigned int masg_index);
extern unsigned int  Gaspower_index;      //命令序列号
extern unsigned char Gaspsend_delaycount; //发送计时
extern unsigned char Gasprev_delaycount;  //接收计时
extern unsigned int  send_waittime_cunt;//发送等待延时计数


extern unsigned char EXTI4_15_Single_Wire_Pin_flag; //总线发生中断

extern unsigned long CostofGas;			//燃气用量 0.001 M3
extern unsigned long CostofWater; 	//水量积算 单位0.1L
extern unsigned long CostofElect;  	//电量积算 单位WH
extern unsigned char IntegrationFlag; //积分标识
extern unsigned char IntegrationDelay; //积分倒计时

extern struct GASwall_Manage  GASwall_Manage[2];
void  Handle_Gaspowercontrol(void); //主程序调用

void Handle_GasCost(void);//燃气用量估算

#ifdef __cplusplus
}
#endif
#endif /*__ usart_H */
