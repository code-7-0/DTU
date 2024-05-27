 #include "topic.h"
//1.�����ͱ�ʶ��2.����(��λ0.5��)��3.��ʶ����4.���ݣ�
struct Topic_dat   Topic_data[2] = 
{				//��ʱ����
				 {GASPOWER_DAT_UPLOAD_DELAY,0,"ALL_DAT"},//,"ABCDEF"
				{TEMPER_DAT_UPLOAD_DELAY,0,"status"}
};
//1.�����ͱ�ʶ��2.��ʶ����3.���ݣ�
struct Topic_err  Topic_error[] = 
{
				//���ϻ��߱���������Ϊ��ʱ��������
				 "E1","0",			//20���ʧ�ܹ���
				 "E2","0",			//21���桢�ٻ�
				 "E3","0",			//22����������(��е�������Ͽ�)
				 "E4","0",			//23����4������Ϩ��5�Σ��Ż���������Ч����Ϩ��
				 "E6","0",			//24�¶Ȳ������򴫸����������(���ȼ�ճ���30s��,��ů����������ԡˮ�������¶ȱ仯������2��)
				 "E7","0",			//25�������ϡ�3��
				 "F1","0",			//26��ѹ����
				 "F2","0",			//27ȱˮ��ˮѹ���ع��ϣ�ˮ������ǰ��⣬���ʧ�ܲ�����ˮ�ã�����5s���ʧ�ܱ�������
         "F3","0",			//28��ů�¶ȴ��������ϣ���ůNTC��·����·���������ѡ���·��
				 "F4","0",			//29��ԡˮ�¶ȴ��������ϣ���ԡˮNTC��·����·���������ѡ���·)
				 "F5","0",			//30�¶ȴ��������¹���
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
//FaultInfo faults = {0,0,0,0};			//�豸���ϸ澯����
//uint8_t topic_send_wait=1;			//����������ֱ��
uint8_t topic_send_wait=0xFF;//0;			//������������ţ���ʼ�����̼�����
uint8_t topic_send_last = 0; //�ѷ�������ֱ��
uint8_t topic_send_ok = 1;			  //ƴװ�����ʶ
char message_buf[MESSAGE_BUF_SIZE];//5.69���ɱ��豸
char topic_buf[256];		
			
uint32_t  topic_count = 0;			  //�������
uint8_t topic_update_flag = 0;  //ÿ�����һ�α�־				


// ����ж��ٸ�δ��������
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
    char temp[5]; // �㹻���int���ֵ��'\0'
    sprintf(temp, "%d", value);
    strcat(buffer, temp);
}
char   QCCID[21];

// �� sub_id ת��Ϊ�ַ���
void sub_id_to_str(uint8_t sub_id[4], char* sub_id_str) {
    sprintf(sub_id_str, "%02X%02X%02X%02X", sub_id[0], sub_id[1], sub_id[2], sub_id[3]);
}
 

// ���� IDLIST JSON �ַ���
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


// ���� ADD��DELETE JSON �ַ���
void build_add_del_json(TemperMqttTopic* temper_topic_send, cJSON* root,LinkedList* list) {
//    Device* device = findDevice(list, temper_topic_send->sub_id);
//    if (device==NULL) { 
//        return ;
//    }
    char sub_id_str[9] = {0}; // 8 ���ַ� + ��ֹ��
    sub_id_to_str(temper_topic_send->sub_id, sub_id_str);		
		
		if(temper_topic_send->command==ADD)
			cJSON_AddStringToObject(root, "action", "add");
		else if(temper_topic_send->command==DELETE)
			cJSON_AddStringToObject(root, "action", "delete");
		
		cJSON* params = cJSON_CreateObject();
		cJSON_AddStringToObject(params, "sub_id", sub_id_str);
		cJSON_AddItemToObject(root, "params", params);

}

// ���� ONLINE JSON �ַ���
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
 
//�ֶ��Ƴ��ո�ĺ���
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

 
// ���� JSON �ַ���
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
		//char* json_string = cJSON_Print(root);//��̬����
    uint8_t success = cJSON_PrintPreallocated(root, message_buf ,buff_size, 1);
    if (success) {
        remove_spaces(message_buf); // �Ƴ��ո�
    }		
//    char* json_string = cJSON_PrintUnformatted(root);//�޿ո�
//    if (json_string == NULL) {
//        cJSON_Delete(root);
//        return 0;
//    }
    cJSON_Delete(root);
    return success; 
}

// ���� STATUS JSON �ַ���
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
      // ������������ sw �� mode ��ֵ
      //debug_printf("Adding sw: %d, mode: %d\n", current->device_info.device_data.sw, current->device_info.device_data.mode);
				cJSON_AddNumberToObject(device_status, "sw", current->device_info.device_data.sw);
        cJSON_AddNumberToObject(device_status, "mode", current->device_info.device_data.mode);


        cJSON_AddItemToArray(params, device_status);
        current = current->next;
    }
 
		
    uint8_t success = cJSON_PrintPreallocated(root,message_buf , buff_size, 1);
    if (success) {
        remove_spaces(message_buf); // �Ƴ��ո�
    }		
    cJSON_Delete(root);
    return success; 		
} 

void build_message(char* message_buf, uint8_t topic_send_wait, uint32_t topic_count,TemperMqttTopic* temper_topic_send) {
    // ��ʼ������ջ�����
    clear_buffer(message_buf, MESSAGE_BUF_SIZE);

		if(topic_send_wait==0xFF)
		{   
				char version[16]; // ���ڴ�Ű汾���ַ���
				// ���ɰ汾���ַ���
				sprintf(version, "v%d.%d.%d", VER_MAJOR, VER_MINOR, VER_REVISION);
				// ����JSON�ַ���
				sprintf(message_buf, "{\"QCCID\": \"%s\", \"software_version\": \"%s\"}", QCCID, version);
			
		}
		else if(topic_send_wait < 13)
		{
				// ���� Topic_data �� Topic_error �� identifier �� dat �����ַ�������
				const char* identifier = (topic_send_wait == 1) ? Topic_data[0].identifier : Topic_error[topic_send_wait - 2].identifier;
				const char* data = (topic_send_wait == 1) ? GasPower_Manage.ALL_DATA : Topic_error[topic_send_wait - 2].dat;

				// ʹ�� sprintf һ��������ַ�������
				// ע�⣺ȷ�� message_buf �㹻���Ա������
				sprintf(message_buf, "{\"id\":\"5\",\"params\":{\"%s\":%s},\"SN\":\"%d\"}\r\n", identifier, data, topic_count);
		}else if(topic_send_wait==13){//�ϱ��豸����
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
    // ��ʼ������ջ�����
    clear_buffer(topic_buf, 256);

    // ���������ַ����Ŀ�ͷ
    strcat(topic_buf, "AT+QMTPUBEX=0,0,0,0,\"");

    // ����topic_send_wait��ֵѡ������
//		strcat(topic_buf, 
//					 (topic_send_wait == 1) ? Device_Flash.QMTSUB1 : 
//					 (topic_send_wait == 0xFF) ? Device_Flash.QMTSUB4 : Device_Flash.QMTSUB3);
////    strcat(topic_buf, (topic_send_wait == 1) ? Device_Flash.QMTSUB1 : Device_Flash.QMTSUB3);
    // ���� topic_send_wait ��ֵѡ������
    if (topic_send_wait == 1) {
        strcat(topic_buf, Device_Flash.QMTSUB1);
    } else if (topic_send_wait == 0xFF) {
        strcat(topic_buf, Device_Flash.QMTSUB4);
    } else if (topic_send_wait >= 2 && topic_send_wait <= 13) {
        strcat(topic_buf, Device_Flash.QMTSUB3);
    } else if (topic_send_wait == 14 || topic_send_wait == 15) {
        strcat(topic_buf, Device_Flash.QMTSUB7);
    } else {
        strcat(topic_buf, Device_Flash.QMTSUB3); // Ĭ������
    }    
	
	
		strcat(topic_buf, "\",");

    // �����Ϣ����
    append_int_as_str(topic_buf, message_length);
    strcat(topic_buf, "\r\n");
}

 

//������������
void Handle_topic(void)
{
		uint8_t i; 
	  // ��ʱ�����ݸ��£��������-����//////////////////////////////////////////////////////////////////////////
	  if(topic_update_flag) //�����Խ��� 500mSһ��
		{
				topic_update_flag = 0;
				for(i=0;i<2;i++){
						Topic_data[i].send_count++;
						 //��ʱ����
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
		// ���ϱ���
		if (GasPower_Manage.ERRO_CODE != error_code_pre) { // �жϹ��ϴ����Ƿ����仯
				// �������
				if (GasPower_Manage.ERRO_CODE == 0) { 
						for (i = 0; i <= 10; i++) {
								if (Topic_error[i].dat[0] == '1') {
										send_flag |= 1 << (i + 1);
										strcpy(Topic_error[i].dat,"0");
								}
						}
				} else { // ���Ϸ���
						// ȷ��i��ֵ
						if(GasPower_Manage.ERRO_CODE==ERROR_FIRE) //E1���ʧ�ܹ���
							i=0;
//					send_flag|=1<<1; 
//				  append_int_as_str(Topic_error[0].dat,1); 
//					break;
						else if(GasPower_Manage.ERRO_CODE==ERROR_BURN)  //E2 ERROR_BURN
							i=1;
						else if(GasPower_Manage.ERRO_CODE==ERROR_HIGH2) //E3 ����������
							i=2;
						else if(GasPower_Manage.ERRO_CODE==ERROR_OUT)  //E4 ����4������Ϩ��5��
							i=3;
						else if(GasPower_Manage.ERRO_CODE==ERROR_HEAT) //E6 �¶Ȳ������򴫸����������
							i=4;
						else if(GasPower_Manage.ERRO_CODE==ERROR_FREZ) //E7 ��������
							i=5;
						else if(GasPower_Manage.ERRO_CODE==ERROR_FAN) //F1 ��ѹ����
							i=6;
						else if(GasPower_Manage.ERRO_CODE==ERROR_PRESS) //F2 ȱˮ��ˮѹ���ع���
							i=7;
						else if(GasPower_Manage.ERRO_CODE==ERROR_TEMP2) //F3 ��ů�¶ȴ���������
							i=8;
						else if(GasPower_Manage.ERRO_CODE==ERROR_TEMP1) //F4 ��ԡˮ�¶ȴ���������
							i=9;
						else if(GasPower_Manage.ERRO_CODE==ERROR_HIGH) //F5 �¶ȴ��������¹���
							i=10; 

						send_flag |= 1 << (i + 1); 
						strcpy(Topic_error[i].dat, "1");
				}
				error_code_pre = GasPower_Manage.ERRO_CODE; // ����
		}		
		
		
		
		//�������-����///////////////////////////////////////////////////////////////////////////////
		 TemperMqttTopic temper_topic_send;
		if(topic_send_wait ==0 ) //����������ֱ��
		{
				if(MqttTopicFiFoSize()>0)//�����ȼ�����
				{ 
					MqttTopicFiFoPop(&temper_topic_send);
					topic_send_wait=15;
				}else{
					i = topic_send_last; //���ϴ�����
					if(send_flag>0)
					{
						do
						{
							 i++;
							
							 if(i > 13) i = 1; //ѭ��
							
							if(send_flag & ( 1 << (i-1)) ) //����Ƿ��д���������
							 {
									topic_send_wait = i; //��¼����ָ��,>0
									send_flag &= ~( 1 << (i-1) ); //�����־��ֻ����һ�� 
							 }
						}while((topic_send_wait == 0)&&(i != topic_send_last)); 	
					}
				}
		}		
//		static uint8_t first_flag=1;
    //ƴװ����///////////////////////////////////////////////////////////////////////////////
    if((topic_send_wait!=0)&&(topic_send_ok==1)&&(qccid_get_flag==1))//������Ҫ�ϴ�ͬʱ��һ�������ϴ����
		{
					topic_send_ok = 0;  //�ȴ����ͱ�ʶ
					build_message(message_buf, topic_send_wait, topic_count,&temper_topic_send); 
					int message_length = strlen(message_buf);
					build_topic(topic_buf, topic_send_wait, message_length); 
			
//					if(topic_send_wait>=14){
//						debug_printf("topic_buf:%s\r\n",topic_buf);			
//						debug_printf("message_buf:%s\r\n",message_buf);		
//					}
					
		}

}


//uint8_t Trigger_MQ_state = 0;// 0 ����

//�ӿ��������·������������ϱ�mqtt����
void GasPower_Rapid_Reply(void)
{
			Topic_data[0].send_count = Topic_data[0].send_count_cycl-DAT_UPLOAD_DELAY_LIVE;  
}
void Temper_Rapid_Reply(void)
{
			Topic_data[1].send_count = Topic_data[1].send_count_cycl-1;  
}