#include "debug.h"
//uint8_t fist_run_flag=1; //���ε�¼��־

 
void Handle_Debug(void)
{  
//     if(fist_run_flag ==1)//������ӡ
//		 {
//			 debug_printf("%s\r\n", Device_Flash.hello);
//			 debug_printf("*****Ӳ���汾V1.0.0*****\r\n");  // \n��ʾ����
//			 debug_printf("*****����汾V1.0.0*****\r\n");  // \n��ʾ����
//			 debug_printf("ClientId:    %s\r\n", Device_Flash.clientId);
//			 debug_printf("MqttHostUrl: %s\r\n", Device_Flash.mqttHostUrl);
//			 debug_printf("Port:        %s\r\n", Device_Flash.port);
//			 debug_printf("Passwd:      %s\r\n", Device_Flash.passwd);
//			 debug_printf("Username:    %s\r\n", Device_Flash.username);
//			 debug_printf("QMTSUB1:     %s\r\n", Device_Flash.QMTSUB1);
//			 debug_printf("QMTSUB2:     %s\r\n", Device_Flash.QMTSUB2);
//			 debug_printf("QMTSUB3:     %s\r\n", Device_Flash.QMTSUB3);
//			 fist_run_flag=0;
//		 }
		 int tempValue = 0; 
		 if(g_tModule.Debug_RevFlag)
		 {
		   g_tModule.Debug_RevFlag = 0; 
			 Debug_Usart_Read(g_tModule.Debug_RevBuf,DEBUG_SIZE_MAX);//���¿������ڽ���
			 if(g_tModule.Debug_BufLen>0)
				{
					 if(strcmp((char*)g_tModule.Debug_RevBuf,"version") ==0) //�̼��汾
					 {
							debug_printf("�̼��汾��%d \r\n",VER_MAJOR);
					 }						
					 if(strcmp((char*)g_tModule.Debug_RevBuf,"zero") ==0) //���
					 {
							debug_printf("��ֵ%04d,����%d\r\n",zero_value,zero_cnt);
							debug_printf("zero_buf:%s\r\n",zero_buf);
					 }					
					 if(strcmp((char*)g_tModule.Debug_RevBuf,"outlinecnt") ==0) //ec600��λ����
					 {
							debug_printf("��λ����%d��",outlinetimes);
					 }
					 if(strcmp((char*)g_tModule.Debug_RevBuf,"update") ==0) //����
					 {
							update_flag=1;
							debug_printf("׼��OTA����");
					 }					
						//�¿������� 01-ɾ���豸04-����-03����豸-�豸�����ϱ���������07-05״̬��ѯ
					 //-�����ı䵥��07����-�鿴��������
					 //-06����
					 //��������0x01
					 #define RAPID_CNT 30
					 if(strcmp((char*)g_tModule.Debug_RevBuf,"temper01") ==0) 
					 {
						 temper_rapif_reply_flag=1;
						 temper_rapif_reply_cnt=RAPID_CNT;
							construct_send_command_to_fifo(0x01,NULL,NULL);
					 }						 
					 
					 //�ر�����02
					 if(strcmp((char*)g_tModule.Debug_RevBuf,"temper02") ==0) 
					 {
						 temper_rapif_reply_flag=1;
						 temper_rapif_reply_cnt=RAPID_CNT;
							construct_send_command_to_fifo(0x02,NULL,NULL);
					 }							 
					 //�����·�
//						if(sscanf((char*)g_tModule.Debug_RevBuf, "temper=%d", &tempValue) == 1) 
//						{
//								 construct_send_command_to_fifo(0x06,NULL);
//						}				 
					 //״̬��ѯ05
					 if(strcmp((char*)g_tModule.Debug_RevBuf,"temper05") ==0) 
					 {
						 temper_rapif_reply_flag=1;
						 temper_rapif_reply_cnt=RAPID_CNT;
						 temper_rapif_reply_cnt=RAPID_CNT;
							construct_send_command_to_fifo(0x05,NULL,NULL);
					 }			 
					 //�豸ɾ��04 delete:5E 56 80 31
					 uint8_t buf[4]={0x5E,0x56,0x80,0x31};
				   if(strcmp((char*)g_tModule.Debug_RevBuf,"temper04") ==0) 
//					 if(sscanf((char*)g_tModule.Debug_RevBuf,"delete:%s",buf) ==1) 
					 {
						 temper_rapif_reply_flag=1;
						 temper_rapif_reply_cnt=RAPID_CNT;
							construct_send_command_to_fifo(0x04,buf,NULL);
					 }
					//��������06				 
				   if(strcmp((char*)g_tModule.Debug_RevBuf,"temper06") ==0)  
					 {
//							Device* device=findDevice(&Linked_List,buf);
//							device->device_info.device_data.mode=1;
//							 device->device_info.device_data.sw=1;
//							 device->device_info.device_data.temp_set=450;
							DeviceData device_data;
							device_data.mode=1;
							 device_data.sw=1;
							 device_data.temp_set=450;						 
							construct_send_command_to_fifo(0x06,buf,&device_data);
						 temper_rapif_reply_flag=1;
						 temper_rapif_reply_cnt=RAPID_CNT;
					 }	
						//�豸��ѯ0A
				   if(strcmp((char*)g_tModule.Debug_RevBuf,"temper0A") ==0)  
					 { 
							construct_send_command_to_fifo(0x0A,NULL,NULL);
//						 temper_rapif_reply_flag=1;
						 
					 }						 
					 //�豸�鿴
					 if(strcmp((char*)g_tModule.Debug_RevBuf,"idlist") ==0) 
					 {
							extern void traverseDevices(void);
							traverseDevices();
						 temper_rapif_reply_flag=1;
						 temper_rapif_reply_cnt=RAPID_CNT;
					 }		
					 if(strncmp((char*)g_tModule.Debug_RevBuf,"temper_mqtt:",strlen("temper_mqtt:")) ==0)  
					 { 
//mqtt:+QMTRECV: 0,0,"/sys/i60d1iPGj1Pl/lhGP0rlsr5COaGog/thing/event/property/tempercontrol/distribution","{"action":"networkIn"}
							sscanf((char*)g_tModule.Debug_RevBuf, "temper_mqtt:%s", g_tModule.EC600_RevBuf);
							g_tModule.EC600_RevBuf[g_tModule.Debug_BufLen]=0;
						//	debug_printf("MQTT_setting_username:    %s\r\n", Device_Flash.username);
					 }			
					 
					 //�ڹ�¯����
						/*ȼ���ڹ�¯�ֶ�����*/
					 if(strcmp((char*)g_tModule.Debug_RevBuf,"gasp_closed") ==0) //�ֶ��ر�ȼ���ڹ�¯
					 {
							Gaspower_Command(GAS_SET_ONOFF,0,Gaspower_index);
					 }
					 
					 if(strcmp((char*)g_tModule.Debug_RevBuf,"gasp_open") ==0) //�ֶ�����ȼ���ڹ�¯
					 {
							Gaspower_Command(GAS_SET_ONOFF,1,Gaspower_index);
					 }
//					 if(strcmp((char*)g_tModule.Debug_RevBuf,"bathroom") ==0) // 
//					 {
//							Gaspower_Command(GAS_SET_BATHROOM_TEMP,40,Gaspower_index);
//					 }		
						if(sscanf((char*)g_tModule.Debug_RevBuf, "bathroom=%d", &tempValue) == 1) 
						{
								Gaspower_Command(GAS_SET_BATHROOM_TEMP, tempValue, Gaspower_index);
						}		
//					 if(strcmp((char*)g_tModule.Debug_RevBuf,"heat") ==0) //�ֶ�����ȼ���ڹ�¯
//					 {
//							Gaspower_Command(GAS_SET_HEATINGTEMP,50,Gaspower_index);
//					 }						 
						if(sscanf((char*)g_tModule.Debug_RevBuf, "heat=%d", &tempValue) == 1) 
						{
								Gaspower_Command(GAS_SET_HEATINGTEMP, tempValue, Gaspower_index);
						}					 
//					 if(strcmp((char*)g_tModule.Debug_RevBuf,"TEST_ER") ==0) //���Թ��������
//					 {
//						  if(GasPower_Manage.ERRO_CODE) GasPower_Manage.ERRO_CODE = 0;
//						  else GasPower_Manage.ERRO_CODE = ERROR_FIRE;
//					 }
		
					 
		//			  if(strcmp((char*)g_tModule.Debug_RevBuf,"Realtime_Set") ==0) //ʱ��У׼
		//			 {
		//					 Gaspower_Command(GAS_SET_TIME_CALIB,1505,Gaspower_index);
		//			 }
					 
					 
					 /*�����λ*/
					 if(strcmp((char*)g_tModule.Debug_RevBuf,"system_RST=1") ==0) //����λ
					 {
							//__set_FAULTMASK(1);//�ر������ж�
							NVIC_SystemReset();//��λ����
					 }
					 
					 
					 /*���ص�����Ϣ*/
						if(strcmp((char*)g_tModule.Debug_RevBuf,"debug_Gaspower") ==0) //ȼ���ڹ�¯������Ϣ����
					 {
							if(debug_onoff.Gaspower) 
							{
								debug_onoff.Gaspower = 0;
								debug_printf("�ڹ�¯��Ϣ��ӡ-OFF");
							}
							else 
							{
								debug_onoff.Gaspower = 1;
								debug_printf("�ڹ�¯��Ϣ��ӡ-ON");
							}
					 }				 
					 if(strcmp((char*)g_tModule.Debug_RevBuf,"debug_EC600N") ==0) //ȼ���ڹ�¯������Ϣ����
					 {
							if(debug_onoff.EC600N) 
								{debug_onoff.EC600N = 0;
								 debug_printf("EC600N��Ϣ��ӡ-OFF");
								}
							else 
							{
								debug_onoff.EC600N = 1;
								debug_printf("EC600N��Ϣ��ӡ-ON");
							}
					 }
					 if(strcmp((char*)g_tModule.Debug_RevBuf,"GasCost") ==0) //ȼ�������ӡ����
					 {
							if(debug_onoff.GasCost) 
							{
								debug_onoff.GasCost = 0;
								debug_printf("ȼ��������Ϣ��ӡ-OFF");
							}
							else 
							{
								debug_onoff.GasCost = 1;
								debug_printf("ȼ��������Ϣ��ӡ-ON");
							}
					 }
						if(strcmp((char*)g_tModule.Debug_RevBuf,"MQ_send") ==0)  //������Ϣ��ӡ
					 {
							if(debug_onoff.MQ_send) debug_onoff.MQ_send = 0;
							else debug_onoff.MQ_send = 1;
					 }
			 
					 
	 
					 /*FLASH��Ϣ����*/
					 if(strcmp((char*)g_tModule.Debug_RevBuf,"flash_save") ==0) //�ֶ��洢����
					 {	
						 Flag_fresh_flag = 1;
					 }
						if(strcmp((char*)g_tModule.Debug_RevBuf,"flash_factory") ==0) //�ָ���������
					 {	
						 Flash_Factory();
					 }
					 if(strncmp((char*)g_tModule.Debug_RevBuf,"MQTT_username=",strlen("MQTT_username=")) ==0) //����MQTT_username
					 {
//							memset(Device_Flash.username,0,sizeof(Device_Flash.username)); //���
//							strcat(Device_Flash.username,((char*)g_tModule.Debug_RevBuf+strlen("MQTT_username=")));
							sscanf((char*)g_tModule.Debug_RevBuf, "MQTT_username=%s", Device_Flash.username);
							debug_printf("MQTT_setting_username:    %s\r\n", Device_Flash.username);
					 }
					if(strncmp((char*)g_tModule.Debug_RevBuf, "MQTT_clientId=", strlen("MQTT_clientId=")) == 0) {
							sscanf((char*)g_tModule.Debug_RevBuf, "MQTT_clientId=%s", Device_Flash.clientId);
							debug_printf("MQTT_setting_clientId:    %s\r\n", Device_Flash.clientId);
					}

					if(strncmp((char*)g_tModule.Debug_RevBuf, "MQTT_mqttHostUrl=", strlen("MQTT_mqttHostUrl=")) == 0) {
							sscanf((char*)g_tModule.Debug_RevBuf, "MQTT_mqttHostUrl=%s", Device_Flash.mqttHostUrl);
							debug_printf("MQTT_setting_mqttHostUrl:    %s\r\n", Device_Flash.mqttHostUrl);
					}

					if(strncmp((char*)g_tModule.Debug_RevBuf, "MQTT_passwd=", strlen("MQTT_passwd=")) == 0) {
							sscanf((char*)g_tModule.Debug_RevBuf, "MQTT_passwd=%s", Device_Flash.passwd);
							debug_printf("MQTT_setting_passwd:    %s\r\n", Device_Flash.passwd);
					}

					if(strncmp((char*)g_tModule.Debug_RevBuf, "MQTT_port=", strlen("MQTT_port=")) == 0) {
							sscanf((char*)g_tModule.Debug_RevBuf, "MQTT_port=%s", Device_Flash.port);
							debug_printf("MQTT_setting_port:    %s\r\n", Device_Flash.port);
					}
					
					if(strncmp((char*)g_tModule.Debug_RevBuf, "MQTT_IotCode=", strlen("MQTT_IotCode=")) == 0) {
							sscanf((char*)g_tModule.Debug_RevBuf, "MQTT_IotCode=%s", Device_Flash.iotCode);
							debug_printf("MQTT_setting_iotcode:    %s\r\n", Device_Flash.iotCode);
					}
					if(strncmp((char*)g_tModule.Debug_RevBuf, "MQTT_QMTSUB1=", strlen("MQTT_QMTSUB1=")) == 0) {
							sscanf((char*)g_tModule.Debug_RevBuf, "MQTT_QMTSUB1=%s", Device_Flash.QMTSUB1);
							debug_printf("MQTT_setting_QMTSUB1:    %s\r\n", Device_Flash.QMTSUB1);
					}

					if(strncmp((char*)g_tModule.Debug_RevBuf, "MQTT_QMTSUB2=", strlen("MQTT_QMTSUB2=")) == 0) {
							sscanf((char*)g_tModule.Debug_RevBuf, "MQTT_QMTSUB2=%s", Device_Flash.QMTSUB2);
							debug_printf("MQTT_setting_QMTSUB2:    %s\r\n", Device_Flash.QMTSUB2);
					}

					if(strncmp((char*)g_tModule.Debug_RevBuf, "MQTT_QMTSUB3=", strlen("MQTT_QMTSUB3=")) == 0) {
							sscanf((char*)g_tModule.Debug_RevBuf, "MQTT_QMTSUB3=%s", Device_Flash.QMTSUB3);
							debug_printf("MQTT_setting_QMTSUB3:    %s\r\n", Device_Flash.QMTSUB3);
					}

					if(strncmp((char*)g_tModule.Debug_RevBuf, "MQTT_QMTSUB4=", strlen("MQTT_QMTSUB4=")) == 0) {
							sscanf((char*)g_tModule.Debug_RevBuf, "MQTT_QMTSUB4=%s", Device_Flash.QMTSUB4);
							debug_printf("MQTT_setting_QMTSUB4:    %s\r\n", Device_Flash.QMTSUB4);
					}

					if(strncmp((char*)g_tModule.Debug_RevBuf, "MQTT_QMTSUB5=", strlen("MQTT_QMTSUB5=")) == 0) {
							sscanf((char*)g_tModule.Debug_RevBuf, "MQTT_QMTSUB5=%s", Device_Flash.QMTSUB5);
							debug_printf("MQTT_setting_QMTSUB5:    %s\r\n", Device_Flash.QMTSUB5);
					}
					if(strncmp((char*)g_tModule.Debug_RevBuf, "MQTT_QMTSUB6=", strlen("MQTT_QMTSUB6=")) == 0) {
							sscanf((char*)g_tModule.Debug_RevBuf, "MQTT_QMTSUB6=%s", Device_Flash.QMTSUB6);
							debug_printf("MQTT_setting_QMTSUB6:    %s\r\n", Device_Flash.QMTSUB6);
					}
					if(strncmp((char*)g_tModule.Debug_RevBuf, "MQTT_QMTSUB7=", strlen("MQTT_QMTSUB7=")) == 0) {
							sscanf((char*)g_tModule.Debug_RevBuf, "MQTT_QMTSUB7=%s", Device_Flash.QMTSUB7);
							debug_printf("MQTT_setting_QMTSUB7:    %s\r\n", Device_Flash.QMTSUB7);
					}					
				//�������,sscanfͨ��'\0���жϽ���
			 memset(g_tModule.Debug_RevBuf,0,sizeof(g_tModule.Debug_RevBuf));
			 }
	 }
}
 
