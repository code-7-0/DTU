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
					
				//�������,sscanfͨ��'\0���жϽ���
			 memset(g_tModule.Debug_RevBuf,0,sizeof(g_tModule.Debug_RevBuf));
			 }
	 }
}
 
