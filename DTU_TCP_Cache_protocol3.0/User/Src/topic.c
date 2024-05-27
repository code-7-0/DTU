 #include "topic.h"
//1.待发送标识；2.周期(单位0.5秒)；3.标识符；4.数据；
struct Topic_dat   Topic_data[2] = 
{				//定时命令
				 {GASPOWER_DAT_UPLOAD_DELAY,0,"ALL_DAT"},//,"ABCDEF"
				{TEMPER_DAT_UPLOAD_DELAY,0,"status"}
};
//1.待发送标识；2.标识符；3.数据；
struct Topic_err  Topic_error[] = 
{
				//故障或者报警，不作为定时命令启用
				 "E1","0",			//20点火失败故障
				 "E2","0",			//21残焰、假火
				 "E3","0",			//22限温器故障(机械限温器断开)
				 "E4","0",			//23火焰4分钟内熄火5次（着火后火焰检测有效后又熄火）
				 "E6","0",			//24温度不上升或传感器脱落故障(点火燃烧持续30s后,采暖传感器和卫浴水传感器温度变化不超过2℃)
				 "E7","0",			//25防冻故障≤3℃
				 "F1","0",			//26风压故障
				 "F2","0",			//27缺水或水压开关故障（水泵启动前检测，检测失败不启动水泵，连续5s检测失败报警。）
         "F3","0",			//28采暖温度传感器故障（采暖NTC开路、短路或连线松脱、短路）
				 "F4","0",			//29卫浴水温度传感器故障（卫浴水NTC开路、短路或连线松脱、短路)
				 "F5","0",			//30温度传感器超温故障
				 "F0","0",			//
				 "F0","0",			//
				 "F0","0",			//
				 "F0","0",			//
				 "F0","0",			//
				 "F0","0",			//
				 "F0","0",			//
				 "F0","0",			//
				 "F0","0"				//
};

uint32_t send_flag;
//FaultInfo faults = {0,0,0,0};			//设备故障告警计数
//uint8_t topic_send_wait=1;			//待发送命令直针
uint8_t topic_send_wait=0xFF;//0;			//待发送命令序号，初始发布固件主题
uint8_t topic_send_last = 0; //已发送命令直针
uint8_t topic_send_ok = 1;			  //拼装命令标识
char message_buf[MESSAGE_BUF_SIZE];//5.69个可变设备
char topic_buf[256];		
			
uint32_t  topic_count = 0;			  //命令序号
uint8_t topic_update_flag = 0;  //每秒更新一次标志				


// 检查有多少个未发送命令
//uint8_t  topic_check_list(void)
//{
//    uint8_t REdat,i;
//	
//	  REdat = 0;
//	  for(i=0;i<TOPIC_DAT_NUM;i++)
//		{
//			REdat += Topic_data.send_flag;
//	  }
//		return REdat;
//}

void clear_buffer(char* buffer, size_t size) {
    memset(buffer, 0, size);
}

void append_int_as_str(char* buffer, int value) {
    char temp[5]; // 足够存放int最大值和'\0'
    sprintf(temp, "%d", value);
    strcat(buffer, temp);
}
char   QCCID[21];

// 将 sub_id 转换为字符串
void sub_id_to_str(uint8_t sub_id[4], char* sub_id_str) {
    sprintf(sub_id_str, "%02X%02X%02X%02X", sub_id[0], sub_id[1], sub_id[2], sub_id[3]);
}
 

// 构建 IDLIST JSON 字符串
void build_idlist_json(cJSON* root,LinkedList* list) {
    //cJSON* root = cJSON_CreateObject();
    cJSON* params = cJSON_CreateArray();
    cJSON_AddStringToObject(root, "action", "idlist");
    cJSON_AddItemToObject(root, "params", params);

    Device* current = list->head;
    while (current != NULL) {
        cJSON* device_id = cJSON_CreateObject();
        char sub_id_str[9] = {0};
        sub_id_to_str(current->device_info.device_base_info.sub_id, sub_id_str);

        cJSON_AddStringToObject(device_id, "sub_id", sub_id_str);
        cJSON_AddItemToArray(params, device_id);

        current = current->next;
    }

 
}


// 构建 ADD、DELETE JSON 字符串
void build_add_del_json(TemperMqttTopic* temper_topic_send, cJSON* root,LinkedList* list) {
//    Device* device = findDevice(list, temper_topic_send->sub_id);
//    if (device==NULL) { 
//        return ;
//    }
    char sub_id_str[9] = {0}; // 8 个字符 + 终止符
    sub_id_to_str(temper_topic_send->sub_id, sub_id_str);		
		
		if(temper_topic_send->command==ADD)
			cJSON_AddStringToObject(root, "action", "add");
		else if(temper_topic_send->command==DELETE)
			cJSON_AddStringToObject(root, "action", "delete");
		
		cJSON* params = cJSON_CreateObject();
		cJSON_AddStringToObject(params, "sub_id", sub_id_str);
		cJSON_AddItemToObject(root, "params", params);

}

// 构建 ONLINE JSON 字符串
void build_online_json(uint8_t* sub_id, cJSON* root,LinkedList* list) {
    Device* device = findDevice(list, sub_id);
    if (device==NULL) { 
        return ;
    } 
		
    cJSON_AddStringToObject(root, "action", "online");

    cJSON* params = cJSON_CreateObject(); 
    char sub_id_str[9] = {0};
    sub_id_to_str(device->device_info.device_base_info.sub_id, sub_id_str);

    cJSON_AddStringToObject(params, "sub_id", sub_id_str);
    cJSON_AddNumberToObject(params, "Online", device->device_info.online); 
    cJSON_AddItemToObject(root, "params", params);
 
}
 
//手动移除空格的函数
void remove_spaces(char* str) {
    char* i = str;
    char* j = str;
    while (*j != '\0') {
        if (*j != ' ' && *j != '\n' && *j != '\r' && *j != '\t') {
            *i = *j;
            i++;
        }
        j++;
    }
    *i = '\0';
}

 
// 构建 JSON 字符串
uint8_t build_temper_json(char* message_buf,uint16_t buff_size,TemperMqttTopic* temper_topic_send) {
    cJSON* root = cJSON_CreateObject(); 

    switch (temper_topic_send->command) {
        case NETIN:
            cJSON_AddStringToObject(root, "action", "networkIn");
            break;
        case ADD: 
        case DELETE:
            build_add_del_json(temper_topic_send,root,&Linked_List);
            break; 
        case ONLINE:
            build_online_json(temper_topic_send->sub_id,root,&Linked_List);
            break;
        case IDLIST:
            build_idlist_json(root,&Linked_List);
            break;
        default:
//            printf("Unknown command\n");
            cJSON_Delete(root);
            return NULL;
    }
		//char* json_string = cJSON_Print(root);//动态分配
    uint8_t success = cJSON_PrintPreallocated(root, message_buf ,buff_size, 1);
    if (success) {
        remove_spaces(message_buf); // 移除空格
    }		
//    char* json_string = cJSON_PrintUnformatted(root);//无空格
//    if (json_string == NULL) {
//        cJSON_Delete(root);
//        return 0;
//    }
    cJSON_Delete(root);
    return success; 
}

// 构建 STATUS JSON 字符串
uint8_t build_status_json(char* message_buf,uint16_t buff_size,LinkedList* list) {
    cJSON* root = cJSON_CreateObject();
    cJSON* params = cJSON_CreateArray();
    cJSON_AddStringToObject(root, "action", "status");
    cJSON_AddItemToObject(root, "params", params); 
	
    Device* current = list->head;
    while (current != NULL) {
        cJSON* device_status = cJSON_CreateObject();
        char sub_id_str[9] = {0};
        sub_id_to_str(current->device_info.device_base_info.sub_id, sub_id_str);

        cJSON_AddStringToObject(device_status, "sub_id", sub_id_str);

        cJSON_AddNumberToObject(device_status, "temp_set", current->device_info.device_data.temp_set / 10.0);
        cJSON_AddNumberToObject(device_status, "temp_current", current->device_info.device_data.temp_current / 10.0);
      // 调试输出，检查 sw 和 mode 的值
      //debug_printf("Adding sw: %d, mode: %d\n", current->device_info.device_data.sw, current->device_info.device_data.mode);
				cJSON_AddNumberToObject(device_status, "sw", current->device_info.device_data.sw);
        cJSON_AddNumberToObject(device_status, "mode", current->device_info.device_data.mode);


        cJSON_AddItemToArray(params, device_status);
        current = current->next;
    }
 
		
    uint8_t success = cJSON_PrintPreallocated(root,message_buf , buff_size, 1);
    if (success) {
        remove_spaces(message_buf); // 移除空格
    }		
    cJSON_Delete(root);
    return success; 		
} 

void build_message(char* message_buf, uint8_t topic_send_wait, uint32_t topic_count,TemperMqttTopic* temper_topic_send) {
    // 初始化和清空缓冲区
    clear_buffer(message_buf, MESSAGE_BUF_SIZE);

		if(topic_send_wait==0xFF)
		{   
				char version[16]; // 用于存放版本号字符串
				// 生成版本号字符串
				sprintf(version, "v%d.%d.%d", VER_MAJOR, VER_MINOR, VER_REVISION);
				// 构造JSON字符串
				sprintf(message_buf, "{\"QCCID\": \"%s\", \"software_version\": \"%s\"}", QCCID, version);
			
		}
		else if(topic_send_wait < 13)
		{
				// 假设 Topic_data 和 Topic_error 的 identifier 和 dat 都是字符串类型
				const char* identifier = (topic_send_wait == 1) ? Topic_data[0].identifier : Topic_error[topic_send_wait - 2].identifier;
				const char* data = (topic_send_wait == 1) ? GasPower_Manage.ALL_DATA : Topic_error[topic_send_wait - 2].dat;

				// 使用 sprintf 一次性完成字符串构建
				// 注意：确保 message_buf 足够大，以避免溢出
				sprintf(message_buf, "{\"id\":\"5\",\"params\":{\"%s\":%s},\"SN\":\"%d\"}\r\n", identifier, data, topic_count);
		}else if(topic_send_wait==13){//上报设备数据
				build_status_json(message_buf,MESSAGE_BUF_SIZE,&Linked_List); 
				//debug_printf("%s",message_buf); 
//			topic_send_wait=0;
//			topic_send_ok=1;
		}else{  
				build_temper_json(message_buf,MESSAGE_BUF_SIZE,temper_topic_send);  
				//debug_printf("%s",message_buf);
//			topic_send_wait=0;
//			topic_send_ok=1;
		}
		
}

void build_topic(char* topic_buf, int topic_send_wait, int message_length) {
    // 初始化和清空缓冲区
    clear_buffer(topic_buf, 256);

    // 构建主题字符串的开头
    strcat(topic_buf, "AT+QMTPUBEX=0,0,0,0,\"");

    // 根据topic_send_wait的值选择主题
//		strcat(topic_buf, 
//					 (topic_send_wait == 1) ? Device_Flash.QMTSUB1 : 
//					 (topic_send_wait == 0xFF) ? Device_Flash.QMTSUB4 : Device_Flash.QMTSUB3);
////    strcat(topic_buf, (topic_send_wait == 1) ? Device_Flash.QMTSUB1 : Device_Flash.QMTSUB3);
    // 根据 topic_send_wait 的值选择主题
    if (topic_send_wait == 1) {
        strcat(topic_buf, Device_Flash.QMTSUB1);
    } else if (topic_send_wait == 0xFF) {
        strcat(topic_buf, Device_Flash.QMTSUB4);
    } else if (topic_send_wait >= 2 && topic_send_wait <= 13) {
        strcat(topic_buf, Device_Flash.QMTSUB3);
    } else if (topic_send_wait == 14 || topic_send_wait == 15) {
        strcat(topic_buf, Device_Flash.QMTSUB7);
    } else {
        strcat(topic_buf, Device_Flash.QMTSUB3); // 默认主题
    }    
	
	
		strcat(topic_buf, "\",");

    // 添加消息长度
    append_int_as_str(topic_buf, message_length);
    strcat(topic_buf, "\r\n");
}

 

//数据主动发送
void Handle_topic(void)
{
		uint8_t i; 
	  // 计时与数据更新，命令队列-推入//////////////////////////////////////////////////////////////////////////
	  if(topic_update_flag) //周期性进入 500mS一次
		{
				topic_update_flag = 0;
				for(i=0;i<2;i++){
						Topic_data[i].send_count++;
						 //计时更新
						 if(Topic_data[i].send_count>Topic_data[i].send_count_cycl)
						 {
								Topic_data[i].send_count = 0; 
							 if(i==0) 
								 send_flag|=0x01;
							 else {
								 if(Linked_List.head!=NULL)
										send_flag |=0x01<<12;
							 }
								//send_flag |= (i==0?0x01:(0x01<<12)); 
						 }
				}
		}
		// 故障报警
		if (GasPower_Manage.ERRO_CODE != error_code_pre) { // 判断故障代码是否发生变化
				// 故障清除
				if (GasPower_Manage.ERRO_CODE == 0) { 
						for (i = 0; i <= 10; i++) {
								if (Topic_error[i].dat[0] == '1') {
										send_flag |= 1 << (i + 1);
										strcpy(Topic_error[i].dat,"0");
								}
						}
				} else { // 故障发生
						// 确定i的值
						if(GasPower_Manage.ERRO_CODE==ERROR_FIRE) //E1点火失败故障
							i=0;
//					send_flag|=1<<1; 
//				  append_int_as_str(Topic_error[0].dat,1); 
//					break;
						else if(GasPower_Manage.ERRO_CODE==ERROR_BURN)  //E2 ERROR_BURN
							i=1;
						else if(GasPower_Manage.ERRO_CODE==ERROR_HIGH2) //E3 限温器故障
							i=2;
						else if(GasPower_Manage.ERRO_CODE==ERROR_OUT)  //E4 火焰4分钟内熄火5次
							i=3;
						else if(GasPower_Manage.ERRO_CODE==ERROR_HEAT) //E6 温度不上升或传感器脱落故障
							i=4;
						else if(GasPower_Manage.ERRO_CODE==ERROR_FREZ) //E7 防冻故障
							i=5;
						else if(GasPower_Manage.ERRO_CODE==ERROR_FAN) //F1 风压故障
							i=6;
						else if(GasPower_Manage.ERRO_CODE==ERROR_PRESS) //F2 缺水或水压开关故障
							i=7;
						else if(GasPower_Manage.ERRO_CODE==ERROR_TEMP2) //F3 采暖温度传感器故障
							i=8;
						else if(GasPower_Manage.ERRO_CODE==ERROR_TEMP1) //F4 卫浴水温度传感器故障
							i=9;
						else if(GasPower_Manage.ERRO_CODE==ERROR_HIGH) //F5 温度传感器超温故障
							i=10; 

						send_flag |= 1 << (i + 1); 
						strcpy(Topic_error[i].dat, "1");
				}
				error_code_pre = GasPower_Manage.ERRO_CODE; // 更新
		}		
		
		
		
		//命令队列-拉出///////////////////////////////////////////////////////////////////////////////
		 TemperMqttTopic temper_topic_send;
		if(topic_send_wait ==0 ) //待发送命令直针
		{
				if(MqttTopicFiFoSize()>0)//高优先级队列
				{ 
					MqttTopicFiFoPop(&temper_topic_send);
					topic_send_wait=15;
				}else{
					i = topic_send_last; //从上次命令
					if(send_flag>0)
					{
						do
						{
							 i++;
							
							 if(i > 13) i = 1; //循环
							
							if(send_flag & ( 1 << (i-1)) ) //检查是否有待发送命令
							 {
									topic_send_wait = i; //记录命令指针,>0
									send_flag &= ~( 1 << (i-1) ); //清除标志，只发送一次 
							 }
						}while((topic_send_wait == 0)&&(i != topic_send_last)); 	
					}
				}
		}		
//		static uint8_t first_flag=1;
    //拼装命令///////////////////////////////////////////////////////////////////////////////
    if((topic_send_wait!=0)&&(topic_send_ok==1)&&(qccid_get_flag==1))//有数据要上传同时上一条数据上传完毕
		{
					topic_send_ok = 0;  //等待发送标识
					build_message(message_buf, topic_send_wait, topic_count,&temper_topic_send); 
					int message_length = strlen(message_buf);
					build_topic(topic_buf, topic_send_wait, message_length); 
			
//					if(topic_send_wait>=14){
//						debug_printf("topic_buf:%s\r\n",topic_buf);			
//						debug_printf("message_buf:%s\r\n",message_buf);		
//					}
					
		}

}


//uint8_t Trigger_MQ_state = 0;// 0 发送

//从控制命令下发后，立即主动上报mqtt数据
void GasPower_Rapid_Reply(void)
{
			Topic_data[0].send_count = Topic_data[0].send_count_cycl-DAT_UPLOAD_DELAY_LIVE;  
}
void Temper_Rapid_Reply(void)
{
			Topic_data[1].send_count = Topic_data[1].send_count_cycl-1;  
}