#ifndef __LINKED_LIST_H
#define __LINKED_LIST_H
#include "global.h"  
#include <string.h> 
#include <stdlib.h> 
 typedef struct {
//    uint8_t pk_type;      // 产品类型
    uint8_t sub_id[4];      // 子设备标识，4字节
//    uint8_t pid[16];         // 产品标识
//    uint8_t ver[3];         // 版本号
//    uint8_t ota;          // OTA支持
} DeviceBaseInfo;
 
 typedef struct { 
		uint8_t sw;   	 //开关 bool:
		uint8_t mode;  //模式 enum:auto, manual 
		uint16_t temp_set;			//设定温度50-3000/10
    int16_t  temp_current;    // 当前温度 -300-1000/10
} DeviceData;
 
typedef struct {  
//		uint8_t num;
		DeviceBaseInfo device_base_info;
		//参数信息
		uint8_t online;//在线状态
		DeviceData device_data;
} DeviceInfo;


typedef struct Device {
    DeviceInfo  device_info; 
    struct Device* next;
} Device;

typedef struct LinkedList {
    Device* head; // 指向链表头部的指针
    Device* tail; // 指向链表尾部的指针
//		Device* delet;
//		Device* add;
} LinkedList;

extern LinkedList Linked_List;
 //初始化链表
extern void initLinkedList(LinkedList* list) ;
//创建新设备
Device* createDevice(DeviceBaseInfo* device_baseinfo); 
//添加设备到链表
extern Device* addDevice(LinkedList* list, DeviceBaseInfo* device_baseinfo);
//查找设备
Device* findDevice(LinkedList* list,  uint8_t* sub_id);
//删除设备
void deleteDevice(LinkedList* list,  uint8_t* sub_id);

//uint8_t addMissingDevices(uint8_t** current_sub_ids, uint8_t sub_num, LinkedList* list);
//void removeExtraDevices(uint8_t** current_sub_ids, uint8_t sub_num, LinkedList* list);
#endif 




