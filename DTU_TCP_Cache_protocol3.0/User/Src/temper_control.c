#include "temper_control.h"  
 
/*******��ͬ�����ֵĽ�������*******/
///////////ģ����������/////////////

//�������豸����0x01
//mqtt����-��ջ-��ջ-��������ϱ�mqtt-�ȴ�0x03 60s-δ�յ�����0x02 							
// �ر����豸���� 0x02
 //mqtt����-��ջ-��ջ-��������
//���豸ɾ�� 0x04
//mqtt����-��ջ-��ջ-��������-ɾ������-mqtt����
//״̬��ѯ 0x05
//��ջ-��ջ-�������� 
//�����·� 0x06
//mqtt����-��ջ-��ջ-�������� 
//��ѯ���豸�б� 0x0A
//��ջ-��ջ-��������-��ѯ�������

///////////������������/////////////
//���豸��� 0x03
//��������-��������-mqtt����,0x01��ʱ����
//״̬�ϱ� 0x07
//��������-�޸�����-��mqtt��ʱ������
//��ȡ����ʱ�� 0x08
//mqttʱ���ѯ����-�ظ�����
//����ɾ�����豸 0x09
//��������-�ظ�����-mqtt����-ɾ������
//�ϱ����豸����״̬ 0x0B
//��������-�޸�����-mqtt����
//��ȡ����״̬ 0x0C
//��������- 
//Զ������ 0x0D

/*******mqtt���Ĺ���*******/


/*******mqtt���Ľ���*******/


/*******��������*******/

//0   1   2   3    4
//[]  []      []  []
/*******datapoint���������*******/
typedef enum {
    RAW = 0x00,
    BOOL = 0x01,
    INT = 0x02,
    STRING = 0x03,
    ENUM = 0x04,
    BITMAP = 0x05
} DatapointType;

typedef struct {
    uint8_t dpid;     // ���ݵ����
    uint8_t type;  // ��������
    uint8_t len;     // ���ݳ���
//	  uint16_t value;
    void * value;      // ָ������ֵ��ָ��
} Datapoint;
#define TOTAL_DP_COUNT 4
Datapoint datapoints[4] = {
    {1, BOOL, 1 ,NULL},
    {2, ENUM, 1 ,NULL},
    {16, INT, 2 ,NULL},
    {24, INT, 2 ,NULL}
};
uint8_t construct_06_cmd_param(uint8_t command,LinkedList* list, uint8_t* sub_id, DeviceData *device_data) {
    uint16_t index = 0;
		uint8_t buffer[25];//23
	
    Device* device = findDevice(list, sub_id);
    if (device == NULL) return NULL; // �豸δ�ҵ�
//		DeviceData *device_data=&(device->device_info.device_data); 
	
		//���豸id
		buffer[index++]=4;
		memcpy(&buffer[index],sub_id,4);
		index+=4;
		//�ı������
		for (int j = 0; j < TOTAL_DP_COUNT-1; j++) {
					buffer[index++] = datapoints[j].dpid;
					buffer[index++] = datapoints[j].type;
					buffer[index++] = datapoints[j].len ;

					switch (datapoints[j].dpid) {
							case 1: // ����
									buffer[index++]=device_data->sw;  
									break;
							case 2: // ģʽ
									buffer[index++]=device_data->mode;  
									break;
							case 16: // �趨�¶�
									buffer[index++]=(device_data->temp_set)>>8 ;  
									buffer[index++]=(device_data->temp_set)&0xFF;  
									break;
//							case 24: // ��ǰ�¶�
//									buffer[index++]=(device_data->temp_current)>>8 ;  
//									buffer[index++]=(device_data->temp_current)&0xFF; 						
//									break;
							default:
									// �����������ݵ�
									break;
					}
		}
   
		// ���仺���� ����ջ
		TemperCommand temper_send = {0, 0, NULL};
		temper_send.command=command;
		temper_send.param_length=index;  
    temper_send.params = (uint8_t*)malloc(index);
    if (temper_send.params != NULL) {
        // ����sub_id��������
        memcpy(temper_send.params, buffer, index); 
    }		
		 // ��������뷢�Ͷ���
    TemperFiFoPush(&temper_send);
    return index;  // ���ع��������ݳ���
}
//uint8_t* prepare_datapoint(const uint8_t* dpids, uint8_t num_dpids, uint8_t* sub_id) {
//    uint16_t index = 0;
//		uint8_t buffer[20];
//		//���豸id
//		buffer[index++]=4;
//		memcpy(&buffer[index],sub_id,4);
//		index+=4;
//		//�ı������
//    for (int i = 0; i < num_dpids; i++) {
//        uint8_t dpid = dpids[i];
//        for (int j = 0; j < TOTAL_DP_COUNT; j++) {
//            if (datapoints[j].dpid == dpid) {
//                buffer[index++] = datapoints[j].dpid;
//                buffer[index++] = datapoints[j].type;
//                buffer[index++] = datapoints[j].len ;
//							
//                // �����������ͽ��д�˴���
//                switch (datapoints[j].type) {
//                    case BOOL:
//                    case ENUM:
//                    case RAW:
//                    case STRING:
//                        // ֱ�Ӹ�������
//                        memcpy(&buffer[index], datapoints[j].value, datapoints[j].len);
//                        break;
//                    case INT:
//                    case BITMAP:
//                        // ��˷�ʽ������������
//                        for (int k = 0; k < datapoints[j].len; k++) {
//                            buffer[index + k] = ((uint8_t*)datapoints[j].value)[datapoints[j].len - 1 - k];
//                        }
//                        break;
//                }
//                index += datapoints[j].len;
//                break;  // �ҵ���Ӧ��dpid��Ϳ�������ѭ��
//            }
//        }
//    }
//		
//	   // ���仺����
//    uint8_t* buff = (uint8_t*)malloc(index);
//    if (buff != NULL) {
//        // ����sub_id��������
//        memcpy(buff, sub_id, index); 
//    }		
//    return buff;  // ���ع��������ݳ���
//}
 
/**
* @brief Ϊ0x06�����·�����׼��datapoint����
* @param datapoint - ���ݵ�ṹ��
*				 buffer - ָ��Ҫ����buffer��ָ��
* @return ��������ݳ���
**/ 
//uint16_t prepare_datapoint(Datapoint *datapoint, uint8_t* buffer) {
//    uint16_t index = 0;
//    buffer[index++] = datapoint->dpid;      // ���ݵ����
//    buffer[index++] = datapoint->type;      // ��������

//    // ����������ݳ���
//    buffer[index++] = (datapoint->len >> 8) & 0xFF;
//    buffer[index++] = datapoint->len & 0xFF;

//    // ���������������value
//    switch (datapoint->type) {
//        case BOOL:
//        case ENUM:
//        case RAW:
//            memcpy(&buffer[index], datapoint->value, datapoint->len);
//            index += datapoint->len;
//            break;
//        case INT:
//            // ������������Ϊ���
//            for (int i = 0; i < datapoint->len; i++) {
//                buffer[index++] = ((uint8_t*)datapoint->value)[datapoint->len - 1 - i];
//            }
//            break;
//        case STRING:
//            memcpy(&buffer[index], datapoint->value, datapoint->len);
//            index += datapoint->len;
//            break;
//        case BITMAP:
//            memcpy(&buffer[index], datapoint->value, datapoint->len);
//            index += datapoint->len;
//            break;
//    }
//    return index;  // ����buffer�е����ݳ���
//}

//���datapoint��Ԫ
//uint16_t prepare_datapoints(Datapoint *datapoints, int count, uint8_t* buffer) {
//    uint16_t index = 0;
//    for (int i = 0; i < count; i++) {
//        buffer[index++] = datapoints[i].dpid;
//        buffer[index++] = datapoints[i].type;
//        buffer[index++] = (datapoints[i].len >> 8) & 0xFF;
//        buffer[index++] = datapoints[i].len & 0xFF;
//        memcpy(&buffer[index], datapoints[i].value, datapoints[i].len);
//        index += datapoints[i].len;
//    }
//    return index;
//}

// ���ܣ��������յ���datapoints����
// ������buffer - ���յ������ݻ�����
//       length - �������ĳ���
//       datapoints - ָ���ѷ����㹻�ռ��datapoint�����ָ��
//       max_count - ���Դ洢�����datapoint����
// ���أ�������datapoint����
//uint8_t parse_07_datapoints(const uint8_t* buffer, uint16_t length, Datapoint* datapoints ) {
//    uint16_t index = 0;
//    uint8_t count = 0;

//    while (index < length /*&& count < max_count*/) {
//        if (index + 4 > length) {
//            break; // ��ֹ���������
//        }
//        
//        datapoints[count].dpid = buffer[index++];
//        datapoints[count].type = buffer[index++];
//        datapoints[count].len = (buffer[index] << 8) | buffer[index + 1];
//        index += 2;

//        if (index + datapoints[count].len > length) {
//            break; // ��ֹ���������
//        }

//        datapoints[count].value = malloc(datapoints[count].len);
//        if (datapoints[count].value == NULL) {
//            break; // �ڴ����ʧ��
//        }

//        memcpy(datapoints[count].value, &buffer[index], datapoints[count].len);
//        index += datapoints[count].len;
//        count++;
//    }

//    return count; // ���ؽ�����datapoint����
//}



// �������ݲ������豸״̬
uint8_t parse_07_datapoints( uint8_t* buffer, uint16_t length, LinkedList* device_list) {
    if (length < 4) return 0; // ������Ҫ���豸ID

//    uint8_t sub_id[4];
//    memcpy(sub_id, buffer, sizeof(sub_id));
    Device* device = findDevice(device_list, &buffer[1]);
    if (device == NULL) return 0; // �豸δ�ҵ�
		DeviceInfo *device_info=&(device->device_info);
		device_info->online=1;//�豸����
	
    uint16_t index = 5; // �������豸ID
    uint8_t count = 0;
		
    while (index < length) {
        if (index + 4 > length) break; // ��ֹ���������

        uint8_t dpid = buffer[index++];
        uint8_t type = buffer[index++];
        uint16_t len = buffer[index++];
//        index += 2;

        if (index + len > length) break; // ��ֹ���������

        switch (dpid) {
            case 1: // ����
								device_info->device_data.sw = buffer[index];
                break;
            case 2: // ģʽ
                device_info->device_data.mode = buffer[index];
                break;
            case 16: // �趨�¶�
                device_info->device_data.temp_set = (buffer[index] << 8) | buffer[index + 1];
                break;
            case 24: // ��ǰ�¶�
                device_info->device_data.temp_current = (buffer[index] << 8) | buffer[index + 1];
                break;
            default:
                // �����������ݵ�
                break;
        }
        index += len;
        count++;
    }

    return count; // ���ظ��µ����ݵ�����
}

//��������ṹ�����͡����ܣ�
void base_command_construct(uint8_t *frame,TemperCommand* temper_send) {
//    uint8_t frame[256]; // ֡���飬��С���������ܵ�֡�����趨
    uint16_t index = 0;

    // ֡ͷ
    frame[index++] = FRAME_HEADER >> 8;
    frame[index++] = FRAME_HEADER & 0xFF;

    // �汾��
    frame[index++] = VERSION;

    // ������
    frame[index++] = temper_send->command;

    // ���ݳ���
    frame[index++] = (temper_send->param_length) >> 8;
    frame[index++] = (temper_send->param_length) & 0xFF;

		if(temper_send->param_length>0){
    // ����
			memcpy(&frame[index], temper_send->params, temper_send->param_length);
			index += temper_send->param_length;
		}
		
    // У���
    frame[index] = calculate_checksum(frame, index);
		index++;
    // ��������
    Gateway_Usart_Send(frame, index); 
}






/*******��������*******/
//void construct_04_delete_subdevice_param(uint8_t command,char* sub_id )
//{
//		TemperCommand temper_send={0,0,0};
//		uint8_t tp=0;
//		
//		temper_send.command=command;
//		
//  // ������Ҫ�Ļ�������С
//    temper_send.param_length = snprintf(NULL, 0, "{\"sub_id\":\"%s\",\"tp\":%d}", sub_id, tp) ; 
//		uint16_t BufferSize= temper_send.param_length+1;
//    char *commandBuffer = (char *)malloc(temper_send.param_length+1);

//    if (commandBuffer != NULL) {
//			    // �����ַ���
//				snprintf(commandBuffer, BufferSize, "{\"sub_id\":\"%s\",\"tp\":%d}", sub_id, tp); 
//			 temper_send.params=(uint8_t*)commandBuffer; 
//    } 
//		TemperFiFoPush(&temper_send);	
//		   
//} 
void construct_04_delete_subdevice_param(uint8_t command,uint8_t* sub_id )
{
    TemperCommand temper_send = {0, 0, NULL};
    temper_send.command = command; 
		
    //   sub_id ����4���ַ�������tpΪ1���ֽ�
    int sub_id_len = 4;   
    uint8_t tp = 0;       // 0:ɾ�����豸��1���ظ���������

    // �ܵĲ������� = sub_id�ĳ��� + tp�Ĵ�С
    temper_send.param_length = sub_id_len + 1;

    // ���仺����
    temper_send.params = (uint8_t*)malloc(temper_send.param_length);
    if (temper_send.params != NULL) {
        // ����sub_id��������
        memcpy(temper_send.params, sub_id, sub_id_len);
        // ����tp��ֵ
        temper_send.params[sub_id_len] = tp;
    }

    // ��������뷢�Ͷ���
    TemperFiFoPush(&temper_send);
    // free(temper_send.params);	
		   
} 
//0x0A 
//uint8_t parse_0A_query_subdevice_list(uint8_t* data, size_t data_length) {
//    if (data == NULL || data_length < 2) { 
//        return 0;
//    }
//    
//    uint8_t offset = 0;// ��ǰ��ȡλ�� 
//    uint8_t index = data[offset] & 0x7F;
//    uint8_t id_bit7 = (data[offset++] >> 7) & 0x01;// ��7λ

//    uint8_t sub_num = data[offset++];//������ݵ����豸����
//    if (offset + sub_num > data_length) return 0;

//    for (int i = 0; i < sub_num && offset < data_length; i++) {//offset��ֹԽ��
//        uint8_t sub_len = data[offset++];
//        if (sub_len > 18 || offset + sub_len > data_length) {
//            return 0; // У�����ݳ��ȣ���ֹԽ��
//        }
//        char *sub_id = malloc(sub_len + 1); // ��̬����
//        if (sub_id == NULL) return 0; // �ڴ����ʧ��
//        memcpy(sub_id, &data[offset], sub_len);
//        sub_id[sub_len] = '\0'; // null-terminated string
////        printf("Sub-device %d: ID = %s\n", i + 1, sub_id);
//        offset += sub_len;
//        free(sub_id);
//    }
//    return 1;
//}
//0X0A�������
 // �������������豸ID����Ӳ����ڵ��豸
uint8_t addMissingDevices(uint8_t** current_sub_ids, uint8_t sub_num, LinkedList* list) {
		uint8_t state=0;
    for (uint8_t i = 0; i < sub_num; i++) {
        if (findDevice(list, current_sub_ids[i]) == NULL) {
					//// mqtt�ϱ��豸���
							MqttTopicFiFoPush(ADD,current_sub_ids[i]);
//						uint8_t temp[4];
//						for(uint8_t j=0;j<4;j++)//���ģʽתС��ģʽ
//								temp[j] = current_sub_ids[i][3-j];
						//������� 
            if(addDevice(list, (DeviceBaseInfo*)current_sub_ids[i])!=NULL)
							state=1;
        }
    }
		return state;
}

// ɾ�������в������ڵ�ǰ�豸ID�б���豸
void removeExtraDevices(uint8_t** current_sub_ids, uint8_t sub_num, LinkedList* list) {
    Device* temp = list->head;
    Device* prev = NULL;
    while (temp != NULL) {
        uint8_t found = 0;
        for (uint8_t i = 0; i < sub_num; i++) {
            if (memcmp(temp->device_info.device_base_info.sub_id, current_sub_ids[i],4) == 0) {
								
                found = 1;
                break;
            }
        }
        if (!found) {
						//// mqtt�ϱ��豸ɾ�� DELETE��sub_id  
						MqttTopicFiFoPush(DELETE,temp->device_info.device_base_info.sub_id);	
						//����ɾ��
            Device* toDelete = temp;
            if (prev == NULL) {
                list->head = temp->next; // ɾ��ͷ�ڵ�
                temp = list->head;
            } else {
                prev->next = temp->next; // ɾ���м��β���ڵ�
                temp = prev->next;
            }
						
            if (toDelete == list->tail) {
                list->tail = prev; // ����βָ��
            }
            free(toDelete); // �ͷ��ڴ�
						debug_printf("delete device!");
						
        } else {
            prev = temp;
            temp = temp->next;
        }
    }
}

//��ѯƵ�ʣ�
//�������豸�б���Ӧ���������������޸�
uint8_t parse_0A_query_subdevice_list(uint8_t* data, size_t data_length, LinkedList* list) {
    if (data == NULL || data_length < 2) { 
        return 0;
    }
    
    uint8_t offset = 0;
		offset++;
    uint8_t sub_num = data[offset++];
    if (offset + sub_num > data_length) return 0;

    uint8_t** current_sub_ids = malloc(sizeof(uint8_t*) * sub_num);
    if (current_sub_ids == NULL) return 0;

    uint8_t sub_len;
    for (int i = 0; i < sub_num && offset < data_length; i++) {
        sub_len = data[offset++];
        if (offset + sub_len > data_length) {
            for (int j = 0; j < i; j++) free(current_sub_ids[j]);
            free(current_sub_ids);
            return 0;
        }
        current_sub_ids[i] = malloc(sub_len);
        if (current_sub_ids[i] == NULL) {
            for (int j = 0; j < i; j++) free(current_sub_ids[j]);
            free(current_sub_ids);
            return 0;
        }
        memcpy(current_sub_ids[i], &data[offset], sub_len); 
        offset += sub_len;
    }
		
		//����������������豸
    uint8_t state=addMissingDevices(current_sub_ids, sub_num, list);
		//������ɾ��������豸
    removeExtraDevices(current_sub_ids, sub_num, list);

    for (int i = 0; i < sub_num; i++) free(current_sub_ids[i]);
    free(current_sub_ids);

		//mqtt�ϱ������豸��
		MqttTopicFiFoPush(IDLIST,NULL);
		//���������豸 ����05�����ѯ״̬
		if(state==1)
			construct_send_command_to_fifo(0x05,NULL,NULL);
    return 1;
}
//���췢�����fifo��
void construct_send_command_to_fifo(uint8_t command, uint8_t* sub_id,DeviceData *device_data)
{ 
	
		switch(command)
		{
			case CMD_DELETE_SUBDEVICE:// 0x04���豸ɾ�� 
				//����id����ɾ���ַ���
				construct_04_delete_subdevice_param(command,sub_id); 
				break;			 
			case CMD_ISSUE_COMMAND:// 0x06�����·�
				//id ���������ơ���ֵ 
				construct_06_cmd_param(command, &Linked_List,sub_id,device_data);
				break;
			default:
			{
			//�������豸���� 0x01 
			//�ر����豸���� 0x02 
			//   �����·�   0x05 			
			//��ѯ���豸�б� 0x0A 	
				if(Linked_List.head!=NULL || command!=CMD_QUERY_STATUS)//0x05������Ϊ�ղ���ջ
				{
					TemperCommand temper_send={0,0,0};//��ջ
					temper_send.command=command; 
					TemperFiFoPush(&temper_send);		
				}					
			}			
				break;
			
		}   
}


char* extract_delete_sub_id(char *json) {
    const char *key = "\"sub_id\":\"";  // ���ҵĹؼ���
    char *start, *end;

    // ���� key �� json �ַ����е�λ��
    start = strstr(json, key);
    if (start != NULL) {
        start += strlen(key);  // �ƶ�ָ�뵽 key �������
        end = strchr(start, '"');  // ���ҽ�������
        if (end != NULL) {
						*end='\0';
						return start;
//            memcpy(output, start, end - start);  // ��ȡ���ݵ��������
//            output[end - start] = '\0';  // ����ַ���������
        } else {
						return NULL;
//            output=NULL;  // δ�ҵ���������
        }
    } else {
						return NULL;
//        output=NULL;// δ�ҵ��ؼ���
    }
}


uint8_t waiting_for_response = 0;  
uint8_t unsolicited_message_flag=0;
uint8_t expected_command=0;//�ڴ���Ӧ��
uint16_t response_timeout_cnt;
TemperCommand	response_command;
//uint8_t delete_id[4];//Ҫɾ����id���ַ�����ַ
//#define MAX_RETRIES 3
//#define RESPONSE_TIMEOUT 200
enum{
	SEND_IDLE=0,
	SEND_CONSTRUCT,
	SEND_UNSOLICITED,
	SEND_WAIT
};
//char sub_id[18];

//����ظ�������
uint8_t add_device_time_cnt=0;
void process_command_response(TemperCommand *ask,TemperCommand *response) { 
 
    switch (response->command) {
				case CMD_ALLOW_JOIN_NETWORK:// 01����
				{
					//60s�ȴ��豸���0x03��ʱ
					add_device_time_cnt=60; 
					// mqtt�ϱ�:�����ɹ�   
					MqttTopicFiFoPush(NETIN,NULL);						
				}
				break;			
				case CMD_DELETE_SUBDEVICE:// 04���豸ɾ��
				{					
					//ɾ����������  
					deleteDevice(&Linked_List,ask->params); 
				//ɾ���ɹ���mqtt�ϱ�   mqtt�ϱ����в�����action��id 
					MqttTopicFiFoPush(DELETE,ask->params);					
				}
				break;
				
        case CMD_REPORT_STATUS:// 0x07��Ӧ״̬��ѯ0x05
				{
            // ����״̬��ѯ��Ӧ ��������������
						parse_07_datapoints(response->params,response->param_length,&Linked_List);
						//  �����ϱ�ʱ��
						if(temper_rapif_reply_flag==1)
								Temper_Rapid_Reply();
				}
        break;
        case CMD_QUERY_SUBDEVICE_LIST:// ��ѯ���豸�б�0x0A
            // �������豸�б���Ӧ���������������޸�
						parse_0A_query_subdevice_list(response->params,response->param_length,&Linked_List);
  
            break; 
        default:
					//��������ֻУ������ṹ
            break;
    }
}












/*******����ظ�����*******/

//uint8_t parse_03_add_subdevice(const char* jsonString, DeviceBaseInfo* info) {
//    cJSON *json = cJSON_Parse(jsonString);
//    if (json == NULL) {
//        const char *error_ptr = cJSON_GetErrorPtr();
//        if (error_ptr != NULL) {
////            fprintf(stderr, "Error before: %s\n", error_ptr);
//        }
//        return 0;
//    }

//    // ������Ʒ����
////    const cJSON *pk_type = cJSON_GetObjectItemCaseSensitive(json, "pk_type");
////    if (cJSON_IsNumber(pk_type)) {
////        info->pk_type = (uint8_t)pk_type->valueint;
////    }

//    // �������豸��ʶ
//    const cJSON *sub_id = cJSON_GetObjectItemCaseSensitive(json, "sub_id");
//    if (cJSON_IsString(sub_id) && (sub_id->valuestring != NULL)) {
//        strncpy(info->sub_id, sub_id->valuestring, sizeof(info->sub_id) - 1);
//        info->sub_id[sizeof(info->sub_id) - 1] = '\0'; // Ensure null-termination
//    }

////    // ������Ʒ��ʶ
////    const cJSON *pid = cJSON_GetObjectItemCaseSensitive(json, "pid");
////    if (cJSON_IsString(pid) && (pid->valuestring != NULL)) {
////        strncpy(info->pid, pid->valuestring, sizeof(info->pid) - 1);
////        info->pid[sizeof(info->pid) - 1] = '\0'; // Ensure null-termination
////    }

////    // �����汾��
////    const cJSON *ver = cJSON_GetObjectItemCaseSensitive(json, "ver");
////    if (cJSON_IsString(ver) && (ver->valuestring != NULL)) {
////        strncpy(info->ver, ver->valuestring, sizeof(info->ver) - 1);
////        info->ver[sizeof(info->ver) - 1] = '\0'; // Ensure null-termination
////    }

////    // ����OTA֧��
////    const cJSON *ota = cJSON_GetObjectItemCaseSensitive(json, "ota");
////    if (cJSON_IsNumber(ota)) {
////        info->ota = (uint8_t)ota->valueint;
////    }

//    cJSON_Delete(json);
//		return 1;
//}
void parse_03_add_subdevice(const uint8_t* data, DeviceBaseInfo* info){
    // ���ݽṹ���ֶ�������ȡ����
    uint8_t offset = 0;
//    memcpy(&info->pk_type, data + offset, sizeof(info->pk_type));
//    offset += sizeof(info->pk_type);
		offset+=1;
    memcpy(&info->sub_id, data + offset, sizeof(info->sub_id));
//    offset += sizeof(info->sub_id);
//    memcpy(&info->pid, data + offset, sizeof(info->pid));
//    offset += sizeof(info->pid);
//    memcpy(&info->ver, data + offset, sizeof(info->ver));
//    offset += sizeof(info->ver);
//    memcpy(&info->ota, data + offset, sizeof(info->ota));
 	
}
 
// ����0x0B�����JSON���ݣ��������豸״̬
//uint8_t parse_0B_report_online(char* jsonString, LinkedList* list) {
//    cJSON* json = cJSON_Parse(jsonString);
//    if (json == NULL) {
//        const char* error_ptr = cJSON_GetErrorPtr();
//        if (error_ptr != NULL) {
////            fprintf(stderr, "Error in parseDeviceStatus: %s\n", error_ptr);
//        }
//        return 0; // ����ʧ��
//    }

//    cJSON* jsonSubId = cJSON_GetObjectItemCaseSensitive(json, "sub_id");
//    cJSON* jsonStatus = cJSON_GetObjectItemCaseSensitive(json, "status");

//    if (!cJSON_IsString(jsonSubId) || jsonSubId->valuestring == NULL ||
//        !cJSON_IsNumber(jsonStatus)) {
//        cJSON_Delete(json);
//        return 0; // ���ݲ�����
//    }

//    // �����豸
//    Device* device = findDevice(list, jsonSubId->valuestring);
//    if (device != NULL) {
//        // �����豸������״̬
//        device->device_info.onlinestate = (uint8_t)jsonStatus->valueint;
////        printf("Updated device %s online status to %d\n", jsonSubId->valuestring, device->device_info.onlinestate);
//    } else {
////        printf("Device not found: %s\n", jsonSubId->valuestring);
//    }

//    cJSON_Delete(json);
//    return 1; // �ɹ�
//} 
uint8_t parse_0B_report_online(LinkedList* list,uint8_t* data, uint16_t length) {
    if (data == NULL || length==0) {
        return 0; // ����ָ����Ч
    } 
		uint8_t status=data[length-1];//����״̬
    // �����豸 
		uint8_t data_temp[4];
		if(status==1){
			memcpy(data_temp,data,4); 
		}else{ 
			for(uint8_t i=0;i<4;i++)//С��ģʽת���ģʽ
				data_temp[i] = data[3-i];				 
		}
		Device* device= findDevice(list, data_temp);
    if (device != NULL) {
        // ������������״̬
        device->device_info.online  = status;//����״̬
 
			//����mqtt���� action:ONLINE,sub_id 
				MqttTopicFiFoPush(ONLINE,data_temp);				
    }  
		
    return 1; // �ɹ�
} 
//����0x09ɾ�������豸������
//uint8_t	parse_09_delete_local_device(char* jsonString)
//{
//		cJSON* json = cJSON_Parse(jsonString);
//    if (json == NULL) { // ��������
//        const char* error_ptr = cJSON_GetErrorPtr();
//        if (error_ptr != NULL) {
////            fprintf(stderr, "Error before: %s\n", error_ptr);
//        }
//        return 0; // ����0��ʾʧ��
//    }

//    // ��ȡɾ�������豸ID
//    cJSON* jsonSubId = cJSON_GetObjectItemCaseSensitive(json, "sub_id");
//    if (cJSON_IsString(jsonSubId) && (jsonSubId->valuestring != NULL)) { 
//        memcpy(delete_id, jsonSubId->valuestring, 4); 
//    } else {
//        cJSON_Delete(json);
//        return 0; // ����0��ʾʧ��
//    }

//    cJSON_Delete(json);
//    return 1; // ����1��ʾ�ɹ�
//}


void construct_09_delete_local_device(uint8_t command,uint16_t length)
{ 
    TemperCommand temper_send = {0, length, NULL}; // ʹ�ô����length
    temper_send.command = command; 
		uint8_t data=0;
		temper_send.params=&data;
		base_command_construct(g_tModule.Gateway_SendBuf,&temper_send);
} 
void construct_08_local_time(uint8_t command,uint16_t length,DateTime* time)
{ 
    TemperCommand temper_send = {0, length, NULL}; // ʹ�ô����length
    temper_send.command = command; 
		uint8_t data[]={1,datetime.year,datetime.month,datetime.day,datetime.hour,
										datetime.min,datetime.sec,datetime.week};
//		temper_send.params=(uint8_t*)time;
		temper_send.params=data;
		base_command_construct(g_tModule.Gateway_SendBuf,&temper_send);
} 

void construct_0c_net_status(uint8_t command,uint16_t length,uint8_t status)
{ 
    TemperCommand temper_send = {0, length, NULL}; // ʹ�ô����length
    temper_send.command = command;  
		temper_send.params=(uint8_t*)&status;
		base_command_construct(g_tModule.Gateway_SendBuf,&temper_send);
} 
void construct_nomarl_param(uint8_t command,uint16_t length)
{ 
    TemperCommand temper_send = {0, length, NULL}; // ʹ�ô����length
    temper_send.command = command; 
		base_command_construct(g_tModule.Gateway_SendBuf,&temper_send);
} 


void process_unsolicited_message(TemperCommand *tc) {   
    switch (tc->command) {
        case CMD_ADD_SUBDEVICE://03���豸���
				{ 
						//�������豸����  
//						DeviceBaseInfo  device_base_info;
//						parse_03_add_subdevice(tc->params, &device_base_info);
//					
//						//��������
////						Device* device_addr=addDevice(&Linked_List,&device_base_info); 
//						addDevice(&Linked_List,&device_base_info);
						// ����ȷ����Ϣ  
						construct_nomarl_param(tc->command,  0); 
					
						uint8_t *id_addr=tc->params+1;
						uint8_t sub_id[4];
						for(uint8_t i=0;i<4;i++)//С��ģʽת���ģʽ
							sub_id[i] = id_addr[3-i];							
						if(addDevice(&Linked_List,(DeviceBaseInfo*)sub_id)!=NULL){ 
								//����mqtt���ģ���ӵ�id��Ҫ�洢��  action��sub_id 
								MqttTopicFiFoPush(ADD,sub_id);
						}
						add_device_time_cnt=0;//0x01�ȴ���ʱ���� 
				}
					break; 
        case CMD_REPORT_STATUS://0x07״̬�ϱ�   ����ظ�
				{
					//����״̬�������޸�����
						parse_07_datapoints(tc->params,tc->param_length,&Linked_List); 
						//�����ϱ�ʱ��
						if(temper_rapif_reply_flag==1)
							Temper_Rapid_Reply();
				}				
					break;
        case CMD_GET_LOCAL_TIME://0x08��ȡ����ʱ��    
				{
					//�ظ�ʱ������(ʱ�乹�캯��)
						construct_08_local_time(tc->command,8,&datetime); 
					break;	
				}
        case CMD_DELETE_LOCAL_SUBDEVICE://0x09����ɾ�����豸  
				{
						//����ɾ������  
//						construct_09_delete_local_device(tc->command,1);//�ظ�ɾ���ɹ� 
						//����mqtt���� action:DELETE,sub_id 
						MqttTopicFiFoPush(DELETE,tc->params);							
						//���ͱ��ĺ� ɾ������
						deleteDevice(&Linked_List,tc->params);	 
				}
					break;		
				case CMD_REPORT_SUBDEVICE_ONLINE_STATUS:// 0x0B�ϱ����豸����״̬
				{
						//�������� ���޸����� �� ����mqtt����
						if(parse_0B_report_online(&Linked_List, tc->params,tc->param_length))
									construct_nomarl_param(tc->command,0);// ����ȷ����Ϣ  
				} 
					break;
				case CMD_GET_NETWORK_STATUS:// 0x0C�ϱ�����״̬
				{
						//�ظ�����״̬
						construct_0c_net_status(tc->command,1,5); 
				} 
					break;
        default: 
            break;
    }
}

 
 
//������յ�������
void gateway_receive_command(uint8_t* rx_buffer,uint16_t len) { 
//		expected_command = 0;
//		response_timeout_cnt=0;
    uint16_t received_header = (rx_buffer[0] << 8) | rx_buffer[1];
		// У��֡ͷ
    if (received_header != FRAME_HEADER) { 
        return;  
    }

    uint8_t version = rx_buffer[2];
    uint8_t command = rx_buffer[3];
    uint16_t data_length = (rx_buffer[4] << 8) | rx_buffer[5]; 
		if(len<(data_length+HEADER_SIZE+1))//���ȼ��
				return;
		// У��ͼ��
    uint8_t checksum = calculate_checksum(rx_buffer, data_length + HEADER_SIZE);
    if (checksum != rx_buffer[HEADER_SIZE + data_length]) { 
        return;  
    }
		
		setTemperCommand(&response_command,command,data_length, (rx_buffer+HEADER_SIZE));
    // ����������ڵȴ���Ӧ�ҽ��յ��������Ԥ��
    if (waiting_for_response && command == expected_command) {
//        process_command_response(command, rx_buffer + HEADER_SIZE, data_length);
        waiting_for_response = 0;//����ȴ���־λ
//				expected_command = 0;
//				response_timeout_cnt=0;
    } else {
        // �������Ӧ���Ϸ�����
				setTemperCommand(&response_command,command,data_length, (rx_buffer+HEADER_SIZE));
//				unsolicited_message_flag=1;
				process_unsolicited_message(&response_command); 
//        process_unsolicited_message(command,rx_buffer+ HEADER_SIZE, data_length);
    }
 	
//		//��������
//		switch (command) {
//				case CMD_ALLOW_JOIN_NETWORK://0x01�������豸����
//						handle_status_query(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_STOP_JOIN_NETWORK://0x02ֹͣ���豸����
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_ADD_SUBDEVICE://0x03���豸���
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_DELETE_SUBDEVICE://0x04���豸ɾ��
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_QUERY_STATUS://0x05״̬��ѯ
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_ISSUE_COMMAND://0x06�����·�
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_REPORT_STATUS://0x07״̬�ϱ�
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_GET_LOCAL_TIME://0x08��ȡ����ʱ��
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_DELETE_LOCAL_SUBDEVICE://0x09����ɾ�����豸
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_QUERY_SUBDEVICE_LIST://0x0A��ѯ���豸�б�
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_REPORT_SUBDEVICE_ONLINE_STATUS://0x0B�ϱ����豸����״̬
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_GET_NETWORK_STATUS://0x0C��ȡ����״̬
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;		
//				case CMD_REMOTE_UPGRADE://0x0DԶ������
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;						
//				// �����������Ĵ���		
//		}
}

//��������ȴ���Ӧ
void send_command_with_wait(void) { 
		static uint8_t  MAX_RETRIES=3;
		static uint16_t RESPONSE_TIMEOUT=200;
		static uint8_t send_state=SEND_IDLE,retry_count=0;
		static TemperCommand temper_send;//�����ջָ��
		switch (send_state)
		{
			case SEND_IDLE://�ȴ�����������
			{
					//����mqtt��debug����󣬹���temper_send��push��fifo
					if(TemperFiFoSize()>0)//���ڴ��������� 	
					{
						TemperFiFoPop(&temper_send);//��ջ
						send_state=SEND_CONSTRUCT; 
						expected_command=temper_send.command; 
						
						//���ñ�־λ
						if(temper_send.command==CMD_QUERY_STATUS){//����0X05�ظ�0x07
								MAX_RETRIES=0;
								RESPONSE_TIMEOUT=3000;
								expected_command=CMD_REPORT_STATUS;//0x07
						}else{
								MAX_RETRIES=3;
								RESPONSE_TIMEOUT=100;							
						}						 
					}else if (unsolicited_message_flag) {
                send_state = SEND_UNSOLICITED;
					}
			}
			break;
			case SEND_CONSTRUCT://��������
			{   
						//��������������
						base_command_construct(g_tModule.Gateway_SendBuf,&temper_send);  
						//���ñ�־λ
						response_timeout_cnt=RESPONSE_TIMEOUT;//�ȴ��ظ�ʱ��			
						waiting_for_response = 1;//��λ�ȴ���־λ
						send_state=SEND_WAIT;  
			}
			break;
			case SEND_WAIT://�ȴ�����ظ�
			{
					if(waiting_for_response==0)//��ʱ���ߵõ��ظ�
					{
						if(response_timeout_cnt>0)//200ms�ڵõ���Ӧ
						{
								//������յ�������
								process_command_response(&temper_send,&response_command);
								//��־λ����
								response_timeout_cnt=0;
								expected_command=0;
								send_state=SEND_IDLE;
						}
						else//��ʱδ��Ӧ���ط�
						{ 
								retry_count++; 
								if (retry_count >= MAX_RETRIES) {//�ش������������ֵ
										send_state = SEND_IDLE; 
//												expected_command=0; 
								} else {
										send_state = SEND_CONSTRUCT;
								}								
						}
						
						if(send_state==SEND_IDLE){//����ʧ�ܻ�ɹ��ظ������ͷ�ռ�õĿռ�
								if(temper_send.param_length > 0 && temper_send.params!=NULL){
									retry_count = 0;
									expected_command=0; 
									free(temper_send.params);//�ͷ�����
									
								}
						}
							
					} 
			}	
			break;
			case SEND_UNSOLICITED://�������Ӧ���Ϸ�����
					process_unsolicited_message(&response_command); 
					unsolicited_message_flag = 0; // ������Ϻ����ñ�־λ
					send_state = SEND_IDLE; // ���ؿ���״̬�ȴ��µ�����
					break;			
			default:
				send_state = SEND_IDLE;
				break;
		}
}



void Handle_Temper_Crontrol(void)	
{
		//�յ���������
		if(g_tModule.Gateway_RevFlag)  
		{	
			if(g_tModule.Gateway_BufLen>0){//��ֹ�տ������ڽ��վͽ���IDLE�жϵ��´���
					debug_printf("rev:");
					SendDataAsHex(g_tModule.Gateway_RevBuf,g_tModule.Gateway_BufLen);//�������				
					gateway_receive_command(g_tModule.Gateway_RevBuf,g_tModule.Gateway_BufLen);//��Ӧ������ִ�в���	
			}
			g_tModule.Gateway_RevFlag = 0;
//			HAL_UART_Receive_DMA(&huart2,g_tModule.Gateway_RevBuf,GATEWAY_SIZE_MAX);//�ӻ����¿���DMA����
 
		}
		
			// ����Ƿ���Ҫ�������������������
//			if (need_to_send_command()) 
//			if(waiting_for_response==0)//��һ����������
			{
					send_command_with_wait();
			}
}




