
//燃气壁挂炉
#include "global.h"

typedef enum
{
    BUSY_NULL=0,        //空闲
	  BUSY_WAIT,          //等待
		BUSY_SEND,        	//发送忙
	  BUSY_REV            //接收忙
}   BUS_STATES;

typedef enum
{
	  BYTE_STATE_0 = 0,   //引导码
		BYTE_STATE_1,
		BYTE_STATE_2,
		BYTE_STATE_3,
		BYTE_STATE_4,
		BYTE_STATE_5,
		BYTE_STATE_6,
		BYTE_STATE_7,
		BYTE_STATE_8,
		BYTE_STATE_9,
		BYTE_STATE_10,
	  BYTE_STATE_11,
    BYTE_STATE_OFF = 0XFE,
} BYTE_STATES;


unsigned int  Gaspower_index = 0;  //命令序列号,未使用
unsigned char Single_bus_state= 0; //0空闲，1发送中，2接收中  总线状态
unsigned char Gaspowercontrol_send_flag = 0; //等待发送标志
unsigned char Single_wire_state= 0 ; //0空闲，1发送中，2接收中

unsigned char Gaspsend_delaycount= 0; //发送计时
unsigned char Gasprev_delaycount = 0; //接收计时

unsigned char EXTI4_15_Single_Wire_Pin_flag = 0; //总线发生中断

unsigned char gaspc_send[11] = {0xAA,11,2,31,10,15,0,0,0,1,0};//控制命令

unsigned char gaspc_rev[30] = {0xAA,11,2,31,10,15,1,1,1,1,0}; //设备返回命令

struct GASwall_Manage  GASwall_Manage[2] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
																						1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
																						};

//清空内存
void clear_arry(unsigned char *arry,unsigned char num)
{
    unsigned char i;
	  for(i=0;i<num;i++)
		{
			arry[i] = 0;
		}
}
//控制命令1填值
void Gaspowercontrol(unsigned char command,unsigned int dat,unsigned int masg_index)
{
    unsigned int i = 0;
	  unsigned int check_sum = 0;
		gaspc_send[2] = masg_index>>8;   //指令序号
		gaspc_send[3] = masg_index%256;
		gaspc_send[4] = command;   //指令码
	
		switch(command)
		{
			case GAS_SET_RSSI:					//信号连接状态
//				   gaspc_send[5] = 2;     //0：没有连接路由器（WiFi指示灯灭）
//																	//1：正在配网（WiFi指示灯快闪）
//																	//2：已连接上了路由器（WiFi指示灯慢闪）
//																	//3：已连接上了服务器（WiFi指示灯常亮）
				gaspc_send[5] = (unsigned char) dat;
				break;
			case GAS_SET_STATE:					//状态查询
			case GAS_SET_ONOFF:					//0:关机 1:开机
			case GAS_SET_MODE:					//1:夏季模式2：冬季模式					
			case GAS_SET_Bathroom_TEMP:	//30-75分别对应35℃-60℃						
			case GAS_SET_HEATINGTEMP:		//30-80分别对应30℃-xx℃					
			case GAS_SET_RST:						//1:复位请求					
			case GAS_SET_HOT_W_R:				//0:关闭1：开启					
			case GAS_SET_LOCK:					//0:关闭1：开启						
			case GAS_SET_ECO:						//0:ECO开启1：ECO关闭
				  Trigger_MQ_T(1); //第一步
			    live_flag_cunt = 600; //5分钟
					gaspc_send[5] = (unsigned char) dat;
				break;						
			case GAS_SET_NULL:
				
				break;
			case GAS_SET_TIME_CALIB:		//分钟0~59分别对应0~59分钟,小时0~24分别对应0~24小时
					 gaspc_send[5] = dat%100;   //参数0
					 gaspc_send[6] = dat/100;   //参数1
				break;											
			default:
				
				break;
		}
		//计算校验和
		for(i=1;i<9;i++)
		{
		 check_sum+= gaspc_send[i];
		}
		gaspc_send[9] = 	check_sum/256;
		gaspc_send[10] = check_sum%256;
		Gaspowercontrol_send_flag = 1; //发送等待
		
}

unsigned int  byte_send_state = 0;//字段发送状态机
unsigned int  bit_send_state = 0;//位段发送状态机
unsigned char DC_TX_flag;//正在发送标志
unsigned char DC_TX_state;//发送状态机
unsigned int  TX_PWM_DAT;//发送状态机


void Gaspsendwait(void)
//发送一个字 0/1
{
   switch(DC_TX_state)
   {
       case 0:  //开始并计时
					Gaspsend_delaycount = 0;//初始化计数器 
          DC_TX_state++;
           
       break;
       case 1:  
         if(Gaspsend_delaycount>=63) //第一个9msmS结束
         {
             Gaspsend_delaycount = 0;//初始化计数器
             DC_TX_state++;
         }
       break;
       case 2:  
         if(Gaspsend_delaycount>=7)
         {
							bit_send_state = 0;//下一个位
					    byte_send_state++;
              DC_TX_state = 0;
         }
       break;
       default: //完成一个字节发送
         DC_TX_state = 0;
         break;
   }
}
void GaspsendBoot(void)
//发送一个字 0/1
{
   switch(DC_TX_state)
   {
       case 0:  //开始并计时
					Gaspsend_delaycount = 0;//初始化计数器 
					HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin,GPIO_PIN_RESET); //低电平9ms
          DC_TX_state++;
           
       break;
       case 1:  
         if(Gaspsend_delaycount>=63) //第一个9msmS结束
         {
             Gaspsend_delaycount = 0;//初始化计数器
             HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin,GPIO_PIN_SET); //低电平1ms
             DC_TX_state++;
         }
       break;
       case 2:  
         if(Gaspsend_delaycount>=7)
         {
							bit_send_state = 0;//下一个位
					    byte_send_state++;
              DC_TX_state = 0;
					    //printf("已发送引导码: %d\r\n",Single_bus_state);	
         }
       break;
       default: //完成一个字节发送
         DC_TX_state = 0;
         break;
   }
}

unsigned char  Gaspsendbit(unsigned char dat)
{
    unsigned char returndat = 0;
	  switch(DC_TX_state)
   {
       case 0:  //开始并计时
					Gaspsend_delaycount = 0;//初始化计数器 
					HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin,GPIO_PIN_RESET); //低电平
          DC_TX_state++;
           
       break;
       case 1:   //低电平等待
				 if(dat)
				 {
						if(Gaspsend_delaycount>=21) //第一个9msmS结束
						 {
								 Gaspsend_delaycount = 0;//初始化计数器
								 HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin,GPIO_PIN_SET); //低电平1ms
								 DC_TX_state++;
						 }
				 }else
				 {
					 if(Gaspsend_delaycount>=7) //第一个9msmS结束
							 {
									 Gaspsend_delaycount = 0;//初始化计数器
									 HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin,GPIO_PIN_SET); //低电平3ms
									 DC_TX_state++;
							 }
				 }
       break;
       case 2:  
         if(Gaspsend_delaycount>=7)
         {
              
					    
					    if(bit_send_state<7)
								 bit_send_state++; //下一个位
							else  //字节结束
							{ 
							   bit_send_state = 0;
								 if(byte_send_state < BYTE_STATE_11)
								   byte_send_state++;
								 else   //数组结束
									 byte_send_state = BYTE_STATE_OFF;
							}
              DC_TX_state = 0;
							return returndat;
         }
       break;
       default: //完成一个字节发送
         DC_TX_state = 0;
         break;
   }
	 return returndat;
}

const char maphexstr[16] = {"0123456789ABCDEF"};
unsigned int  byte_rev_state = 0;//字段接收状态机
unsigned int  bit_rev_state = 0;//位段接收状态机
unsigned int  bit_rev_flag = 0;//位段接收标志
unsigned int  bit_rev_cunt = 0;//调试用
unsigned int  rev_outtime_cunt = 0;//总线超时标志
unsigned int  send_waittime_cunt = 0;//发送等待延时计数
//控制命令下发
void  Handle_Gaspowercontrol(void)
{
	 unsigned int sum;
	 switch(Single_bus_state) //根据总线状态处理
	 {
     	case BUSY_NULL:
		      //数据接收相关
			    if(EXTI4_15_Single_Wire_Pin_flag) //判断是否发生下降沿中断
					{
						EXTI4_15_Single_Wire_Pin_flag = 0;//清零标志
						Single_bus_state = BUSY_REV; //总线忙置位
						Gasprev_delaycount = 0;
						bit_rev_state = 0;//位段接收状态机
						//等待接收引导码
						//printf("总线空闲\r\n");
					}
			break;
			case BUSY_WAIT:		
					if(send_waittime_cunt == 0)
					{
						Single_bus_state = BUSY_SEND;
						Single_Wire_output(); //初始化端口
					}
			break;
		 case BUSY_REV:
					//中断等待
					if(EXTI4_15_Single_Wire_Pin_flag) //判断是否发生下降沿中断
					{
						EXTI4_15_Single_Wire_Pin_flag = 0;//清零标志
						Gasprev_delaycount = 0;
						bit_rev_flag = 1;  //等到接收确认
						rev_outtime_cunt = 0;//标志复位
						//等待接收下一个bit
					}else
					{
						//总线释放等待
						if((HAL_GPIO_ReadPin(Single_Wire_GPIO_Port,Single_Wire_Pin)==GPIO_PIN_SET)&&(bit_rev_flag)) //判断总线是否被释放
						{
							 if((Gasprev_delaycount > 55)&&(Gasprev_delaycount<85))  //判断为引导码
								{
										//printf("引导码%d\r",Gasprev_delaycount);
										byte_rev_state = 0;//字段接收状态机
										bit_rev_state = 0;//位段接收状态机
										bit_rev_cunt = 0;
								}
								else if((Gasprev_delaycount>4)&&(Gasprev_delaycount<12)) //单总线0接收
								{
										gaspc_rev[byte_rev_state] &= ~(1<< bit_rev_state);
										
									//printf("%d-%d,",Gasprev_delaycount,bit_rev_cunt);
									  
									  bit_rev_state++;
									  bit_rev_cunt++;
							      bit_rev_flag = 0; //接收处理完成
								}
								else if((Gasprev_delaycount>11)&&(Gasprev_delaycount<28)) //单总线1接收
								{
										gaspc_rev[byte_rev_state] |= (1<< bit_rev_state);
										
									  //printf("%d-%d,",Gasprev_delaycount,bit_rev_cunt);
									  bit_rev_state++;
									  bit_rev_cunt++;
							      bit_rev_flag = 0; //接收处理完成
									  
								}
								else
								{
//										if(Gasprev_delaycount>5)
//										{
//										printf("单总线接收异常: 计时数%d--位序号%d--字序号%d\r\n",Gasprev_delaycount,bit_rev_cunt,byte_rev_state);
//										Single_bus_state = BUSY_NULL; //总线标志空闲
//									  bit_rev_cunt = 0;
//										}
										//printf("异常: 计时数%d--位序号%d--字序号%d\r\n",Gasprev_delaycount,bit_rev_cunt,byte_rev_state);
										Single_bus_state = BUSY_NULL; //总线标志空闲
									  bit_rev_cunt = 0;
								}
								if(bit_rev_state>7)
								{
										bit_rev_state = 0;
									  //打印一个接收字节
										//printf("%d-%d; ",byte_rev_state,gaspc_rev[byte_rev_state]);
									  byte_rev_state++;//接收字节序号增加
									
									  if(byte_rev_state>=26)
										{
											  char *p;
												int size,i;
											
												byte_rev_state = 0;
											  //接收完成，校验
//											  for(i=0;i<26;i++)
//												{
//													printf("%d+",gaspc_rev[i]);
//												}
											  sum = 0;
											  for(i=1;i<24;i++)
												{
												  sum += gaspc_rev[i];
												}
												
												if(sum ==(gaspc_rev[24]*256+gaspc_rev[25]))
												{
														//数据转移
														p = (char*)&GASwall_Manage[0];
														size = sizeof(GASwall_Manage[0]);
														//printf("检验和+%d\r\n",sum);
													  //printf("流量+%d\r\n",gaspc_rev[12]);
//													  printf("火力+%d\r\n",gaspc_rev[19]);
													  //printf("故障+%d\r\n",gaspc_rev[20]);
														for(i=0;i<size;i++)
														{
																*(p+i) = gaspc_rev[i];
																if((i>=5)&&(i<=21))
																{
																	 GASwall_Manage[1].All_DAT[2*(i-5)+1] = maphexstr[gaspc_rev[i]/16];
																	 GASwall_Manage[1].All_DAT[2*(i-5)+2] = maphexstr[gaspc_rev[i]%16];
																}
														 }
														 GASwall_Manage[1].All_DAT[0]  = 0x22;
														 GASwall_Manage[1].All_DAT[47] = 0x22;
														 
														 if(debug_onoff.Gaspower)
														{
															printf(GASwall_Manage[1].All_DAT,30);
															
														}
														Trigger_MQ_T(2); //第2步
														send_waittime_cunt = 50;//设定延时
														
														if(Gaspowercontrol_send_flag)   
															Single_bus_state = BUSY_WAIT; //总线标志空闲
														else 
															Single_bus_state = BUSY_NULL; //总线标志空闲
												}
												else
												{
														Single_bus_state = BUSY_NULL; //总线标志空闲
														printf("校误+%d\r\n",sum);
												}
												//释放总线
//												if(Gaspowercontrol_send_flag)   
//													Single_bus_state = BUSY_WAIT; //总线标志空闲
//												else 
//													Single_bus_state = BUSY_NULL; //总线标志空闲
										}
								}
						}
					}
			break;
		 case BUSY_SEND:
		 		   switch(byte_send_state)
						{
						  case BYTE_STATE_0:  //发送引导码
								GaspsendBoot();  		
								break;
							case BYTE_STATE_1:  //发送数据
							case BYTE_STATE_2:
							case BYTE_STATE_3:
							case BYTE_STATE_4:
							case BYTE_STATE_5:
							case BYTE_STATE_6:
							case BYTE_STATE_7:
							case BYTE_STATE_8:
							case BYTE_STATE_9:
							case BYTE_STATE_10:
							case BYTE_STATE_11:
								switch(bit_send_state)
								{
									case 0:
									case 1:
									case 2:
									case 3:
									case 4:
									case 5:
									case 6:
									case 7:
										 Gaspsendbit((gaspc_send[byte_send_state-1])&(1<<bit_send_state));
									break;
							default:
										break;
								}
								break;
						case BYTE_STATE_OFF://结束发送
							
								Gaspower_index++;
						
						    if(debug_onoff.Gaspower)
								{
										printf("GAS已发");
								}
								
								
								Single_bus_state = BUSY_NULL; //总线标志空闲
								if(Gaspowercontrol_send_flag>0)
								Gaspowercontrol_send_flag--;//发送等待标志清空
								Single_Wire_input(); //初始化端口
								byte_send_state = 0; //发送字节标志清零
								bit_send_state = 0;  //发送位标志清零
								break;
						default:
								break;
						}
			break;
		default:
			break;
	 }
}

unsigned long CostofGas = 1;       //燃气积算，单位0.001m3
unsigned long CostofWater = 1;     //水量积算 单位L
unsigned long CostofElect = 1;     //电量积算 单位WH
unsigned char IntegrationFlag = 0; //积分标识
unsigned char IntegrationDelay = 0; //积分倒计时


char f(int n)
{
    return "0123456789ABCDEF"[n];
}

//燃气用量估算
void Handle_GasCost(void)
{
   unsigned int dat;

	 if(0) //0点清零
		{
		   
		
		}
		else
		{
		   if(IntegrationFlag) //定时处理
			 {
				 IntegrationFlag = 0; //清零标识
				 
				 dat = 108; //每个积分单位用气量，即0.0108 m3或者10.8 （0.001m3）
				 
				 dat *= GASwall_Manage[0].Firepower; //活力值0-100;
				 
				 CostofGas += dat; //
				 
				 if(GASwall_Manage[0].load_state & 0x20)//判断分段阀门
				 {
					    IntegrationDelay = 12; //6秒一次
					    
					    CostofWater += GASwall_Manage[0].Hot_water_flow/10;
					    
					    if(GASwall_Manage[0].Firepower)
					    CostofElect += 6;//单位0.5W*min  //用电
							else
							CostofElect += 1;//单位0.5W*min  //用电	
				 }
				 else
				 {
					    IntegrationDelay = 30; //15秒一次,
					    CostofWater += GASwall_Manage[0].Hot_water_flow/4;
					 
					    if(GASwall_Manage[0].Firepower)
					    CostofElect += 15;//单位0.5W*min  //用电
					    else
							CostofElect += 2;//单位0.5W*min  //用电	
				 }
				 dat =  (unsigned int)(CostofGas/1000);//单位变更为0.001m3
				 
				 if(debug_onoff.GasCost) 
				 {
						printf("火力:%d          流量:%d \r\n",GASwall_Manage[0].Firepower,GASwall_Manage[0].Hot_water_flow);
						printf("用气:%d*0.001M3  用水:%ld*0.1L \r\n",dat,CostofWater);
				 }
				 
				 //dat += 0x1234;
				 //用气量
					GASwall_Manage[1].All_DAT[38] = f((dat & 0x0f)%16);  
					dat /= 16;
					GASwall_Manage[1].All_DAT[37] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GASwall_Manage[1].All_DAT[36] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GASwall_Manage[1].All_DAT[35] = f((dat & 0x0f)%16);
				 //用水量
				  dat = CostofWater;
					GASwall_Manage[1].All_DAT[42] = f((dat & 0x0f)%16);  
					dat /= 16;
					GASwall_Manage[1].All_DAT[41] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GASwall_Manage[1].All_DAT[40] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GASwall_Manage[1].All_DAT[39] = f((dat & 0x0f)%16);
				 
				  //用电
					dat =  (unsigned int)(CostofElect/60);//单位变更为WH
					GASwall_Manage[1].All_DAT[46] = f((dat & 0x0f)%16);  
					dat /= 16;
					GASwall_Manage[1].All_DAT[45] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GASwall_Manage[1].All_DAT[44] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GASwall_Manage[1].All_DAT[43] = f((dat & 0x0f)%16);
			 }
		}
}

