#ifndef __LINKED_LIST_H
#define __LINKED_LIST_H
#include "global.h"  
#include <string.h> 
#include <stdlib.h> 
 typedef struct {
//    uint8_t pk_type;      // ��Ʒ����
    uint8_t sub_id[4];      // ���豸��ʶ��4�ֽ�
//    uint8_t pid[16];         // ��Ʒ��ʶ
//    uint8_t ver[3];         // �汾��
//    uint8_t ota;          // OTA֧��
} DeviceBaseInfo;
 
 typedef struct { 
		uint8_t sw;   	 //���� bool:
		uint8_t mode;  //ģʽ enum:auto, manual 
		uint16_t temp_set;			//�趨�¶�50-3000/10
    int16_t  temp_current;    // ��ǰ�¶� -300-1000/10
} DeviceData;
 
typedef struct {  
//		uint8_t num;
		DeviceBaseInfo device_base_info;
		//������Ϣ
		uint8_t online;//����״̬
		DeviceData device_data;
} DeviceInfo;


typedef struct Device {
    DeviceInfo  device_info; 
    struct Device* next;
} Device;

typedef struct LinkedList {
    Device* head; // ָ������ͷ����ָ��
    Device* tail; // ָ������β����ָ��
//		Device* delet;
//		Device* add;
} LinkedList;

extern LinkedList Linked_List;
 //��ʼ������
extern void initLinkedList(LinkedList* list) ;
//�������豸
Device* createDevice(DeviceBaseInfo* device_baseinfo); 
//����豸������
extern Device* addDevice(LinkedList* list, DeviceBaseInfo* device_baseinfo);
//�����豸
Device* findDevice(LinkedList* list,  uint8_t* sub_id);
//ɾ���豸
void deleteDevice(LinkedList* list,  uint8_t* sub_id);

//uint8_t addMissingDevices(uint8_t** current_sub_ids, uint8_t sub_num, LinkedList* list);
//void removeExtraDevices(uint8_t** current_sub_ids, uint8_t sub_num, LinkedList* list);
#endif 




