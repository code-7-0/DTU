/********************************************************************************
* @file    	  lte.c
* @function   LTEģ�� AT��������
* @brief 	    ��3.5.0�汾�⽨�Ĺ���ģ��
* @attention	
* @version 	  V1.1
********************************************************************************/
#include "global.h"

//1.�Ƿ����ã�2.�����ͱ�ʶ��3.����(��λ0.5��)��4.������5.��ʶ����6.���ݣ�
struct Topic_dat  Topic_dat[40] = 
{
        0,0,1350,15,"DAT_mode","0",  					//��/�ػ�_ģʽ
				0,0,1550,16,"DAT_Load","0",						//����״̬
				0,0,1560,17,"DAT_PartsState","0",			//�����ź�
				0,0,1520,18,"DAT_RunState","0",				//����״��
				0,0,1810,19,"DAT_Dmenu","0",					//D�˵�
				0,0,1380,20,"DAT_BathTempSet","0",    //��ԡ�趨�¶�
				0,0,1390,0 ,"DAT_HeatTempSet","0",		//��ů�趨�¶�
				0,0,1530,20,"DAT_HotWaterFlow","0",	//��ˮ����
        0,0,1525,30,"DAT_BathTemp","0",			//��ԡ�¶�
				0,0,1565,40,"DAT_HeatTemper","0",		//��ů�¶�
				0,0,1500,50,"DAT_OutDTemper","0",		//�����¶�
				0,0,1520,60,"DAT_IndoorTemper","0",	//�����¶�
				0,0,1540,70,"DAT_InWaterTempER","0",	//�Կ�ˮ�¶�
				0,0,1710,80,"DAT_InWaterPress","0",	//����ˮѹ
				0,0,1330,90,"DAT_FireLevel","0",			//��ǰ����
				0,0,1960,100,"DAT_AlarmCode","0",		//��������
				1,0,MQTT_DAT_UP_DELAY,0,"ALL_DAT","ABCDEF",
				0,0,60,12,"default","0",
				0,0,60,13,"default","0",
				0,0,60,14,"default","0",
				//���ϻ��߱���������Ϊ��ʱ��������
				0,0,1000,1,"E1","0",			//20���ʧ�ܹ���
				0,0,1000,1,"E2","0",			//21���桢�ٻ�
				0,0,1000,1,"E3","0",			//22����������(��е�������Ͽ�)
				0,0,1000,1,"E4","0",			//23����4������Ϩ��5�Σ��Ż���������Ч����Ϩ��
				0,0,1000,1,"E6","0",			//24�¶Ȳ������򴫸����������(���ȼ�ճ���30s��,��ů����������ԡˮ�������¶ȱ仯������2��)
				0,0,1000,1,"E7","0",			//25�������ϡ�3��
				0,0,1000,1,"F1","0",			//26��ѹ����
				0,0,1000,1,"F2","0",			//27ȱˮ��ˮѹ���ع��ϣ�ˮ������ǰ��⣬���ʧ�ܲ�����ˮ�ã�����5s���ʧ�ܱ�������
        0,0,1000,1,"F3","0",			//28��ů�¶ȴ��������ϣ���ůNTC��·����·���������ѡ���·��
				0,0,1000,1,"F4","0",			//29��ԡˮ�¶ȴ��������ϣ���ԡˮNTC��·����·���������ѡ���·)
				0,0,1000,1,"F5","0",			//30�¶ȴ��������¹���
				0,0,1000,1,"F0","0",			//
				0,0,1000,1,"F0","0",			//
				0,0,1000,1,"F0","0",			//
				0,0,1000,1,"F0","0",			//
				0,0,1000,1,"F0","0",			//
				0,0,1000,1,"F0","0",			//
				0,0,1000,1,"F0","0",			//
				0,0,1000,1,"F0","0",			//
				0,0,1000,1,"F0","0"				//
};
			
unsigned char topic_state;			//����������ֱ��
unsigned char topic_state_last = 1; //�ѷ�������ֱ��

char topic_buf1[256];
char topic_buf2[256];		
			
unsigned int  topic_count = 0;			  //�������
unsigned char topic_buf_ok = 0;			  //ƴװ�����ʶ
			
unsigned char topic_mode = 0;   //0����ģʽ	��1����ģʽ	
unsigned char topic_update_flag = 0;  //ÿ�����һ�α�־				

//��ֵת�ַ�������			
void char_str( char *arry,unsigned int dat)
{
      unsigned char i = 0; 
	
	    do
			{
				arry[i] = 0;
				i++;
			}while(arry[i]!=0);
			
			i=0;
	 
	    if(dat>99)  //3λ
					{
					    do
							{
								 arry[2-i] = 0x30 + dat%10;
								 dat/=10;
								 i++;
							}while(dat);
					}
			else if((dat>9)&&(dat<100)) //2λ
					{
							
						  do
							{
								 arry[1-i] = 0x30 + dat%10;
								 dat/=10;
								 i++;
							}while(dat);
					}
			else
			{
              arry[0] = 0x30 + dat%10;  
			}
}

// ����ж��ٸ�δ��������
unsigned char  topic_check_list(void)
{
    unsigned char REdat,i;
	
	  REdat = 0;
	  for(i=0;i<TOPIC_DAT_NUM;i++)
		{
			REdat += Topic_dat[i].send_flag;
	  }
		return REdat;
}

//������������
void Handle_topic(void)
{
		unsigned char i;
	  unsigned int  dat;
	   char buf[5] = {0};
	  //��ʱ�����ݸ��£��������-����//////////////////////////////////////////////////////////////////////////
	  if(topic_update_flag) //�����Խ���
		{
				topic_update_flag = 0;
		    for(i=0;i<TOPIC_DAT_NUM;i++) //����
				{
				  if(Topic_dat[i].enable) //�ж��Ƿ����ã���ʱ����
					{ 
						Topic_dat[i].send_count++;
						 //��ʱ����
						 if(Topic_dat[i].send_count>Topic_dat[i].send_cont_cycl)
						 {
								Topic_dat[i].send_count = 0; 
								Topic_dat[i].send_flag = 1;
							  
						 }
						 //�ж������Ƿ����
						 switch(i)
						 {
							 case 0: 
								 if(GASwall_Manage[1].on_off != GASwall_Manage[0].on_off)
								 {
										char_str(Topic_dat[i].dat,GASwall_Manage[0].on_off);
										GASwall_Manage[1].on_off = GASwall_Manage[0].on_off;
										//Topic_dat[i].send_flag = 1;
								 }
								 break;
							 case 1:
								 if(GASwall_Manage[1].load_state != GASwall_Manage[0].load_state)
								 {
										char_str(Topic_dat[i].dat,GASwall_Manage[0].load_state);
										GASwall_Manage[1].load_state = GASwall_Manage[0].load_state;
										//Topic_dat[i].send_flag = 1;
								 }							 
								 break;
							 case 2:
								 if(GASwall_Manage[1].Dev_switch != GASwall_Manage[0].Dev_switch)
								 {
										char_str(Topic_dat[i].dat,GASwall_Manage[0].Dev_switch);
										GASwall_Manage[1].Dev_switch = GASwall_Manage[0].Dev_switch;
										//Topic_dat[i].send_flag = 1;
								 }
								 
								 break;
							 case 3:
								 if(GASwall_Manage[1].run_state != GASwall_Manage[0].run_state)
								 {
										char_str(Topic_dat[i].dat,GASwall_Manage[0].run_state);
										GASwall_Manage[1].run_state = GASwall_Manage[0].run_state;
										//Topic_dat[i].send_flag = 1;
								 }
								 
								 break;
							 case 4:
								 if(GASwall_Manage[1].ECO != GASwall_Manage[0].ECO)
								 {
										char_str(Topic_dat[i].dat,GASwall_Manage[0].ECO);
										GASwall_Manage[1].ECO = GASwall_Manage[0].ECO;
										//Topic_dat[i].send_flag = 1;
								 }
								 
								 break;
							 case 5:
								 if(GASwall_Manage[1].Set_Bathroomtemp != GASwall_Manage[0].Set_Bathroomtemp)
								 {
										char_str(Topic_dat[i].dat,GASwall_Manage[0].Set_Bathroomtemp);
										GASwall_Manage[1].Set_Bathroomtemp = GASwall_Manage[0].Set_Bathroomtemp;
										//Topic_dat[i].send_flag = 1;
								 }
								 
								 break;
							 case 6:
								 if(GASwall_Manage[1].Sst_Heating_temp != GASwall_Manage[0].Sst_Heating_temp)
								 {
										char_str(Topic_dat[i].dat,GASwall_Manage[0].Sst_Heating_temp);
										GASwall_Manage[1].Sst_Heating_temp = GASwall_Manage[0].Sst_Heating_temp;
										//Topic_dat[i].send_flag = 1;
								 }
								 
								 break;
							 case 7:
								 if(GASwall_Manage[1].Hot_water_flow != GASwall_Manage[0].Hot_water_flow)
								 {
										char_str(Topic_dat[i].dat,GASwall_Manage[0].Hot_water_flow);
										GASwall_Manage[1].Hot_water_flow = GASwall_Manage[0].Hot_water_flow;
										//Topic_dat[i].send_flag = 1;
								 }
								 
								 break;
							 case 8:
								 if(GASwall_Manage[1].Bathroomtemp != GASwall_Manage[0].Bathroomtemp)
								 {
										char_str(Topic_dat[i].dat,GASwall_Manage[0].Bathroomtemp);
										GASwall_Manage[1].Bathroomtemp = GASwall_Manage[0].Bathroomtemp;
										//Topic_dat[i].send_flag = 1;
								 }
								 
								 break;
							 case 9:
								 if(GASwall_Manage[1].Heating_temp != GASwall_Manage[0].Heating_temp)
								 {
										char_str(Topic_dat[i].dat,GASwall_Manage[0].Heating_temp);
										GASwall_Manage[1].Heating_temp = GASwall_Manage[0].Heating_temp;
										//Topic_dat[i].send_flag = 1;
								 }
								 
								 break;
							 case 10:
								 if(GASwall_Manage[1].Outdoor_temp != GASwall_Manage[0].Outdoor_temp)
								 {
										char_str(Topic_dat[i].dat,GASwall_Manage[0].Outdoor_temp);
										GASwall_Manage[1].Outdoor_temp = GASwall_Manage[0].Outdoor_temp;
										//Topic_dat[i].send_flag = 1;
								 }
								 break;
							 case 11:
								 if(GASwall_Manage[1].Indoor_temp != GASwall_Manage[0].Indoor_temp)
								 {
										char_str(Topic_dat[i].dat,GASwall_Manage[0].Indoor_temp);
										GASwall_Manage[1].Indoor_temp = GASwall_Manage[0].Indoor_temp;
										//Topic_dat[i].send_flag = 1;
								 }
								 break;
							 case 12:
								 if(GASwall_Manage[1].Water_temp != GASwall_Manage[0].Water_temp)
								 {
										char_str(Topic_dat[i].dat,GASwall_Manage[0].Water_temp);
										GASwall_Manage[1].Water_temp = GASwall_Manage[0].Water_temp;
										//Topic_dat[i].send_flag = 1;
								 }							 
								 break;
							 case 13:
								 if(GASwall_Manage[1].Hydraulic != GASwall_Manage[0].Hydraulic)
								 {
										char_str(Topic_dat[i].dat,GASwall_Manage[0].Hydraulic);
										GASwall_Manage[1].Hydraulic = GASwall_Manage[0].Hydraulic;
										//Topic_dat[i].send_flag = 1;
								 }							 
								 break;
							 case 14:
								 if(GASwall_Manage[1].Firepower != GASwall_Manage[0].Firepower)
								 {
										char_str(Topic_dat[i].dat,GASwall_Manage[0].Firepower);
										GASwall_Manage[1].Firepower = GASwall_Manage[0].Firepower;
										//Topic_dat[i].send_flag = 1;
								 }
								 break;
							 default:
								 break;
						 }
						}
				}
		}
		//�������-����///////////////////////////////////////////////////////////////////////////////
		if(topic_state == 0) //����������ֱ��
		{
				i = topic_state_last; //���ϴ�����
			  if(topic_check_list())
				{
					do
					{
						 i++;
						
						 if(i > TOPIC_DAT_NUM) i = 1; //ѭ��
						
						if(Topic_dat[i-1].send_flag) //����Ƿ��д���������
						 {
								topic_state = i; //��¼����ָ��
								Topic_dat[i-1].send_flag = 0; //�����־��ֻ����һ��
							  //printf("�Ƴ���%d\r",topic_state); //��ӡ����ָ��
						 }
					}while((topic_state == 0)&&(i != topic_state_last)); 	
				}
		}
    //ƴװ����///////////////////////////////////////////////////////////////////////////////
    if((topic_state!=0)&&(topic_buf_ok==0))
		{
			    
			    topic_buf_ok = 1;  //�ȴ����ͱ�ʶ
			    //printf("MQƴװ");
			    clear_bufer(topic_buf1,256);
					clear_bufer(topic_buf2,256);
				  strcat(topic_buf2,"{\"id\":\"");
				  i=0;

				  //buf[0] =0x35; //�̶�Ϊ�ַ�"5"
			    
					
					strcat(topic_buf2,"5");
					strcat(topic_buf2,"\",\"params\":{\"");
					strcat(topic_buf2,Topic_dat[topic_state-1].identifier); //����ʶ��
				  strcat(topic_buf2,"\":");
			    if(topic_state == 17)  //�ַ���
					{
						 strcat(topic_buf2,GASwall_Manage[1].All_DAT);
					}
					else  //char������
					{
					    
						  strcat(topic_buf2,Topic_dat[topic_state-1].dat);        //������ݣ����㼴��
					}
				  
					//strcat(topic_buf2,"}}\r\n");
					
					strcat(topic_buf2,"}");
					strcat(topic_buf2,",\"SN\":\"");
					sprintf(buf,"%d",topic_count);
					strcat(topic_buf2,buf);
					strcat(topic_buf2,"\"}\r\n");
         
					//����-ƴװ
					strcat(topic_buf1,"AT+QMTPUBEX=0,0,0,0,\"");
					
					if(topic_state<=20)
					{
					strcat(topic_buf1,Device_Flash.QMTSUB1);
					}
					else
					{
					strcat(topic_buf1,Device_Flash.QMTSUB3);
					}
					strcat(topic_buf1,"\",");
					
					dat = strlen(topic_buf2);
					buf[4] = buf[3] = buf[2] = buf[1] = buf[0] = 0;
					i = 0;
					if(dat>99)
					{
					    do
							{
								 buf[2-i] = 0x30 + dat%10;
								 dat/=10;
								 i++;
							}while(dat);
					}
					else
					{
							do
							{
								 buf[1-i] = 0x30 + dat%10;
								 dat/=10;
								 i++;
							}while(dat);
					}
					strcat(topic_buf1,buf);
					strcat(topic_buf1,"\r\n");
		}
		
		//���ϱ���
		if(GASwall_Manage[1].ERRO_CODE != GASwall_Manage[0].ERRO_CODE) //�жϹ��ϴ����Ƿ����仯
		{

			switch(GASwall_Manage[0].ERRO_CODE) //�������ϴ���
			{ 
				//-----------------------------���Ͻ��---------------------------
				case 0: //�������
						switch(GASwall_Manage[1].ERRO_CODE) //�������Ͻ������
						{
							case ERROR_FIRE: //E1���ʧ�ܹ���
									Topic_dat[20].send_flag = 1;
									char_str(Topic_dat[20].dat,0); 
								break;
							case ERROR_BURN://E2 ERROR_BURN
									Topic_dat[21].send_flag = 1;
									char_str(Topic_dat[21].dat,0); 
								break;
							case ERROR_HIGH2: //E3 ����������
									Topic_dat[22].send_flag = 1;
									char_str(Topic_dat[22].dat,0); 
								break;
							case ERROR_OUT: //E4 ����4������Ϩ��5��
								Topic_dat[23].send_flag = 1;
									char_str(Topic_dat[23].dat,0); 
								break;
							case ERROR_HEAT://E6 �¶Ȳ������򴫸����������
								Topic_dat[24].send_flag = 1;
									char_str(Topic_dat[24].dat,0); 
								break;
							case ERROR_FREZ://E7 ��������
								Topic_dat[25].send_flag = 1;
									char_str(Topic_dat[25].dat,0); 
								break;
							case ERROR_FAN://F1 ��ѹ����
								Topic_dat[26].send_flag = 1;
									char_str(Topic_dat[26].dat,0); 
								break;
							case ERROR_PRESS://F2 ȱˮ��ˮѹ���ع���
								Topic_dat[27].send_flag = 1;
									char_str(Topic_dat[27].dat,0); 
								break;
							case ERROR_TEMP2://F3 ��ů�¶ȴ���������
								Topic_dat[28].send_flag = 1;
									char_str(Topic_dat[28].dat,0); 
								break;
							case ERROR_TEMP1://F4 ��ԡˮ�¶ȴ���������
								Topic_dat[29].send_flag = 1;
									char_str(Topic_dat[29].dat,0); 
								break;
							case ERROR_HIGH://F5 �¶ȴ��������¹���
								Topic_dat[30].send_flag = 1;
									char_str(Topic_dat[30].dat,0); 
								break;
							default:
								break;
						}
					break;
				//-----------------------------���Ϸ���---------------------------
				case ERROR_FIRE: //E1���ʧ�ܹ���
					Topic_dat[20].send_flag = 1;
				  char_str(Topic_dat[20].dat,1); 
					break;
				case ERROR_BURN://E2 ERROR_BURN
					Topic_dat[21].send_flag = 1;
				  char_str(Topic_dat[21].dat,1); 
					break;
				case ERROR_HIGH2: //E3 ����������
					Topic_dat[22].send_flag = 1;
				  char_str(Topic_dat[22].dat,1); 
					break;
				case ERROR_OUT: //E4 ����4������Ϩ��5��
					Topic_dat[23].send_flag = 1;
				  char_str(Topic_dat[23].dat,1); 
					break;
				case ERROR_HEAT://E6 �¶Ȳ������򴫸����������
					Topic_dat[24].send_flag = 1;
				  char_str(Topic_dat[24].dat,1); 
					break;
				case ERROR_FREZ://E7 ��������
					Topic_dat[25].send_flag = 1;
				  char_str(Topic_dat[25].dat,1); 
					break;
				case ERROR_FAN://F1 ��ѹ����
					Topic_dat[26].send_flag = 1;
				  char_str(Topic_dat[26].dat,1); 
					break;
				case ERROR_PRESS://F2 ȱˮ��ˮѹ���ع���
					Topic_dat[27].send_flag = 1;
				  char_str(Topic_dat[27].dat,1); 
					break;
				case ERROR_TEMP2://F3 ��ů�¶ȴ���������
					Topic_dat[28].send_flag = 1;
				  char_str(Topic_dat[28].dat,1); 
					break;
				case ERROR_TEMP1://F4 ��ԡˮ�¶ȴ���������
					Topic_dat[29].send_flag = 1;
				  char_str(Topic_dat[29].dat,1); 
					break;
				case ERROR_HIGH://F5 �¶ȴ��������¹���
					Topic_dat[30].send_flag = 1;
				  char_str(Topic_dat[30].dat,1); 
					break;
				default:
					break;
			}
			//����
			char_str(Topic_dat[15].dat,GASwall_Manage[0].ERRO_CODE);
			GASwall_Manage[1].ERRO_CODE = GASwall_Manage[0].ERRO_CODE;
		}
}

unsigned char Trigger_MQ_state = 0;// 0 ����


//�Ѵ��ֶ����������·��������ϱ�mqtt����
void Trigger_MQ_T(unsigned char state)
{
     switch(state)
		 {
			 case 0:
				 
				 break;
			 case 1:
				 Trigger_MQ_state = 1;
				 break;
			 case 2:
			   if(Trigger_MQ_state == 1)
				 {
					 Topic_dat[16].send_count = Topic_dat[16].send_cont_cycl-1; 
					 //Topic_dat[16].send_flag = 1;
					 Trigger_MQ_state = 0;//��λ��־
				 }
				 break;
			 default:
				 break;
		 }
}
//���û��ֶ���������ʱ�������Ծ״̬��10�뱨��һ��
unsigned int live_flag_cunt = 300;//��Ծ��ʱ 150��

void Handle_MQTT_live(void) 
{
     if(live_flag_cunt%10==1)
		 {
		     Topic_dat[16].send_count = Topic_dat[16].send_cont_cycl-MQTT_DAT_UP_DELAY_LIVE;
		 }
}

