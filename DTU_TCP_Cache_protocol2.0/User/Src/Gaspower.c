//燃气壁挂炉
#include "Gaspower.h"

/*标志位*/ 
uint8_t  Single_Wire_Pin_Rise_flag=0;//外部上升沿中断标志
uint8_t  Single_Wire_Pin_Falling_flag=0;//外部下降沿中断标志
uint16_t  Gas_rev_timercnt = 0; //单总线接收计数
uint8_t  siglewire_wait_cnt = 0;//发送等待延时计数
uint8_t  bit_start_rev_flag = 0;//位段接收标志


/*状态机*/
//uint8_t Gaspowercontrol_send_flag = 0; //等待发送标志 
uint8_t  	byte_send_state = 0;//字段发送状态机
uint8_t  	bit_send_state = 0;//位段发送状态机
uint32_t  byte_rev_state = 0;//字段接收状态机
uint32_t  bit_rev_state = 0;//位段接收状态机

uint8_t 	Single_Tx_State=0;//发送状态机 
uint8_t 	Single_Bus_State= 0; //总线状态：0空闲，1等待，2发送中，3接收中 

//16进制字符数组
const char hex_to_str[16] = {"0123456789ABCDEF"};


/*收发缓存*/
uint8_t gaspc_send[GAS_SEND_SIZE] = {0xAA,GAS_SEND_SIZE,0,0,0,0,0,0,0,0,0}; //单总线设置指令
uint8_t gaspc_rev[26] = {0};  //接收到的单总线上报指令
S_GasPower_Manage  GasPower_Manage = {0};//壁挂炉回复的数据结构体
uint16_t  Gaspower_index = 0;  //发送命令序列号 
//char   Gaspower_Ascii[50];            //数据拼接为字符串  
uint8_t 	error_code_pre; //之前的错误代码

//外部中断函数
void EXTI4_15_IRQHandler(void)
{ 
//		static uint16_t rev_lastTimestamp = 0; 
		    // 检查是否为指定引脚的中断
    if (__HAL_GPIO_EXTI_GET_IT(Single_Wire_Pin) != RESET)
    {
        // 清除中断标志
        __HAL_GPIO_EXTI_CLEAR_IT(Single_Wire_Pin);
        // 检测当前引脚状态来区分上升沿和下降沿
        if (HAL_GPIO_ReadPin(Single_Wire_GPIO_Port, Single_Wire_Pin) == GPIO_PIN_RESET)
        {
            // 下降沿触发的操作
           Single_Wire_Pin_Falling_flag = 1;
						__HAL_TIM_SET_COUNTER(&htim3, 0); //  清零定时器
//						rev_lastTimestamp= __HAL_TIM_GET_COUNTER(&htim3);
        }
        else
        {
            // 上升沿触发的操作
						 Single_Wire_Pin_Rise_flag = 1;
							Gas_rev_timercnt = __HAL_TIM_GET_COUNTER(&htim3);
//						uint16_t currentTimestamp = __HAL_TIM_GET_COUNTER(&htim3);
//						if (currentTimestamp < rev_lastTimestamp)
//						{
//								Gas_rev_timercnt = (currentTimestamp + 10000-1) - rev_lastTimestamp;
//						}
//						else
//						{
//								Gas_rev_timercnt = currentTimestamp - rev_lastTimestamp;
//						}            
        }
    }
		
  HAL_GPIO_EXTI_IRQHandler(Single_Wire_Pin);
}



//控制命令填值
void Gaspower_Command(uint8_t command,uint16_t dat,uint16_t masg_index)
{
		uint8_t gaspc_send[GAS_SEND_SIZE] = {0xAA,GAS_SEND_SIZE,0,0,0,0,0,0,0,0,0};
    uint8_t i = 0;
	  uint16_t check_sum = 0;
		gaspc_send[2] = masg_index>>8;   //指令序号
		gaspc_send[3] = masg_index%256;
		gaspc_send[4] = command;   //指令码
	
		switch(command)
		{
			case GAS_SET_RSSI:				//信号连接状态
				//0：没有连接路由器（WiFi指示灯灭）
				//1：正在配网（WiFi指示灯快闪）
				//2：已连接上了路由器（WiFi指示灯慢闪）
				//3：已连接上了服务器（WiFi指示灯常亮）					
				gaspc_send[5] = (uint8_t) dat;
				break;
		
			case GAS_SET_STATE:					//状态查询
			case GAS_SET_ONOFF:					//0:关机 1:开机
			case GAS_SET_MODE:					//1:夏季模式2：冬季模式					
			case GAS_SET_BATHROOM_TEMP:	//30-75分别对应35℃-60℃						
			case GAS_SET_HEATINGTEMP:		//30-80分别对应30℃-xx℃					
			case GAS_SET_RST:						//1:复位请求					
			case GAS_SET_HOT_WATER_CIRC://0:关闭1：开启 热水循环					
			case GAS_SET_LOCK:					//0:关闭1：开启						
			case GAS_SET_ECO:						//0:ECO开启1：ECO关闭 
					gaspc_send[5] = (uint8_t) dat;
				break;						
//			case GAS_SET_NULL:
//				break;
			case GAS_SET_TIME_CALIB:		//dat:2456对应24:56,分钟0~59分别对应0~59分钟,小时0~24分别对应0~24小时
					gaspc_send[5] = dat%100;   //参数0 01,34  3401
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
		gaspc_send[9]  = 	check_sum/256;
		gaspc_send[10] = check_sum%256;
		GasFiFo_Push(gaspc_send);//存入队列
//		Gaspowercontrol_send_flag = 1; //发送等待
		
}


// 时间戳读取和比较
uint16_t GetElapsedTime(uint16_t *last_timestamp)
{
    uint16_t current_time = __HAL_TIM_GET_COUNTER(&htim3);
    uint16_t time_elapsed;

    if (current_time < *last_timestamp)
    {
        time_elapsed = (current_time + 10000-1) - *last_timestamp;
    }
    else
    {
        time_elapsed = current_time - *last_timestamp;
    }

//    *last_timestamp = current_time; // 更新时间戳
    return time_elapsed;
}
 


//发送引导码
void GaspsendBoot(void)
{
//    static uint16_t last_timestamp = 0; // 上一个时间戳
    uint16_t elapsed_time; // 经过的时间
    switch(Single_Tx_State)
    {
        case 0: // 开始并计时
            HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin, GPIO_PIN_RESET); // 低电平
//            last_timestamp = __HAL_TIM_GET_COUNTER(&htim3); // 获取当前时间戳
						__HAL_TIM_SET_COUNTER(&htim3, 0); // 清零定时器
            Single_Tx_State++;
            break;

        case 1:
//					elapsed_time = GetElapsedTime(&last_timestamp);
						elapsed_time=__HAL_TIM_GET_COUNTER(&htim3);
            if(elapsed_time >= 630) // 9ms*70%
            {
                HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin, GPIO_PIN_SET); // 高电平
//              last_timestamp = __HAL_TIM_GET_COUNTER(&htim3);  
							__HAL_TIM_SET_COUNTER(&htim3, 0); // 再次清零定时器

                Single_Tx_State++;
            }
						break;
        case 2:
//					elapsed_time = GetElapsedTime(&last_timestamp);
						elapsed_time=__HAL_TIM_GET_COUNTER(&htim3);
            if(elapsed_time >= 70) // 1ms*70%
            {
                bit_send_state = 0; // 下一个位
                byte_send_state++;	// 下一个字节
                Single_Tx_State = 0;

                //debug_printf("已发送引导码: %d\r\n",Single_Bus_State); 
            } 
            break;

        default: // 完成一个字节发送
            Single_Tx_State = 0;
            break;
    }
}




//发送1bit
void Gaspsendbit(uint8_t dat)
{
//    static uint16_t last_timestamp = 0; // 上一个时间戳
//    uint16_t elapsed_time; // 经过的时间

    switch(Single_Tx_State)
    {
        case 0: // 开始并计时
            HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin, GPIO_PIN_RESET); // 低电平
            __HAL_TIM_SET_COUNTER(&htim3, 0); // 清零定时器
            Single_Tx_State++;
            break;

        case 1: // 低电平等待
//            elapsed_time = GetElapsedTime(&last_timestamp);
            if(dat)
            {
                if(__HAL_TIM_GET_COUNTER(&htim3) >= 210) // 3ms
                {		
                    HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin, GPIO_PIN_SET); // 高电平
										__HAL_TIM_SET_COUNTER(&htim3, 0); // 再次清零定时器
                    Single_Tx_State++;
                }
            }
            else
            {
                if(__HAL_TIM_GET_COUNTER(&htim3) >= 70) // 1ms
                {
                    HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin, GPIO_PIN_SET); // 高电平
                    __HAL_TIM_SET_COUNTER(&htim3, 0); // 再次清零定时器
										Single_Tx_State++;
                }
            }
            break;

        case 2:
//            elapsed_time = GetElapsedTime(&last_timestamp);
            if(__HAL_TIM_GET_COUNTER(&htim3) >= 70) // 1ms
            {
                if(bit_send_state < 7)
                    bit_send_state++; // 下一个位
                else // 字节结束
                { 
                    bit_send_state = 0;
                    if(byte_send_state < BYTE_SEND_STATE_11)
                        byte_send_state++;
                    else // 数组结束
                        byte_send_state = BYTE_SEND_STATE_OFF;
                }
                Single_Tx_State = 0;
            }
            break;

        default: // 完成一个字节发送
            Single_Tx_State = 0;
            break;
    }
}




//壁挂炉收发命令
void Handle_Gaspower(void)
{
	 switch(Single_Bus_State) //根据总线状态处理
	 {
			case BUS_NULL:
					// 数据接收相关
					if (Single_Wire_Pin_Falling_flag) // 判断是否发生下降沿中断
					{
							Single_Wire_Pin_Falling_flag = 0; // 清零标志
							Single_Bus_State = BUS_REV; // 总线忙置位
						  bit_start_rev_flag=1;
					}
			break;
		 case BUS_REV:
					// 中断等待
				 if(Single_Wire_Pin_Falling_flag)
					{
							Single_Wire_Pin_Falling_flag = 0; // 清零标志
						 if(bit_start_rev_flag==0)
								bit_start_rev_flag=1;//开始接收到低电平
						 else 
							 bit_start_rev_flag=0;
					}
		
					if (Single_Wire_Pin_Rise_flag) // 判断是否发生上升沿中断
					{
						Single_Wire_Pin_Rise_flag = 0; // 清零标志
						if(bit_start_rev_flag==1)
						{
							bit_start_rev_flag=0;
							// 处理接收的数据
							if (Gas_rev_timercnt > 550 && Gas_rev_timercnt < 850) // 判断为引导码
							{
									byte_rev_state = 0; // 字段接收状态机
									bit_rev_state = 0; // 位段接收状态机
							}
							else if (Gas_rev_timercnt > 40 && Gas_rev_timercnt < 120) // 单总线接收0
							{
									gaspc_rev[byte_rev_state] &= ~(1 << bit_rev_state);
									bit_rev_state++;
							}
							else if (Gas_rev_timercnt > 110 && Gas_rev_timercnt < 280) // 单总线接收1
							{
									gaspc_rev[byte_rev_state] |= (1 << bit_rev_state);
									bit_rev_state++;
							}
							else // 单总线接收异常 
							{
									if(debug_onoff.Gaspower)
											debug_printf("单总线接收异常");
									Single_Bus_State = BUS_NULL; //总线标志空闲
							}
							
							if(bit_rev_state>7)
							{
									uint8_t i;
									bit_rev_state = 0;
									//打印一个接收字节
//									debug_printf("%d-%d; ",byte_rev_state,gaspc_rev[byte_rev_state]);
									byte_rev_state++;//接收字节序号增加
								  
								 //接收长度达到26
									if(byte_rev_state>=26)
									{
										
											uint16_t sum=0;
											byte_rev_state = 0;
											//接收完成，校验
//											  for(i=0;i<26;i++)
//												{
//													//debug_printf("%d+",gaspc_rev[i]);
//												}
											for(i=1;i<24;i++)
											{
												sum += gaspc_rev[i];
											}
											//比较校验和
											if(sum ==(gaspc_rev[24]*256+gaspc_rev[25]))
											{
													if(gaspc_rev[4]=='H')
													{
															//数据转移到结构体
															memcpy(&GasPower_Manage,gaspc_rev,sizeof(S_GasPower_Manage)-50);
														
//														debug_printf("设备状态:%d,卫浴温度:%d,采暖温度:%d,故障:%d \r\n",
//														GasPower_Manage.on_off,GasPower_Manage.Set_Bathroomtemp,GasPower_Manage.Set_Heating_temp,GasPower_Manage.ERRO_CODE);
															//hex转成ascii码
															for(i=5;i<=21;i++)
															 {
																		 GasPower_Manage.ALL_DATA[2*(i-5)+1] = hex_to_str[gaspc_rev[i]/16];
																		 GasPower_Manage.ALL_DATA[2*(i-5)+2] = hex_to_str[gaspc_rev[i]%16];
															 }
															 GasPower_Manage.ALL_DATA[0]  = 0x22;//"
															 GasPower_Manage.ALL_DATA[47] = 0x22;//"
													}
														
													siglewire_wait_cnt = 150;//设定延时
																						
													if(GasFifo_Size()>0)  //控制命令缓存有数据 
													{
														Single_Bus_State = BUS_WAIT; //总线等待
														GasFifo_Pop(gaspc_send);//从缓存取出到gaspc_send
													}
													else 
														Single_Bus_State = BUS_NULL; //总线空闲
													
											}
											else
											{
													Single_Bus_State = BUS_NULL; //总线标志空闲
											}
									}
							}
						}
					}
			break;
			case BUS_WAIT:		
					if(siglewire_wait_cnt == 0)
					{
							Single_Bus_State = BUS_SEND;
							SetPinMode(OUTPUT_MODE); //输出模式
					}
			break;					
		 case BUS_SEND:
		 		   switch(byte_send_state)
						{
						  case BYTE_SEND_STATE_0:  //发送引导码
									GaspsendBoot();  		
									break;
							case BYTE_SEND_STATE_1:  //发送数据
							case BYTE_SEND_STATE_2:
							case BYTE_SEND_STATE_3:
							case BYTE_SEND_STATE_4:
							case BYTE_SEND_STATE_5:
							case BYTE_SEND_STATE_6:
							case BYTE_SEND_STATE_7:
							case BYTE_SEND_STATE_8:
							case BYTE_SEND_STATE_9:
							case BYTE_SEND_STATE_10:
							case BYTE_SEND_STATE_11:
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
							case BYTE_SEND_STATE_OFF://结束发送
									Gaspower_index++;//发送命令序列号++
									if(debug_onoff.Gaspower)
									{
											if(debug_onoff.Gaspower)
													debug_printf("GAS已发,快速上报\r\n");
									}
									
									Data_Rapid_Reply();//快速上报数据
									SetPinMode(INPUT_MODE); //端口设置为输入中断
									Single_Bus_State = BUS_NULL; //总线空闲，等待回复
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

uint64_t CostofGas = 0;       //燃气积算，单位0.001m3
uint64_t CostofWater = 0;     //水量积算 单位L
uint64_t CostofElect = 0;     //电量积算 单位WH
uint8_t IntegrationFlag = 0; //积分标识
uint8_t IntegrationDelay = 0; //积分倒计时


char f(int n)
{
    return "0123456789ABCDEF"[n];
}

//燃气用量估算
void Handle_GasCost(void)
{
   uint32_t dat;

	 if(0) //0点清零
		{
		   
		
		}
		else
		{
		   if(IntegrationFlag) //定时处理
			 {
				 IntegrationFlag = 0; //清零标识
				 
				 dat = 108; //每个积分单位用气量，即0.0108 m3或者10.8 （0.001m3）
				 
				 dat *= GasPower_Manage.Firepower; //活力值0-100;
				 
				 CostofGas += dat; //
				 
				 if(GasPower_Manage.load_state & 0x20)//判断分段阀门
				 {
					    IntegrationDelay = 12; //6秒一次
					    
					    CostofWater += GasPower_Manage.Hot_water_flow/10;
					    
					    if(GasPower_Manage.Firepower)
					    CostofElect += 6;//单位0.5W*min  //用电
							else
							CostofElect += 1;//单位0.5W*min  //用电	
				 }
				 else
				 {
					    IntegrationDelay = 30; //15秒一次,
					    CostofWater += GasPower_Manage.Hot_water_flow/4;
					 
					    if(GasPower_Manage.Firepower)
					    CostofElect += 15;//单位0.5W*min  //用电
					    else
							CostofElect += 2;//单位0.5W*min  //用电	
				 }
				 dat =  (uint32_t)(CostofGas/1000);//单位变更为0.001m3
				 
//				 if(debug_onoff.GasCost) 
//				 {
//						//debug_printf("火力:%d          流量:%d \r\n",GasPower_Manage.Firepower,GasPower_Manage.Hot_water_flow);
//						//debug_printf("用气:%d*0.001M3  用水:%ld*0.1L \r\n",dat,CostofWater);
//				 }
				 
				 //dat += 0x1234;
				 //用气量
					GasPower_Manage.ALL_DATA[38] = f((dat & 0x0f)%16);  
					dat /= 16;
					GasPower_Manage.ALL_DATA[37] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GasPower_Manage.ALL_DATA[36] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GasPower_Manage.ALL_DATA[35] = f((dat & 0x0f)%16);
				 //用水量
				  dat = CostofWater;
					GasPower_Manage.ALL_DATA[42] = f((dat & 0x0f)%16);  
					dat /= 16;
					GasPower_Manage.ALL_DATA[41] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GasPower_Manage.ALL_DATA[40] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GasPower_Manage.ALL_DATA[39] = f((dat & 0x0f)%16);
				 
				  //用电
					dat =  (uint32_t)(CostofElect/60);//单位变更为WH
					GasPower_Manage.ALL_DATA[46] = f((dat & 0x0f)%16);  
					dat /= 16;
					GasPower_Manage.ALL_DATA[45] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GasPower_Manage.ALL_DATA[44] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GasPower_Manage.ALL_DATA[43] = f((dat & 0x0f)%16);
			 }
		}
}


