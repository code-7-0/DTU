#include "linked_list.h" 
/*******数据结构链表*******/
LinkedList Linked_List;

//初始化链表
void initLinkedList(LinkedList* list) {
    list->head = NULL;
    list->tail = NULL;
//		list->delet = NULL;
//		list->add = NULL;
}


//创建新设备
Device* createDevice(DeviceBaseInfo* device_baseinfo) {
	 //查找是否已存在device_baseinfo
//		DeviceBaseInfo device_baseinfo_temp;
//		for(uint8_t i=0;i<4;i++)//小端模式转大端模式
//			device_baseinfo_temp.sub_id[i] = device_baseinfo->sub_id[3-i];	
		if(findDevice(&Linked_List,device_baseinfo->sub_id)!=NULL)
				return NULL;
    Device* newDevice = (Device*) malloc(sizeof(Device));
    if (newDevice == NULL) {
        return NULL; // 内存分配失败
    }
		memset(newDevice,0,sizeof(Device));
//		for(uint8_t i=0;i<4;i++)//小端模式转大端模式
//			(newDevice->device_info).device_base_info.sub_id[i] = device_baseinfo->sub_id[3-i];
		(newDevice->device_info).device_base_info=*device_baseinfo;
    newDevice->next = NULL;
//		newDevice->device_info.online=1;//设备添加即为在线
    return newDevice;
}


//添加设备到链表
Device* addDevice(LinkedList* list, DeviceBaseInfo* device_baseinfo) {
    Device* newDevice = createDevice(device_baseinfo);
    if (newDevice == NULL) {
//        printf("Failed to add device due to memory allocation error.\n");
        return NULL;
    }
    if (list->head == NULL) {
        list->head = newDevice;
        list->tail = newDevice;
    } else {
        list->tail->next = newDevice;
        list->tail = newDevice;
    }
		return newDevice;
}


//查找设备
Device* findDevice(LinkedList* list,  uint8_t* sub_id) {
    Device* temp = list->head;
    while (temp != NULL) {
        if (memcmp(temp->device_info.device_base_info.sub_id, sub_id,4) == 0) {
//            return &(temp->device_info);
						return temp;
        }
        temp = temp->next;
    }
    return NULL; // 设备未找到
}


//删除设备
void deleteDevice(LinkedList* list,  uint8_t* sub_id) {
    Device *current = list->head;
    Device *previous = NULL;

    while (current != NULL) {
        if (memcmp(current->device_info.device_base_info.sub_id, sub_id,4) == 0) {
						//对于头节点、尾节点和中间节点的处理
            if (previous == NULL) {
                list->head = current->next; // 删除的是头部节点
            } else {
                previous->next = current->next; // 中间或尾部节点
            }
            if (current == list->tail) {
                list->tail = previous; // 更新尾指针
            }
            free(current); // 释放内存
            return;
        }
        previous = current;
        current = current->next;
    }
}



 
extern void debug_printf(const char *fmt, ...);
// 函数：遍历所有设备并打印信息
void traverseDevices(void) {
    Device* current = Linked_List.head;
    int count = 0;
    
    while (current != NULL) {
        debug_printf("Device %d:\n", ++count);
				uint8_t *id=current->device_info.device_base_info.sub_id;
				debug_printf("  Sub ID: %u,%u,%u,%u\n", id[0],id[1],id[2],id[3]); 
        debug_printf("  Switch : %d\n", current->device_info.device_data.sw);
				debug_printf("  mode: %d\n", current->device_info.device_data.mode);
				debug_printf("  temp_set: %d\n", current->device_info.device_data.temp_set);
        debug_printf("  temp_current: %d\n", current->device_info.device_data.temp_current);
				debug_printf("  ONLINE: %d\n", current->device_info.online);
        current = current->next; // 移动到下一个节点
    }
    
    if (count == 0) {
        debug_printf("No devices found in the list.\n");
    }
}




