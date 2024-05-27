#include "linked_list.h" 
/*******���ݽṹ����*******/
LinkedList Linked_List;

//��ʼ������
void initLinkedList(LinkedList* list) {
    list->head = NULL;
    list->tail = NULL;
//		list->delet = NULL;
//		list->add = NULL;
}


//�������豸
Device* createDevice(DeviceBaseInfo* device_baseinfo) {
	 //�����Ƿ��Ѵ���device_baseinfo
//		DeviceBaseInfo device_baseinfo_temp;
//		for(uint8_t i=0;i<4;i++)//С��ģʽת���ģʽ
//			device_baseinfo_temp.sub_id[i] = device_baseinfo->sub_id[3-i];	
		if(findDevice(&Linked_List,device_baseinfo->sub_id)!=NULL)
				return NULL;
    Device* newDevice = (Device*) malloc(sizeof(Device));
    if (newDevice == NULL) {
        return NULL; // �ڴ����ʧ��
    }
		memset(newDevice,0,sizeof(Device));
//		for(uint8_t i=0;i<4;i++)//С��ģʽת���ģʽ
//			(newDevice->device_info).device_base_info.sub_id[i] = device_baseinfo->sub_id[3-i];
		(newDevice->device_info).device_base_info=*device_baseinfo;
    newDevice->next = NULL;
//		newDevice->device_info.online=1;//�豸��Ӽ�Ϊ����
    return newDevice;
}


//����豸������
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


//�����豸
Device* findDevice(LinkedList* list,  uint8_t* sub_id) {
    Device* temp = list->head;
    while (temp != NULL) {
        if (memcmp(temp->device_info.device_base_info.sub_id, sub_id,4) == 0) {
//            return &(temp->device_info);
						return temp;
        }
        temp = temp->next;
    }
    return NULL; // �豸δ�ҵ�
}


//ɾ���豸
void deleteDevice(LinkedList* list,  uint8_t* sub_id) {
    Device *current = list->head;
    Device *previous = NULL;

    while (current != NULL) {
        if (memcmp(current->device_info.device_base_info.sub_id, sub_id,4) == 0) {
						//����ͷ�ڵ㡢β�ڵ���м�ڵ�Ĵ���
            if (previous == NULL) {
                list->head = current->next; // ɾ������ͷ���ڵ�
            } else {
                previous->next = current->next; // �м��β���ڵ�
            }
            if (current == list->tail) {
                list->tail = previous; // ����βָ��
            }
            free(current); // �ͷ��ڴ�
            return;
        }
        previous = current;
        current = current->next;
    }
}



 
extern void debug_printf(const char *fmt, ...);
// ���������������豸����ӡ��Ϣ
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
        current = current->next; // �ƶ�����һ���ڵ�
    }
    
    if (count == 0) {
        debug_printf("No devices found in the list.\n");
    }
}




