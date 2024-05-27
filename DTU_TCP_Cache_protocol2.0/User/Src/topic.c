 #include "topic.h"
//1.�����ͱ�ʶ��2.����(��λ0.5��)��3.��ʶ����4.���ݣ�
struct Topic_dat   Topic_data = 
{				//��ʱ����
				 MQTT_DAT_UPLOAD_DELAY,0,"ALL_DAT"//,"ABCDEF"
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
uint8_t topic_send_wait=0xFF;//0;			//����������ֱ�룬��ʼ�����̼�����
uint8_t topic_send_last = 1; //�ѷ�������ֱ��
uint8_t topic_send_ok = 1;			  //ƴװ�����ʶ
char message_buf[256];
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

void build_message(char* message_buf, int topic_send_wait, int topic_count) {
    // ��ʼ������ջ�����
    clear_buffer(message_buf, 256);

		if(topic_send_wait==0xFF)
		{   
				char version[16]; // ���ڴ�Ű汾���ַ���
				// ���ɰ汾���ַ���
				sprintf(version, "v%d.%d.%d", VER_MAJOR, VER_MINOR, VER_REVISION);
				// ����JSON�ַ���
				sprintf(message_buf, "{\"QCCID\": \"%s\", \"software_version\": \"%s\"}", QCCID, version);
			
		}
		else
		{
				// ���� Topic_data �� Topic_error �� identifier �� dat �����ַ�������
				const char* identifier = (topic_send_wait == 1) ? Topic_data.identifier : Topic_error[topic_send_wait - 2].identifier;
				const char* data = (topic_send_wait == 1) ? GasPower_Manage.ALL_DATA : Topic_error[topic_send_wait - 2].dat;

				// ʹ�� sprintf һ��������ַ�������
				// ע�⣺ȷ�� message_buf �㹻���Ա������
				sprintf(message_buf, "{\"id\":\"5\",\"params\":{\"%s\":%s},\"SN\":\"%d\"}\r\n", identifier, data, topic_count);
		}
		
}

void build_topic(char* topic_buf, int topic_send_wait, int message_length) {
    // ��ʼ������ջ�����
    clear_buffer(topic_buf, 256);

    // ���������ַ����Ŀ�ͷ
    strcat(topic_buf, "AT+QMTPUBEX=0,0,0,0,\"");

    // ����topic_send_wait��ֵѡ������
		strcat(topic_buf, 
					 (topic_send_wait == 1) ? Device_Flash.QMTSUB1 : 
					 (topic_send_wait == 0xFF) ? Device_Flash.QMTSUB4 : Device_Flash.QMTSUB3);
//    strcat(topic_buf, (topic_send_wait == 1) ? Device_Flash.QMTSUB1 : Device_Flash.QMTSUB3);
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
				Topic_data.send_count++;
				 //��ʱ����
				 if(Topic_data.send_count>Topic_data.send_count_cycl)
				 {
						Topic_data.send_count = 0; 
						send_flag |= 0x01; 
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
		
		if(topic_send_wait ==0 ) //����������ֱ��
		{
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
//		static uint8_t first_flag=1;
    //ƴװ����///////////////////////////////////////////////////////////////////////////////
    if((topic_send_wait!=0)&&(topic_send_ok==1)&&(qccid_get_flag==1))//������Ҫ�ϴ�ͬʱ��һ�������ϴ����
		{
					topic_send_ok = 0;  //�ȴ����ͱ�ʶ
					build_message(message_buf, topic_send_wait, topic_count); 
					int message_length = strlen(message_buf);
					build_topic(topic_buf, topic_send_wait, message_length); 	
					
		}
}


//uint8_t Trigger_MQ_state = 0;// 0 ����

//�ӿ��������·������������ϱ�mqtt����
void Data_Rapid_Reply(void)
{
			Topic_data.send_count = Topic_data.send_count_cycl-MQTT_DAT_UPLOAD_DELAY_LIVE;  
}
