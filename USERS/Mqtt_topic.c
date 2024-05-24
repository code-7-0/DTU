/********************************************************************************
* @file    	  lte.c
* @function   LTE模块 AT命令配置
* @brief 	    用3.5.0版本库建的工程模板
* @attention	
* @version 	  V1.1
********************************************************************************/
#include "global.h"

//1.是否启用；2.待发送标识；3.周期(单位0.5秒)；4.计数；5.标识符；6.数据；
struct Topic_dat  Topic_dat[40] = 
{
        0,0,1350,15,"DAT_mode","0",  					//开/关机_模式
				0,0,1550,16,"DAT_Load","0",						//负载状态
				0,0,1560,17,"DAT_PartsState","0",			//开关信号
				0,0,1520,18,"DAT_RunState","0",				//运行状况
				0,0,1810,19,"DAT_Dmenu","0",					//D菜单
				0,0,1380,20,"DAT_BathTempSet","0",    //卫浴设定温度
				0,0,1390,0 ,"DAT_HeatTempSet","0",		//采暖设定温度
				0,0,1530,20,"DAT_HotWaterFlow","0",	//热水流量
        0,0,1525,30,"DAT_BathTemp","0",			//卫浴温度
				0,0,1565,40,"DAT_HeatTemper","0",		//采暖温度
				0,0,1500,50,"DAT_OutDTemper","0",		//室外温度
				0,0,1520,60,"DAT_IndoorTemper","0",	//室内温度
				0,0,1540,70,"DAT_InWaterTempER","0",	//自开水温度
				0,0,1710,80,"DAT_InWaterPress","0",	//自来水压
				0,0,1330,90,"DAT_FireLevel","0",			//当前火力
				0,0,1960,100,"DAT_AlarmCode","0",		//报警代码
				1,0,MQTT_DAT_UP_DELAY,0,"ALL_DAT","ABCDEF",
				0,0,60,12,"default","0",
				0,0,60,13,"default","0",
				0,0,60,14,"default","0",
				//故障或者报警，不作为定时命令启用
				0,0,1000,1,"E1","0",			//20点火失败故障
				0,0,1000,1,"E2","0",			//21残焰、假火
				0,0,1000,1,"E3","0",			//22限温器故障(机械限温器断开)
				0,0,1000,1,"E4","0",			//23火焰4分钟内熄火5次（着火后火焰检测有效后又熄火）
				0,0,1000,1,"E6","0",			//24温度不上升或传感器脱落故障(点火燃烧持续30s后,采暖传感器和卫浴水传感器温度变化不超过2℃)
				0,0,1000,1,"E7","0",			//25防冻故障≤3℃
				0,0,1000,1,"F1","0",			//26风压故障
				0,0,1000,1,"F2","0",			//27缺水或水压开关故障（水泵启动前检测，检测失败不启动水泵，连续5s检测失败报警。）
        0,0,1000,1,"F3","0",			//28采暖温度传感器故障（采暖NTC开路、短路或连线松脱、短路）
				0,0,1000,1,"F4","0",			//29卫浴水温度传感器故障（卫浴水NTC开路、短路或连线松脱、短路)
				0,0,1000,1,"F5","0",			//30温度传感器超温故障
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
			
unsigned char topic_state;			//待发送命令直针
unsigned char topic_state_last = 1; //已发送命令直针

char topic_buf1[256];
char topic_buf2[256];		
			
unsigned int  topic_count = 0;			  //命令序号
unsigned char topic_buf_ok = 0;			  //拼装命令标识
			
unsigned char topic_mode = 0;   //0慢速模式	，1快速模式	
unsigned char topic_update_flag = 0;  //每秒更新一次标志				

//数值转字符串函数			
void char_str( char *arry,unsigned int dat)
{
      unsigned char i = 0; 
	
	    do
			{
				arry[i] = 0;
				i++;
			}while(arry[i]!=0);
			
			i=0;
	 
	    if(dat>99)  //3位
					{
					    do
							{
								 arry[2-i] = 0x30 + dat%10;
								 dat/=10;
								 i++;
							}while(dat);
					}
			else if((dat>9)&&(dat<100)) //2位
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

// 检查有多少个未发送命令
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

//数据主动发送
void Handle_topic(void)
{
		unsigned char i;
	  unsigned int  dat;
	   char buf[5] = {0};
	  //计时与数据更新，命令队列-推入//////////////////////////////////////////////////////////////////////////
	  if(topic_update_flag) //周期性进入
		{
				topic_update_flag = 0;
		    for(i=0;i<TOPIC_DAT_NUM;i++) //遍历
				{
				  if(Topic_dat[i].enable) //判断是否启用，定时发送
					{ 
						Topic_dat[i].send_count++;
						 //计时更新
						 if(Topic_dat[i].send_count>Topic_dat[i].send_cont_cycl)
						 {
								Topic_dat[i].send_count = 0; 
								Topic_dat[i].send_flag = 1;
							  
						 }
						 //判断数据是否更新
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
		//命令队列-拉出///////////////////////////////////////////////////////////////////////////////
		if(topic_state == 0) //待发送命令直针
		{
				i = topic_state_last; //从上次命令
			  if(topic_check_list())
				{
					do
					{
						 i++;
						
						 if(i > TOPIC_DAT_NUM) i = 1; //循环
						
						if(Topic_dat[i-1].send_flag) //检查是否有待发送命令
						 {
								topic_state = i; //记录命令指针
								Topic_dat[i-1].send_flag = 0; //清除标志，只发送一次
							  //printf("推出—%d\r",topic_state); //打印命令指针
						 }
					}while((topic_state == 0)&&(i != topic_state_last)); 	
				}
		}
    //拼装命令///////////////////////////////////////////////////////////////////////////////
    if((topic_state!=0)&&(topic_buf_ok==0))
		{
			    
			    topic_buf_ok = 1;  //等待发送标识
			    //printf("MQ拼装");
			    clear_bufer(topic_buf1,256);
					clear_bufer(topic_buf2,256);
				  strcat(topic_buf2,"{\"id\":\"");
				  i=0;

				  //buf[0] =0x35; //固定为字符"5"
			    
					
					strcat(topic_buf2,"5");
					strcat(topic_buf2,"\",\"params\":{\"");
					strcat(topic_buf2,Topic_dat[topic_state-1].identifier); //填充标识符
				  strcat(topic_buf2,"\":");
			    if(topic_state == 17)  //字符串
					{
						 strcat(topic_buf2,GASwall_Manage[1].All_DAT);
					}
					else  //char型数据
					{
					    
						  strcat(topic_buf2,Topic_dat[topic_state-1].dat);        //填充数据，非零即填
					}
				  
					//strcat(topic_buf2,"}}\r\n");
					
					strcat(topic_buf2,"}");
					strcat(topic_buf2,",\"SN\":\"");
					sprintf(buf,"%d",topic_count);
					strcat(topic_buf2,buf);
					strcat(topic_buf2,"\"}\r\n");
         
					//发布-拼装
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
		
		//故障报警
		if(GASwall_Manage[1].ERRO_CODE != GASwall_Manage[0].ERRO_CODE) //判断故障代码是否发生变化
		{

			switch(GASwall_Manage[0].ERRO_CODE) //检索故障代码
			{ 
				//-----------------------------故障解除---------------------------
				case 0: //故障清除
						switch(GASwall_Manage[1].ERRO_CODE) //检索故障解除代码
						{
							case ERROR_FIRE: //E1点火失败故障
									Topic_dat[20].send_flag = 1;
									char_str(Topic_dat[20].dat,0); 
								break;
							case ERROR_BURN://E2 ERROR_BURN
									Topic_dat[21].send_flag = 1;
									char_str(Topic_dat[21].dat,0); 
								break;
							case ERROR_HIGH2: //E3 限温器故障
									Topic_dat[22].send_flag = 1;
									char_str(Topic_dat[22].dat,0); 
								break;
							case ERROR_OUT: //E4 火焰4分钟内熄火5次
								Topic_dat[23].send_flag = 1;
									char_str(Topic_dat[23].dat,0); 
								break;
							case ERROR_HEAT://E6 温度不上升或传感器脱落故障
								Topic_dat[24].send_flag = 1;
									char_str(Topic_dat[24].dat,0); 
								break;
							case ERROR_FREZ://E7 防冻故障
								Topic_dat[25].send_flag = 1;
									char_str(Topic_dat[25].dat,0); 
								break;
							case ERROR_FAN://F1 风压故障
								Topic_dat[26].send_flag = 1;
									char_str(Topic_dat[26].dat,0); 
								break;
							case ERROR_PRESS://F2 缺水或水压开关故障
								Topic_dat[27].send_flag = 1;
									char_str(Topic_dat[27].dat,0); 
								break;
							case ERROR_TEMP2://F3 采暖温度传感器故障
								Topic_dat[28].send_flag = 1;
									char_str(Topic_dat[28].dat,0); 
								break;
							case ERROR_TEMP1://F4 卫浴水温度传感器故障
								Topic_dat[29].send_flag = 1;
									char_str(Topic_dat[29].dat,0); 
								break;
							case ERROR_HIGH://F5 温度传感器超温故障
								Topic_dat[30].send_flag = 1;
									char_str(Topic_dat[30].dat,0); 
								break;
							default:
								break;
						}
					break;
				//-----------------------------故障发生---------------------------
				case ERROR_FIRE: //E1点火失败故障
					Topic_dat[20].send_flag = 1;
				  char_str(Topic_dat[20].dat,1); 
					break;
				case ERROR_BURN://E2 ERROR_BURN
					Topic_dat[21].send_flag = 1;
				  char_str(Topic_dat[21].dat,1); 
					break;
				case ERROR_HIGH2: //E3 限温器故障
					Topic_dat[22].send_flag = 1;
				  char_str(Topic_dat[22].dat,1); 
					break;
				case ERROR_OUT: //E4 火焰4分钟内熄火5次
					Topic_dat[23].send_flag = 1;
				  char_str(Topic_dat[23].dat,1); 
					break;
				case ERROR_HEAT://E6 温度不上升或传感器脱落故障
					Topic_dat[24].send_flag = 1;
				  char_str(Topic_dat[24].dat,1); 
					break;
				case ERROR_FREZ://E7 防冻故障
					Topic_dat[25].send_flag = 1;
				  char_str(Topic_dat[25].dat,1); 
					break;
				case ERROR_FAN://F1 风压故障
					Topic_dat[26].send_flag = 1;
				  char_str(Topic_dat[26].dat,1); 
					break;
				case ERROR_PRESS://F2 缺水或水压开关故障
					Topic_dat[27].send_flag = 1;
				  char_str(Topic_dat[27].dat,1); 
					break;
				case ERROR_TEMP2://F3 采暖温度传感器故障
					Topic_dat[28].send_flag = 1;
				  char_str(Topic_dat[28].dat,1); 
					break;
				case ERROR_TEMP1://F4 卫浴水温度传感器故障
					Topic_dat[29].send_flag = 1;
				  char_str(Topic_dat[29].dat,1); 
					break;
				case ERROR_HIGH://F5 温度传感器超温故障
					Topic_dat[30].send_flag = 1;
				  char_str(Topic_dat[30].dat,1); 
					break;
				default:
					break;
			}
			//更新
			char_str(Topic_dat[15].dat,GASwall_Manage[0].ERRO_CODE);
			GASwall_Manage[1].ERRO_CODE = GASwall_Manage[0].ERRO_CODE;
		}
}

unsigned char Trigger_MQ_state = 0;// 0 发送


//已从手动控制命令下发后，主动上报mqtt数据
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
					 Trigger_MQ_state = 0;//复位标志
				 }
				 break;
			 default:
				 break;
		 }
}
//当用户手动进行配置时，进入活跃状态，10秒报送一次
unsigned int live_flag_cunt = 300;//活跃延时 150秒

void Handle_MQTT_live(void) 
{
     if(live_flag_cunt%10==1)
		 {
		     Topic_dat[16].send_count = Topic_dat[16].send_cont_cycl-MQTT_DAT_UP_DELAY_LIVE;
		 }
}

