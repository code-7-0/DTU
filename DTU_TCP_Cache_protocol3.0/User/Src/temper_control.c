#include "temper_control.h"  
 
/*******不同命令字的解析函数*******/
///////////模块主动发送/////////////

//允许子设备入网0x01
//mqtt解析-入栈-出栈-构造命令—上报mqtt-等待0x03 60s-未收到发送0x02 							
// 关闭子设备入网 0x02
 //mqtt解析-入栈-出栈-构造命令
//子设备删除 0x04
//mqtt解析-入栈-出栈-构造命令-删除链表-mqtt构建
//状态查询 0x05
//入栈-出栈-构造命令 
//命令下发 0x06
//mqtt解析-入栈-出栈-构造命令 
//查询子设备列表 0x0A
//入栈-出栈-构造命令-轮询检查链表

///////////网关主动发送/////////////
//子设备添加 0x03
//解析命令-增加链表-mqtt构建,0x01计时清零
//状态上报 0x07
//解析命令-修改链表-（mqtt定时构建）
//获取本地时间 0x08
//mqtt时间查询报文-回复构建
//本地删除子设备 0x09
//解析命令-回复构建-mqtt构建-删除链表
//上报子设备在线状态 0x0B
//解析命令-修改链表-mqtt构建
//获取网络状态 0x0C
//解析命令- 
//远程升级 0x0D

/*******mqtt报文构建*******/


/*******mqtt报文解析*******/


/*******错误处理函数*******/

//0   1   2   3    4
//[]  []      []  []
/*******datapoint构造与解析*******/
typedef enum {
    RAW = 0x00,
    BOOL = 0x01,
    INT = 0x02,
    STRING = 0x03,
    ENUM = 0x04,
    BITMAP = 0x05
} DatapointType;

typedef struct {
    uint8_t dpid;     // 数据点序号
    uint8_t type;  // 数据类型
    uint8_t len;     // 数据长度
//	  uint16_t value;
    void * value;      // 指向数据值的指针
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
    if (device == NULL) return NULL; // 设备未找到
//		DeviceData *device_data=&(device->device_info.device_data); 
	
		//子设备id
		buffer[index++]=4;
		memcpy(&buffer[index],sub_id,4);
		index+=4;
		//改变的数据
		for (int j = 0; j < TOTAL_DP_COUNT-1; j++) {
					buffer[index++] = datapoints[j].dpid;
					buffer[index++] = datapoints[j].type;
					buffer[index++] = datapoints[j].len ;

					switch (datapoints[j].dpid) {
							case 1: // 开关
									buffer[index++]=device_data->sw;  
									break;
							case 2: // 模式
									buffer[index++]=device_data->mode;  
									break;
							case 16: // 设定温度
									buffer[index++]=(device_data->temp_set)>>8 ;  
									buffer[index++]=(device_data->temp_set)&0xFF;  
									break;
//							case 24: // 当前温度
//									buffer[index++]=(device_data->temp_current)>>8 ;  
//									buffer[index++]=(device_data->temp_current)&0xFF; 						
//									break;
							default:
									// 忽略其他数据点
									break;
					}
		}
   
		// 分配缓冲区 、入栈
		TemperCommand temper_send = {0, 0, NULL};
		temper_send.command=command;
		temper_send.param_length=index;  
    temper_send.params = (uint8_t*)malloc(index);
    if (temper_send.params != NULL) {
        // 复制sub_id到参数区
        memcpy(temper_send.params, buffer, index); 
    }		
		 // 将命令放入发送队列
    TemperFiFoPush(&temper_send);
    return index;  // 返回构建的数据长度
}
//uint8_t* prepare_datapoint(const uint8_t* dpids, uint8_t num_dpids, uint8_t* sub_id) {
//    uint16_t index = 0;
//		uint8_t buffer[20];
//		//子设备id
//		buffer[index++]=4;
//		memcpy(&buffer[index],sub_id,4);
//		index+=4;
//		//改变的数据
//    for (int i = 0; i < num_dpids; i++) {
//        uint8_t dpid = dpids[i];
//        for (int j = 0; j < TOTAL_DP_COUNT; j++) {
//            if (datapoints[j].dpid == dpid) {
//                buffer[index++] = datapoints[j].dpid;
//                buffer[index++] = datapoints[j].type;
//                buffer[index++] = datapoints[j].len ;
//							
//                // 根据数据类型进行大端处理
//                switch (datapoints[j].type) {
//                    case BOOL:
//                    case ENUM:
//                    case RAW:
//                    case STRING:
//                        // 直接复制数据
//                        memcpy(&buffer[index], datapoints[j].value, datapoints[j].len);
//                        break;
//                    case INT:
//                    case BITMAP:
//                        // 大端方式复制整型数据
//                        for (int k = 0; k < datapoints[j].len; k++) {
//                            buffer[index + k] = ((uint8_t*)datapoints[j].value)[datapoints[j].len - 1 - k];
//                        }
//                        break;
//                }
//                index += datapoints[j].len;
//                break;  // 找到对应的dpid后就可以跳出循环
//            }
//        }
//    }
//		
//	   // 分配缓冲区
//    uint8_t* buff = (uint8_t*)malloc(index);
//    if (buff != NULL) {
//        // 复制sub_id到参数区
//        memcpy(buff, sub_id, index); 
//    }		
//    return buff;  // 返回构建的数据长度
//}
 
/**
* @brief 为0x06命令下发命令准备datapoint数据
* @param datapoint - 数据点结构体
*				 buffer - 指向要填充的buffer的指针
* @return 填充后的数据长度
**/ 
//uint16_t prepare_datapoint(Datapoint *datapoint, uint8_t* buffer) {
//    uint16_t index = 0;
//    buffer[index++] = datapoint->dpid;      // 数据点序号
//    buffer[index++] = datapoint->type;      // 数据类型

//    // 大端序处理数据长度
//    buffer[index++] = (datapoint->len >> 8) & 0xFF;
//    buffer[index++] = datapoint->len & 0xFF;

//    // 根据数据类型填充value
//    switch (datapoint->type) {
//        case BOOL:
//        case ENUM:
//        case RAW:
//            memcpy(&buffer[index], datapoint->value, datapoint->len);
//            index += datapoint->len;
//            break;
//        case INT:
//            // 处理整型数据为大端
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
//    return index;  // 返回buffer中的数据长度
//}

//多个datapoint单元
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

// 功能：解析接收到的datapoints数据
// 参数：buffer - 接收到的数据缓冲区
//       length - 缓冲区的长度
//       datapoints - 指向已分配足够空间的datapoint数组的指针
//       max_count - 可以存储的最大datapoint数量
// 返回：解析的datapoint数量
//uint8_t parse_07_datapoints(const uint8_t* buffer, uint16_t length, Datapoint* datapoints ) {
//    uint16_t index = 0;
//    uint8_t count = 0;

//    while (index < length /*&& count < max_count*/) {
//        if (index + 4 > length) {
//            break; // 防止缓冲区溢出
//        }
//        
//        datapoints[count].dpid = buffer[index++];
//        datapoints[count].type = buffer[index++];
//        datapoints[count].len = (buffer[index] << 8) | buffer[index + 1];
//        index += 2;

//        if (index + datapoints[count].len > length) {
//            break; // 防止缓冲区溢出
//        }

//        datapoints[count].value = malloc(datapoints[count].len);
//        if (datapoints[count].value == NULL) {
//            break; // 内存分配失败
//        }

//        memcpy(datapoints[count].value, &buffer[index], datapoints[count].len);
//        index += datapoints[count].len;
//        count++;
//    }

//    return count; // 返回解析的datapoint数量
//}



// 解析数据并更新设备状态
uint8_t parse_07_datapoints( uint8_t* buffer, uint16_t length, LinkedList* device_list) {
    if (length < 4) return 0; // 至少需要子设备ID

//    uint8_t sub_id[4];
//    memcpy(sub_id, buffer, sizeof(sub_id));
    Device* device = findDevice(device_list, &buffer[1]);
    if (device == NULL) return 0; // 设备未找到
		DeviceInfo *device_info=&(device->device_info);
		device_info->online=1;//设备在线
	
    uint16_t index = 5; // 跳过子设备ID
    uint8_t count = 0;
		
    while (index < length) {
        if (index + 4 > length) break; // 防止缓冲区溢出

        uint8_t dpid = buffer[index++];
        uint8_t type = buffer[index++];
        uint16_t len = buffer[index++];
//        index += 2;

        if (index + len > length) break; // 防止缓冲区溢出

        switch (dpid) {
            case 1: // 开关
								device_info->device_data.sw = buffer[index];
                break;
            case 2: // 模式
                device_info->device_data.mode = buffer[index];
                break;
            case 16: // 设定温度
                device_info->device_data.temp_set = (buffer[index] << 8) | buffer[index + 1];
                break;
            case 24: // 当前温度
                device_info->device_data.temp_current = (buffer[index] << 8) | buffer[index + 1];
                break;
            default:
                // 忽略其他数据点
                break;
        }
        index += len;
        count++;
    }

    return count; // 返回更新的数据点数量
}

//基本命令结构（发送、接受）
void base_command_construct(uint8_t *frame,TemperCommand* temper_send) {
//    uint8_t frame[256]; // 帧数组，大小根据最大可能的帧长度设定
    uint16_t index = 0;

    // 帧头
    frame[index++] = FRAME_HEADER >> 8;
    frame[index++] = FRAME_HEADER & 0xFF;

    // 版本号
    frame[index++] = VERSION;

    // 命令字
    frame[index++] = temper_send->command;

    // 数据长度
    frame[index++] = (temper_send->param_length) >> 8;
    frame[index++] = (temper_send->param_length) & 0xFF;

		if(temper_send->param_length>0){
    // 数据
			memcpy(&frame[index], temper_send->params, temper_send->param_length);
			index += temper_send->param_length;
		}
		
    // 校验和
    frame[index] = calculate_checksum(frame, index);
		index++;
    // 发送数据
    Gateway_Usart_Send(frame, index); 
}






/*******发送命令*******/
//void construct_04_delete_subdevice_param(uint8_t command,char* sub_id )
//{
//		TemperCommand temper_send={0,0,0};
//		uint8_t tp=0;
//		
//		temper_send.command=command;
//		
//  // 计算需要的缓冲区大小
//    temper_send.param_length = snprintf(NULL, 0, "{\"sub_id\":\"%s\",\"tp\":%d}", sub_id, tp) ; 
//		uint16_t BufferSize= temper_send.param_length+1;
//    char *commandBuffer = (char *)malloc(temper_send.param_length+1);

//    if (commandBuffer != NULL) {
//			    // 构建字符串
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
		
    //   sub_id 总是4个字符长，且tp为1个字节
    int sub_id_len = 4;   
    uint8_t tp = 0;       // 0:删除子设备，1：回复出厂设置

    // 总的参数长度 = sub_id的长度 + tp的大小
    temper_send.param_length = sub_id_len + 1;

    // 分配缓冲区
    temper_send.params = (uint8_t*)malloc(temper_send.param_length);
    if (temper_send.params != NULL) {
        // 复制sub_id到参数区
        memcpy(temper_send.params, sub_id, sub_id_len);
        // 设置tp的值
        temper_send.params[sub_id_len] = tp;
    }

    // 将命令放入发送队列
    TemperFiFoPush(&temper_send);
    // free(temper_send.params);	
		   
} 
//0x0A 
//uint8_t parse_0A_query_subdevice_list(uint8_t* data, size_t data_length) {
//    if (data == NULL || data_length < 2) { 
//        return 0;
//    }
//    
//    uint8_t offset = 0;// 当前读取位置 
//    uint8_t index = data[offset] & 0x7F;
//    uint8_t id_bit7 = (data[offset++] >> 7) & 0x01;// 第7位

//    uint8_t sub_num = data[offset++];//这包数据的子设备长度
//    if (offset + sub_num > data_length) return 0;

//    for (int i = 0; i < sub_num && offset < data_length; i++) {//offset防止越界
//        uint8_t sub_len = data[offset++];
//        if (sub_len > 18 || offset + sub_len > data_length) {
//            return 0; // 校验数据长度，防止越界
//        }
//        char *sub_id = malloc(sub_len + 1); // 动态分配
//        if (sub_id == NULL) return 0; // 内存分配失败
//        memcpy(sub_id, &data[offset], sub_len);
//        sub_id[sub_len] = '\0'; // null-terminated string
////        printf("Sub-device %d: ID = %s\n", i + 1, sub_id);
//        offset += sub_len;
//        free(sub_id);
//    }
//    return 1;
//}
//0X0A命令相关
 // 遍历解析到的设备ID，添加不存在的设备
uint8_t addMissingDevices(uint8_t** current_sub_ids, uint8_t sub_num, LinkedList* list) {
		uint8_t state=0;
    for (uint8_t i = 0; i < sub_num; i++) {
        if (findDevice(list, current_sub_ids[i]) == NULL) {
					//// mqtt上报设备添加
							MqttTopicFiFoPush(ADD,current_sub_ids[i]);
//						uint8_t temp[4];
//						for(uint8_t j=0;j<4;j++)//大端模式转小端模式
//								temp[j] = current_sub_ids[i][3-j];
						//链表添加 
            if(addDevice(list, (DeviceBaseInfo*)current_sub_ids[i])!=NULL)
							state=1;
        }
    }
		return state;
}

// 删除链表中不存在于当前设备ID列表的设备
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
						//// mqtt上报设备删除 DELETE、sub_id  
						MqttTopicFiFoPush(DELETE,temp->device_info.device_base_info.sub_id);	
						//链表删除
            Device* toDelete = temp;
            if (prev == NULL) {
                list->head = temp->next; // 删除头节点
                temp = list->head;
            } else {
                prev->next = temp->next; // 删除中间或尾部节点
                temp = prev->next;
            }
						
            if (toDelete == list->tail) {
                list->tail = prev; // 更新尾指针
            }
            free(toDelete); // 释放内存
						debug_printf("delete device!");
						
        } else {
            prev = temp;
            temp = temp->next;
        }
    }
}

//查询频率？
//解析子设备列表响应、检查数据链表后修改
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
		
		//链表中添加新增的设备
    uint8_t state=addMissingDevices(current_sub_ids, sub_num, list);
		//链表中删除多余的设备
    removeExtraDevices(current_sub_ids, sub_num, list);

    for (int i = 0; i < sub_num; i++) free(current_sub_ids[i]);
    free(current_sub_ids);

		//mqtt上报所有设备号
		MqttTopicFiFoPush(IDLIST,NULL);
		//有新增的设备 发送05命令查询状态
		if(state==1)
			construct_send_command_to_fifo(0x05,NULL,NULL);
    return 1;
}
//构造发送命令到fifo中
void construct_send_command_to_fifo(uint8_t command, uint8_t* sub_id,DeviceData *device_data)
{ 
	
		switch(command)
		{
			case CMD_DELETE_SUBDEVICE:// 0x04子设备删除 
				//根据id构造删除字符串
				construct_04_delete_subdevice_param(command,sub_id); 
				break;			 
			case CMD_ISSUE_COMMAND:// 0x06命令下发
				//id 、参数名称、数值 
				construct_06_cmd_param(command, &Linked_List,sub_id,device_data);
				break;
			default:
			{
			//允许子设备入网 0x01 
			//关闭子设备入网 0x02 
			//   命令下发   0x05 			
			//查询子设备列表 0x0A 	
				if(Linked_List.head!=NULL || command!=CMD_QUERY_STATUS)//0x05在链表不为空才入栈
				{
					TemperCommand temper_send={0,0,0};//入栈
					temper_send.command=command; 
					TemperFiFoPush(&temper_send);		
				}					
			}			
				break;
			
		}   
}


char* extract_delete_sub_id(char *json) {
    const char *key = "\"sub_id\":\"";  // 查找的关键字
    char *start, *end;

    // 查找 key 在 json 字符串中的位置
    start = strstr(json, key);
    if (start != NULL) {
        start += strlen(key);  // 移动指针到 key 后的内容
        end = strchr(start, '"');  // 查找结束引号
        if (end != NULL) {
						*end='\0';
						return start;
//            memcpy(output, start, end - start);  // 提取内容到输出数组
//            output[end - start] = '\0';  // 添加字符串结束符
        } else {
						return NULL;
//            output=NULL;  // 未找到结束引号
        }
    } else {
						return NULL;
//        output=NULL;// 未找到关键字
    }
}


uint8_t waiting_for_response = 0;  
uint8_t unsolicited_message_flag=0;
uint8_t expected_command=0;//期待回应的
uint16_t response_timeout_cnt;
TemperCommand	response_command;
//uint8_t delete_id[4];//要删除的id的字符串地址
//#define MAX_RETRIES 3
//#define RESPONSE_TIMEOUT 200
enum{
	SEND_IDLE=0,
	SEND_CONSTRUCT,
	SEND_UNSOLICITED,
	SEND_WAIT
};
//char sub_id[18];

//处理回复的命令
uint8_t add_device_time_cnt=0;
void process_command_response(TemperCommand *ask,TemperCommand *response) { 
 
    switch (response->command) {
				case CMD_ALLOW_JOIN_NETWORK:// 01入网
				{
					//60s等待设备添加0x03计时
					add_device_time_cnt=60; 
					// mqtt上报:入网成功   
					MqttTopicFiFoPush(NETIN,NULL);						
				}
				break;			
				case CMD_DELETE_SUBDEVICE:// 04子设备删除
				{					
					//删除数据链表  
					deleteDevice(&Linked_List,ask->params); 
				//删除成功，mqtt上报   mqtt上报队列参数：action、id 
					MqttTopicFiFoPush(DELETE,ask->params);					
				}
				break;
				
        case CMD_REPORT_STATUS:// 0x07回应状态查询0x05
				{
            // 解析状态查询响应 、更新数据链表
						parse_07_datapoints(response->params,response->param_length,&Linked_List);
						//  缩短上报时间
						if(temper_rapif_reply_flag==1)
								Temper_Rapid_Reply();
				}
        break;
        case CMD_QUERY_SUBDEVICE_LIST:// 查询子设备列表0x0A
            // 解析子设备列表响应、检查数据链表后修改
						parse_0A_query_subdevice_list(response->params,response->param_length,&Linked_List);
  
            break; 
        default:
					//其他命令只校验基本结构
            break;
    }
}












/*******处理回复命令*******/

//uint8_t parse_03_add_subdevice(const char* jsonString, DeviceBaseInfo* info) {
//    cJSON *json = cJSON_Parse(jsonString);
//    if (json == NULL) {
//        const char *error_ptr = cJSON_GetErrorPtr();
//        if (error_ptr != NULL) {
////            fprintf(stderr, "Error before: %s\n", error_ptr);
//        }
//        return 0;
//    }

//    // 解析产品类型
////    const cJSON *pk_type = cJSON_GetObjectItemCaseSensitive(json, "pk_type");
////    if (cJSON_IsNumber(pk_type)) {
////        info->pk_type = (uint8_t)pk_type->valueint;
////    }

//    // 解析子设备标识
//    const cJSON *sub_id = cJSON_GetObjectItemCaseSensitive(json, "sub_id");
//    if (cJSON_IsString(sub_id) && (sub_id->valuestring != NULL)) {
//        strncpy(info->sub_id, sub_id->valuestring, sizeof(info->sub_id) - 1);
//        info->sub_id[sizeof(info->sub_id) - 1] = '\0'; // Ensure null-termination
//    }

////    // 解析产品标识
////    const cJSON *pid = cJSON_GetObjectItemCaseSensitive(json, "pid");
////    if (cJSON_IsString(pid) && (pid->valuestring != NULL)) {
////        strncpy(info->pid, pid->valuestring, sizeof(info->pid) - 1);
////        info->pid[sizeof(info->pid) - 1] = '\0'; // Ensure null-termination
////    }

////    // 解析版本号
////    const cJSON *ver = cJSON_GetObjectItemCaseSensitive(json, "ver");
////    if (cJSON_IsString(ver) && (ver->valuestring != NULL)) {
////        strncpy(info->ver, ver->valuestring, sizeof(info->ver) - 1);
////        info->ver[sizeof(info->ver) - 1] = '\0'; // Ensure null-termination
////    }

////    // 解析OTA支持
////    const cJSON *ota = cJSON_GetObjectItemCaseSensitive(json, "ota");
////    if (cJSON_IsNumber(ota)) {
////        info->ota = (uint8_t)ota->valueint;
////    }

//    cJSON_Delete(json);
//		return 1;
//}
void parse_03_add_subdevice(const uint8_t* data, DeviceBaseInfo* info){
    // 根据结构体字段依次提取数据
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
 
// 解析0x0B命令的JSON数据，并更新设备状态
//uint8_t parse_0B_report_online(char* jsonString, LinkedList* list) {
//    cJSON* json = cJSON_Parse(jsonString);
//    if (json == NULL) {
//        const char* error_ptr = cJSON_GetErrorPtr();
//        if (error_ptr != NULL) {
////            fprintf(stderr, "Error in parseDeviceStatus: %s\n", error_ptr);
//        }
//        return 0; // 解析失败
//    }

//    cJSON* jsonSubId = cJSON_GetObjectItemCaseSensitive(json, "sub_id");
//    cJSON* jsonStatus = cJSON_GetObjectItemCaseSensitive(json, "status");

//    if (!cJSON_IsString(jsonSubId) || jsonSubId->valuestring == NULL ||
//        !cJSON_IsNumber(jsonStatus)) {
//        cJSON_Delete(json);
//        return 0; // 数据不完整
//    }

//    // 查找设备
//    Device* device = findDevice(list, jsonSubId->valuestring);
//    if (device != NULL) {
//        // 更新设备的在线状态
//        device->device_info.onlinestate = (uint8_t)jsonStatus->valueint;
////        printf("Updated device %s online status to %d\n", jsonSubId->valuestring, device->device_info.onlinestate);
//    } else {
////        printf("Device not found: %s\n", jsonSubId->valuestring);
//    }

//    cJSON_Delete(json);
//    return 1; // 成功
//} 
uint8_t parse_0B_report_online(LinkedList* list,uint8_t* data, uint16_t length) {
    if (data == NULL || length==0) {
        return 0; // 输入指针无效
    } 
		uint8_t status=data[length-1];//在线状态
    // 查找设备 
		uint8_t data_temp[4];
		if(status==1){
			memcpy(data_temp,data,4); 
		}else{ 
			for(uint8_t i=0;i<4;i++)//小端模式转大端模式
				data_temp[i] = data[3-i];				 
		}
		Device* device= findDevice(list, data_temp);
    if (device != NULL) {
        // 更新链表在线状态
        device->device_info.online  = status;//在线状态
 
			//构建mqtt报文 action:ONLINE,sub_id 
				MqttTopicFiFoPush(ONLINE,data_temp);				
    }  
		
    return 1; // 成功
} 
//解析0x09删除本地设备的命令
//uint8_t	parse_09_delete_local_device(char* jsonString)
//{
//		cJSON* json = cJSON_Parse(jsonString);
//    if (json == NULL) { // 解析错误
//        const char* error_ptr = cJSON_GetErrorPtr();
//        if (error_ptr != NULL) {
////            fprintf(stderr, "Error before: %s\n", error_ptr);
//        }
//        return 0; // 返回0表示失败
//    }

//    // 提取删除的子设备ID
//    cJSON* jsonSubId = cJSON_GetObjectItemCaseSensitive(json, "sub_id");
//    if (cJSON_IsString(jsonSubId) && (jsonSubId->valuestring != NULL)) { 
//        memcpy(delete_id, jsonSubId->valuestring, 4); 
//    } else {
//        cJSON_Delete(json);
//        return 0; // 返回0表示失败
//    }

//    cJSON_Delete(json);
//    return 1; // 返回1表示成功
//}


void construct_09_delete_local_device(uint8_t command,uint16_t length)
{ 
    TemperCommand temper_send = {0, length, NULL}; // 使用传入的length
    temper_send.command = command; 
		uint8_t data=0;
		temper_send.params=&data;
		base_command_construct(g_tModule.Gateway_SendBuf,&temper_send);
} 
void construct_08_local_time(uint8_t command,uint16_t length,DateTime* time)
{ 
    TemperCommand temper_send = {0, length, NULL}; // 使用传入的length
    temper_send.command = command; 
		uint8_t data[]={1,datetime.year,datetime.month,datetime.day,datetime.hour,
										datetime.min,datetime.sec,datetime.week};
//		temper_send.params=(uint8_t*)time;
		temper_send.params=data;
		base_command_construct(g_tModule.Gateway_SendBuf,&temper_send);
} 

void construct_0c_net_status(uint8_t command,uint16_t length,uint8_t status)
{ 
    TemperCommand temper_send = {0, length, NULL}; // 使用传入的length
    temper_send.command = command;  
		temper_send.params=(uint8_t*)&status;
		base_command_construct(g_tModule.Gateway_SendBuf,&temper_send);
} 
void construct_nomarl_param(uint8_t command,uint16_t length)
{ 
    TemperCommand temper_send = {0, length, NULL}; // 使用传入的length
    temper_send.command = command; 
		base_command_construct(g_tModule.Gateway_SendBuf,&temper_send);
} 


void process_unsolicited_message(TemperCommand *tc) {   
    switch (tc->command) {
        case CMD_ADD_SUBDEVICE://03子设备添加
				{ 
						//解析子设备数据  
//						DeviceBaseInfo  device_base_info;
//						parse_03_add_subdevice(tc->params, &device_base_info);
//					
//						//链表增加
////						Device* device_addr=addDevice(&Linked_List,&device_base_info); 
//						addDevice(&Linked_List,&device_base_info);
						// 发送确认消息  
						construct_nomarl_param(tc->command,  0); 
					
						uint8_t *id_addr=tc->params+1;
						uint8_t sub_id[4];
						for(uint8_t i=0;i<4;i++)//小端模式转大端模式
							sub_id[i] = id_addr[3-i];							
						if(addDevice(&Linked_List,(DeviceBaseInfo*)sub_id)!=NULL){ 
								//构建mqtt报文（添加的id需要存储）  action、sub_id 
								MqttTopicFiFoPush(ADD,sub_id);
						}
						add_device_time_cnt=0;//0x01等待计时清零 
				}
					break; 
        case CMD_REPORT_STATUS://0x07状态上报   无需回复
				{
					//解析状态参数、修改链表
						parse_07_datapoints(tc->params,tc->param_length,&Linked_List); 
						//缩短上报时间
						if(temper_rapif_reply_flag==1)
							Temper_Rapid_Reply();
				}				
					break;
        case CMD_GET_LOCAL_TIME://0x08获取本地时间    
				{
					//回复时间数据(时间构造函数)
						construct_08_local_time(tc->command,8,&datetime); 
					break;	
				}
        case CMD_DELETE_LOCAL_SUBDEVICE://0x09本地删除子设备  
				{
						//解析删除命令  
//						construct_09_delete_local_device(tc->command,1);//回复删除成功 
						//构建mqtt报文 action:DELETE,sub_id 
						MqttTopicFiFoPush(DELETE,tc->params);							
						//发送报文后 删除链表
						deleteDevice(&Linked_List,tc->params);	 
				}
					break;		
				case CMD_REPORT_SUBDEVICE_ONLINE_STATUS:// 0x0B上报子设备在线状态
				{
						//解析命令 、修改链表 、 构建mqtt报文
						if(parse_0B_report_online(&Linked_List, tc->params,tc->param_length))
									construct_nomarl_param(tc->command,0);// 发送确认消息  
				} 
					break;
				case CMD_GET_NETWORK_STATUS:// 0x0C上报网络状态
				{
						//回复网络状态
						construct_0c_net_status(tc->command,1,5); 
				} 
					break;
        default: 
            break;
    }
}

 
 
//处理接收到的数据
void gateway_receive_command(uint8_t* rx_buffer,uint16_t len) { 
//		expected_command = 0;
//		response_timeout_cnt=0;
    uint16_t received_header = (rx_buffer[0] << 8) | rx_buffer[1];
		// 校验帧头
    if (received_header != FRAME_HEADER) { 
        return;  
    }

    uint8_t version = rx_buffer[2];
    uint8_t command = rx_buffer[3];
    uint16_t data_length = (rx_buffer[4] << 8) | rx_buffer[5]; 
		if(len<(data_length+HEADER_SIZE+1))//长度检查
				return;
		// 校验和检查
    uint8_t checksum = calculate_checksum(rx_buffer, data_length + HEADER_SIZE);
    if (checksum != rx_buffer[HEADER_SIZE + data_length]) { 
        return;  
    }
		
		setTemperCommand(&response_command,command,data_length, (rx_buffer+HEADER_SIZE));
    // 如果我们正在等待响应且接收的命令符合预期
    if (waiting_for_response && command == expected_command) {
//        process_command_response(command, rx_buffer + HEADER_SIZE, data_length);
        waiting_for_response = 0;//清零等待标志位
//				expected_command = 0;
//				response_timeout_cnt=0;
    } else {
        // 处理非响应的上发数据
				setTemperCommand(&response_command,command,data_length, (rx_buffer+HEADER_SIZE));
//				unsolicited_message_flag=1;
				process_unsolicited_message(&response_command); 
//        process_unsolicited_message(command,rx_buffer+ HEADER_SIZE, data_length);
    }
 	
//		//处理命令
//		switch (command) {
//				case CMD_ALLOW_JOIN_NETWORK://0x01允许子设备入网
//						handle_status_query(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_STOP_JOIN_NETWORK://0x02停止子设备入网
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_ADD_SUBDEVICE://0x03子设备添加
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_DELETE_SUBDEVICE://0x04子设备删除
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_QUERY_STATUS://0x05状态查询
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_ISSUE_COMMAND://0x06命令下发
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_REPORT_STATUS://0x07状态上报
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_GET_LOCAL_TIME://0x08获取本地时间
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_DELETE_LOCAL_SUBDEVICE://0x09本地删除子设备
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_QUERY_SUBDEVICE_LIST://0x0A查询子设备列表
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_REPORT_SUBDEVICE_ONLINE_STATUS://0x0B上报子设备在线状态
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;
//				case CMD_GET_NETWORK_STATUS://0x0C获取网络状态
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;		
//				case CMD_REMOTE_UPGRADE://0x0D远程升级
//						handle_device_add(rx_buffer + HEADER_SIZE, data_length);
//						break;						
//				// 添加其他命令的处理		
//		}
}

//发送命令并等待响应
void send_command_with_wait(void) { 
		static uint8_t  MAX_RETRIES=3;
		static uint16_t RESPONSE_TIMEOUT=200;
		static uint8_t send_state=SEND_IDLE,retry_count=0;
		static TemperCommand temper_send;//保存出栈指针
		switch (send_state)
		{
			case SEND_IDLE://等待队列新命令
			{
					//解析mqtt或debug命令后，构造temper_send，push进fifo
					if(TemperFiFoSize()>0)//存在待发送命令 	
					{
						TemperFiFoPop(&temper_send);//出栈
						send_state=SEND_CONSTRUCT; 
						expected_command=temper_send.command; 
						
						//设置标志位
						if(temper_send.command==CMD_QUERY_STATUS){//发送0X05回复0x07
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
			case SEND_CONSTRUCT://构建命令
			{   
						//构建、发送命令
						base_command_construct(g_tModule.Gateway_SendBuf,&temper_send);  
						//设置标志位
						response_timeout_cnt=RESPONSE_TIMEOUT;//等待回复时间			
						waiting_for_response = 1;//置位等待标志位
						send_state=SEND_WAIT;  
			}
			break;
			case SEND_WAIT://等待命令回复
			{
					if(waiting_for_response==0)//超时或者得到回复
					{
						if(response_timeout_cnt>0)//200ms内得到回应
						{
								//处理接收到的命令
								process_command_response(&temper_send,&response_command);
								//标志位处理
								response_timeout_cnt=0;
								expected_command=0;
								send_state=SEND_IDLE;
						}
						else//超时未回应，重发
						{ 
								retry_count++; 
								if (retry_count >= MAX_RETRIES) {//重传次数超过最大值
										send_state = SEND_IDLE; 
//												expected_command=0; 
								} else {
										send_state = SEND_CONSTRUCT;
								}								
						}
						
						if(send_state==SEND_IDLE){//重试失败或成功回复，都释放占用的空间
								if(temper_send.param_length > 0 && temper_send.params!=NULL){
									retry_count = 0;
									expected_command=0; 
									free(temper_send.params);//释放数据
									
								}
						}
							
					} 
			}	
			break;
			case SEND_UNSOLICITED://处理非响应的上发数据
					process_unsolicited_message(&response_command); 
					unsolicited_message_flag = 0; // 处理完毕后重置标志位
					send_state = SEND_IDLE; // 返回空闲状态等待新的任务
					break;			
			default:
				send_state = SEND_IDLE;
				break;
		}
}



void Handle_Temper_Crontrol(void)	
{
		//收到主机数据
		if(g_tModule.Gateway_RevFlag)  
		{	
			if(g_tModule.Gateway_BufLen>0){//防止刚开启串口接收就进入IDLE中断导致错误
					debug_printf("rev:");
					SendDataAsHex(g_tModule.Gateway_RevBuf,g_tModule.Gateway_BufLen);//调试输出				
					gateway_receive_command(g_tModule.Gateway_RevBuf,g_tModule.Gateway_BufLen);//回应主机，执行操作	
			}
			g_tModule.Gateway_RevFlag = 0;
//			HAL_UART_Receive_DMA(&huart2,g_tModule.Gateway_RevBuf,GATEWAY_SIZE_MAX);//从机重新开启DMA接收
 
		}
		
			// 检查是否需要发送命令或处理其他任务
//			if (need_to_send_command()) 
//			if(waiting_for_response==0)//上一个命令处理完毕
			{
					send_command_with_wait();
			}
}




