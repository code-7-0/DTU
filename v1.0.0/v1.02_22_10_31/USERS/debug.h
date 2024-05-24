#include "main.h"
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __debug_H
#define __debug_H
#ifdef __cplusplus
 extern "C" {
#endif

//传感器地址表
#define  Dev_LOCK     	10001   //机柜门锁
#define  Dev_TEMP1   		10002   //建大仁科温湿度传感器RS-WS-N01-8
#define  Dev_TEMP2   		10003   //
#define  Dev_BMS    	  10004   //BM1650_BMS
#define  Dev_HCAP    	  10005   //4通道电流采集版，桑达
#define  Dev_D081FD    	10006   //流量计

//USART3中断接收处理
extern unsigned int   usart3delay;           //通信超时时间，定义大于5ms,为通信超时
extern unsigned char  FLAG_uart3;			      //UART2开始新的一帧数据接收前先使能接收允许标志，接受过程中保持为1，在定时器发生接收超时时被清零
extern unsigned char  Debug_Shandle;	//允许帧处理标志
	 
	 
//设备基本信息
#pragma pack(1)
struct Device_Manage
{					
	  unsigned char  name[20];  		//设备名称
		unsigned char  factory[20];  	//生产厂家
	  unsigned char  Model[20];  		//设备型号
	  unsigned short device_code;   //产品编码，起始编码10000，与协议对应
	  unsigned short Station_NO;    //设备站号
	  unsigned short online;        //设备站号
	  unsigned short baud_rate;     //设备波特率 
	  unsigned short Parity_bits;   //设备奇偶校验位
	  unsigned short run_start;     //开机状态
};
#pragma pack()

//调试信息开关
#pragma pack(1)
struct debug_onoff
{					
	  unsigned char  Gaspower;  		//燃气壁挂炉调试信息打印
	  unsigned char  EC600N;  			//EC600N调试信息打印
		unsigned char  GasCost;  			//燃气积算信息打印
		unsigned char  MQ_send;  			//发布信息打印
};
#pragma pack()

extern struct Device_Manage Device_Manage; //设备信息
extern struct debug_onoff debug_onoff;     //调试信息开关

void printfArry(unsigned char *arry,unsigned char num); //打印数组

extern	void Handle_Debug(void);
#ifdef __cplusplus
}
#endif
#endif /*__ usart_H */
