#ifndef __debug_H
#define __debug_H
#include "serial_transceiver.h"
#include "flash.h"
#include "Gaspower.h"	 
#include "temper_control.h" 
	 
//设备基本信息
//#pragma pack(1)
//struct Device_Manage
//{					
//	  uint8_t  name[20];  		//设备名称
//		uint8_t  factory[20];  	//生产厂家
//	  uint8_t  Model[20];  		//设备型号
//	  uint16_t device_code;   //产品编码，起始编码10000，与协议对应
//	  uint16_t Station_NO;    //设备站号
//	  uint16_t online;        //设备站号
//	  uint16_t baud_rate;     //设备波特率 
//	  uint16_t Parity_bits;   //设备奇偶校验位
//	  uint16_t run_start;     //开机状态
//};
//#pragma pack()



//extern struct Device_Manage Device_Manage; //设备信息

extern void Handle_Debug(void);
extern uint8_t	temper_rapif_reply_flag;
extern uint8_t	temper_rapif_reply_cnt;
#endif 
