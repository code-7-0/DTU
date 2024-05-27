#ifndef __LTE_H
#define __LTE_H
#include "ec600.h"
#include "Gaspower.h"
#include "flash.h"
#include "topic.h" 
#include "cJSON.h" 
#include "rtc.h" 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "temper_control.h"
typedef struct {
    char deviceID[20];     
    char key[50];          
    int value;             
} Variable_Device;

typedef struct {
    char key[25];          
    int value;            
} Fixed_Device;

typedef struct {
    uint32_t OTA_store_addr; //   固件读取地址     
	  uint8_t update_flag;//上次升级是否完毕
	  uint16_t ota_version;//固件版本号
			//ota地址 
			 // ota服务器端口号  
} Param_Ota;

void handle_MQTTConvert(void);//接收到云端命令后执行命令透传
void handle_Fixed_Convert(Fixed_Device* fixDevice);
void handle_Fixed_Calib_Convert(void);
void clear_bufer(char *arry,unsigned int dat);
extern void   Handle_Lte(void);
extern uint8_t	temper_rapif_reply_cnt;
extern uint8_t	temper_rapif_reply_flag;
#endif
