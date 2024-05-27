//ȼ���ڹ�¯
#include "Gaspower.h"

/*��־λ*/ 
uint8_t  Single_Wire_Pin_Rise_flag=0;//�ⲿ�������жϱ�־
uint8_t  Single_Wire_Pin_Falling_flag=0;//�ⲿ�½����жϱ�־
uint16_t  Gas_rev_timercnt = 0; //�����߽��ռ���
uint8_t  siglewire_wait_cnt = 0;//���͵ȴ���ʱ����
uint8_t  bit_start_rev_flag = 0;//λ�ν��ձ�־


/*״̬��*/
//uint8_t Gaspowercontrol_send_flag = 0; //�ȴ����ͱ�־ 
uint8_t  	byte_send_state = 0;//�ֶη���״̬��
uint8_t  	bit_send_state = 0;//λ�η���״̬��
uint32_t  byte_rev_state = 0;//�ֶν���״̬��
uint32_t  bit_rev_state = 0;//λ�ν���״̬��

uint8_t 	Single_Tx_State=0;//����״̬�� 
uint8_t 	Single_Bus_State= 0; //����״̬��0���У�1�ȴ���2�����У�3������ 

//16�����ַ�����
const char hex_to_str[16] = {"0123456789ABCDEF"};


/*�շ�����*/
uint8_t gaspc_send[GAS_SEND_SIZE] = {0xAA,GAS_SEND_SIZE,0,0,0,0,0,0,0,0,0}; //����������ָ��
uint8_t gaspc_rev[26] = {0};  //���յ��ĵ������ϱ�ָ��
S_GasPower_Manage  GasPower_Manage = {0};//�ڹ�¯�ظ������ݽṹ��
uint16_t  Gaspower_index = 0;  //�����������к� 
//char   Gaspower_Ascii[50];            //����ƴ��Ϊ�ַ���  
uint8_t 	error_code_pre; //֮ǰ�Ĵ������

//�ⲿ�жϺ���
void EXTI4_15_IRQHandler(void)
{ 
//		static uint16_t rev_lastTimestamp = 0; 
		    // ����Ƿ�Ϊָ�����ŵ��ж�
    if (__HAL_GPIO_EXTI_GET_IT(Single_Wire_Pin) != RESET)
    {
        // ����жϱ�־
        __HAL_GPIO_EXTI_CLEAR_IT(Single_Wire_Pin);
        // ��⵱ǰ����״̬�����������غ��½���
        if (HAL_GPIO_ReadPin(Single_Wire_GPIO_Port, Single_Wire_Pin) == GPIO_PIN_RESET)
        {
            // �½��ش����Ĳ���
           Single_Wire_Pin_Falling_flag = 1;
						__HAL_TIM_SET_COUNTER(&htim3, 0); //  ���㶨ʱ��
//						rev_lastTimestamp= __HAL_TIM_GET_COUNTER(&htim3);
        }
        else
        {
            // �����ش����Ĳ���
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



//����������ֵ
void Gaspower_Command(uint8_t command,uint16_t dat,uint16_t masg_index)
{
		uint8_t gaspc_send[GAS_SEND_SIZE] = {0xAA,GAS_SEND_SIZE,0,0,0,0,0,0,0,0,0};
    uint8_t i = 0;
	  uint16_t check_sum = 0;
		gaspc_send[2] = masg_index>>8;   //ָ�����
		gaspc_send[3] = masg_index%256;
		gaspc_send[4] = command;   //ָ����
	
		switch(command)
		{
			case GAS_SET_RSSI:				//�ź�����״̬
				//0��û������·������WiFiָʾ����
				//1������������WiFiָʾ�ƿ�����
				//2������������·������WiFiָʾ��������
				//3�����������˷�������WiFiָʾ�Ƴ�����					
				gaspc_send[5] = (uint8_t) dat;
				break;
		
			case GAS_SET_STATE:					//״̬��ѯ
			case GAS_SET_ONOFF:					//0:�ػ� 1:����
			case GAS_SET_MODE:					//1:�ļ�ģʽ2������ģʽ					
			case GAS_SET_BATHROOM_TEMP:	//30-75�ֱ��Ӧ35��-60��						
			case GAS_SET_HEATINGTEMP:		//30-80�ֱ��Ӧ30��-xx��					
			case GAS_SET_RST:						//1:��λ����					
			case GAS_SET_HOT_WATER_CIRC://0:�ر�1������ ��ˮѭ��					
			case GAS_SET_LOCK:					//0:�ر�1������						
			case GAS_SET_ECO:						//0:ECO����1��ECO�ر� 
					gaspc_send[5] = (uint8_t) dat;
				break;						
//			case GAS_SET_NULL:
//				break;
			case GAS_SET_TIME_CALIB:		//dat:2456��Ӧ24:56,����0~59�ֱ��Ӧ0~59����,Сʱ0~24�ֱ��Ӧ0~24Сʱ
					gaspc_send[5] = dat%100;   //����0 01,34  3401
					 gaspc_send[6] = dat/100;   //����1 
				break;											
			default:
				break;
		}
		//����У���
		for(i=1;i<9;i++)
		{
			check_sum+= gaspc_send[i];
		}
		gaspc_send[9]  = 	check_sum/256;
		gaspc_send[10] = check_sum%256;
		GasFiFo_Push(gaspc_send);//�������
//		Gaspowercontrol_send_flag = 1; //���͵ȴ�
		
}


// ʱ�����ȡ�ͱȽ�
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

//    *last_timestamp = current_time; // ����ʱ���
    return time_elapsed;
}
 


//����������
void GaspsendBoot(void)
{
//    static uint16_t last_timestamp = 0; // ��һ��ʱ���
    uint16_t elapsed_time; // ������ʱ��
    switch(Single_Tx_State)
    {
        case 0: // ��ʼ����ʱ
            HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin, GPIO_PIN_RESET); // �͵�ƽ
//            last_timestamp = __HAL_TIM_GET_COUNTER(&htim3); // ��ȡ��ǰʱ���
						__HAL_TIM_SET_COUNTER(&htim3, 0); // ���㶨ʱ��
            Single_Tx_State++;
            break;

        case 1:
//					elapsed_time = GetElapsedTime(&last_timestamp);
						elapsed_time=__HAL_TIM_GET_COUNTER(&htim3);
            if(elapsed_time >= 630) // 9ms*70%
            {
                HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin, GPIO_PIN_SET); // �ߵ�ƽ
//              last_timestamp = __HAL_TIM_GET_COUNTER(&htim3);  
							__HAL_TIM_SET_COUNTER(&htim3, 0); // �ٴ����㶨ʱ��

                Single_Tx_State++;
            }
						break;
        case 2:
//					elapsed_time = GetElapsedTime(&last_timestamp);
						elapsed_time=__HAL_TIM_GET_COUNTER(&htim3);
            if(elapsed_time >= 70) // 1ms*70%
            {
                bit_send_state = 0; // ��һ��λ
                byte_send_state++;	// ��һ���ֽ�
                Single_Tx_State = 0;

                //debug_printf("�ѷ���������: %d\r\n",Single_Bus_State); 
            } 
            break;

        default: // ���һ���ֽڷ���
            Single_Tx_State = 0;
            break;
    }
}




//����1bit
void Gaspsendbit(uint8_t dat)
{
//    static uint16_t last_timestamp = 0; // ��һ��ʱ���
//    uint16_t elapsed_time; // ������ʱ��

    switch(Single_Tx_State)
    {
        case 0: // ��ʼ����ʱ
            HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin, GPIO_PIN_RESET); // �͵�ƽ
            __HAL_TIM_SET_COUNTER(&htim3, 0); // ���㶨ʱ��
            Single_Tx_State++;
            break;

        case 1: // �͵�ƽ�ȴ�
//            elapsed_time = GetElapsedTime(&last_timestamp);
            if(dat)
            {
                if(__HAL_TIM_GET_COUNTER(&htim3) >= 210) // 3ms
                {		
                    HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin, GPIO_PIN_SET); // �ߵ�ƽ
										__HAL_TIM_SET_COUNTER(&htim3, 0); // �ٴ����㶨ʱ��
                    Single_Tx_State++;
                }
            }
            else
            {
                if(__HAL_TIM_GET_COUNTER(&htim3) >= 70) // 1ms
                {
                    HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin, GPIO_PIN_SET); // �ߵ�ƽ
                    __HAL_TIM_SET_COUNTER(&htim3, 0); // �ٴ����㶨ʱ��
										Single_Tx_State++;
                }
            }
            break;

        case 2:
//            elapsed_time = GetElapsedTime(&last_timestamp);
            if(__HAL_TIM_GET_COUNTER(&htim3) >= 70) // 1ms
            {
                if(bit_send_state < 7)
                    bit_send_state++; // ��һ��λ
                else // �ֽڽ���
                { 
                    bit_send_state = 0;
                    if(byte_send_state < BYTE_SEND_STATE_11)
                        byte_send_state++;
                    else // �������
                        byte_send_state = BYTE_SEND_STATE_OFF;
                }
                Single_Tx_State = 0;
            }
            break;

        default: // ���һ���ֽڷ���
            Single_Tx_State = 0;
            break;
    }
}




//�ڹ�¯�շ�����
void Handle_Gaspower(void)
{
	 switch(Single_Bus_State) //��������״̬����
	 {
			case BUS_NULL:
					// ���ݽ������
					if (Single_Wire_Pin_Falling_flag) // �ж��Ƿ����½����ж�
					{
							Single_Wire_Pin_Falling_flag = 0; // �����־
							Single_Bus_State = BUS_REV; // ����æ��λ
						  bit_start_rev_flag=1;
					}
			break;
		 case BUS_REV:
					// �жϵȴ�
				 if(Single_Wire_Pin_Falling_flag)
					{
							Single_Wire_Pin_Falling_flag = 0; // �����־
						 if(bit_start_rev_flag==0)
								bit_start_rev_flag=1;//��ʼ���յ��͵�ƽ
						 else 
							 bit_start_rev_flag=0;
					}
		
					if (Single_Wire_Pin_Rise_flag) // �ж��Ƿ����������ж�
					{
						Single_Wire_Pin_Rise_flag = 0; // �����־
						if(bit_start_rev_flag==1)
						{
							bit_start_rev_flag=0;
							// ������յ�����
							if (Gas_rev_timercnt > 550 && Gas_rev_timercnt < 850) // �ж�Ϊ������
							{
									byte_rev_state = 0; // �ֶν���״̬��
									bit_rev_state = 0; // λ�ν���״̬��
							}
							else if (Gas_rev_timercnt > 40 && Gas_rev_timercnt < 120) // �����߽���0
							{
									gaspc_rev[byte_rev_state] &= ~(1 << bit_rev_state);
									bit_rev_state++;
							}
							else if (Gas_rev_timercnt > 110 && Gas_rev_timercnt < 280) // �����߽���1
							{
									gaspc_rev[byte_rev_state] |= (1 << bit_rev_state);
									bit_rev_state++;
							}
							else // �����߽����쳣 
							{
									if(debug_onoff.Gaspower)
											debug_printf("�����߽����쳣");
									Single_Bus_State = BUS_NULL; //���߱�־����
							}
							
							if(bit_rev_state>7)
							{
									uint8_t i;
									bit_rev_state = 0;
									//��ӡһ�������ֽ�
//									debug_printf("%d-%d; ",byte_rev_state,gaspc_rev[byte_rev_state]);
									byte_rev_state++;//�����ֽ��������
								  
								 //���ճ��ȴﵽ26
									if(byte_rev_state>=26)
									{
										
											uint16_t sum=0;
											byte_rev_state = 0;
											//������ɣ�У��
//											  for(i=0;i<26;i++)
//												{
//													//debug_printf("%d+",gaspc_rev[i]);
//												}
											for(i=1;i<24;i++)
											{
												sum += gaspc_rev[i];
											}
											//�Ƚ�У���
											if(sum ==(gaspc_rev[24]*256+gaspc_rev[25]))
											{
													if(gaspc_rev[4]=='H')
													{
															//����ת�Ƶ��ṹ��
															memcpy(&GasPower_Manage,gaspc_rev,sizeof(S_GasPower_Manage)-50);
														
//														debug_printf("�豸״̬:%d,��ԡ�¶�:%d,��ů�¶�:%d,����:%d \r\n",
//														GasPower_Manage.on_off,GasPower_Manage.Set_Bathroomtemp,GasPower_Manage.Set_Heating_temp,GasPower_Manage.ERRO_CODE);
															//hexת��ascii��
															for(i=5;i<=21;i++)
															 {
																		 GasPower_Manage.ALL_DATA[2*(i-5)+1] = hex_to_str[gaspc_rev[i]/16];
																		 GasPower_Manage.ALL_DATA[2*(i-5)+2] = hex_to_str[gaspc_rev[i]%16];
															 }
															 GasPower_Manage.ALL_DATA[0]  = 0x22;//"
															 GasPower_Manage.ALL_DATA[47] = 0x22;//"
													}
														
													siglewire_wait_cnt = 150;//�趨��ʱ
																						
													if(GasFifo_Size()>0)  //��������������� 
													{
														Single_Bus_State = BUS_WAIT; //���ߵȴ�
														GasFifo_Pop(gaspc_send);//�ӻ���ȡ����gaspc_send
													}
													else 
														Single_Bus_State = BUS_NULL; //���߿���
													
											}
											else
											{
													Single_Bus_State = BUS_NULL; //���߱�־����
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
							SetPinMode(OUTPUT_MODE); //���ģʽ
					}
			break;					
		 case BUS_SEND:
		 		   switch(byte_send_state)
						{
						  case BYTE_SEND_STATE_0:  //����������
									GaspsendBoot();  		
									break;
							case BYTE_SEND_STATE_1:  //��������
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
							case BYTE_SEND_STATE_OFF://��������
									Gaspower_index++;//�����������к�++
									if(debug_onoff.Gaspower)
									{
											if(debug_onoff.Gaspower)
													debug_printf("GAS�ѷ�,�����ϱ�\r\n");
									}
									
									Data_Rapid_Reply();//�����ϱ�����
									SetPinMode(INPUT_MODE); //�˿�����Ϊ�����ж�
									Single_Bus_State = BUS_NULL; //���߿��У��ȴ��ظ�
									byte_send_state = 0; //�����ֽڱ�־����
									bit_send_state = 0;  //����λ��־����
									break;
							default:
								break;
						}
			break;
			default:
			break;
	 }
}

uint64_t CostofGas = 0;       //ȼ�����㣬��λ0.001m3
uint64_t CostofWater = 0;     //ˮ������ ��λL
uint64_t CostofElect = 0;     //�������� ��λWH
uint8_t IntegrationFlag = 0; //���ֱ�ʶ
uint8_t IntegrationDelay = 0; //���ֵ���ʱ


char f(int n)
{
    return "0123456789ABCDEF"[n];
}

//ȼ����������
void Handle_GasCost(void)
{
   uint32_t dat;

	 if(0) //0������
		{
		   
		
		}
		else
		{
		   if(IntegrationFlag) //��ʱ����
			 {
				 IntegrationFlag = 0; //�����ʶ
				 
				 dat = 108; //ÿ�����ֵ�λ����������0.0108 m3����10.8 ��0.001m3��
				 
				 dat *= GasPower_Manage.Firepower; //����ֵ0-100;
				 
				 CostofGas += dat; //
				 
				 if(GasPower_Manage.load_state & 0x20)//�жϷֶη���
				 {
					    IntegrationDelay = 12; //6��һ��
					    
					    CostofWater += GasPower_Manage.Hot_water_flow/10;
					    
					    if(GasPower_Manage.Firepower)
					    CostofElect += 6;//��λ0.5W*min  //�õ�
							else
							CostofElect += 1;//��λ0.5W*min  //�õ�	
				 }
				 else
				 {
					    IntegrationDelay = 30; //15��һ��,
					    CostofWater += GasPower_Manage.Hot_water_flow/4;
					 
					    if(GasPower_Manage.Firepower)
					    CostofElect += 15;//��λ0.5W*min  //�õ�
					    else
							CostofElect += 2;//��λ0.5W*min  //�õ�	
				 }
				 dat =  (uint32_t)(CostofGas/1000);//��λ���Ϊ0.001m3
				 
//				 if(debug_onoff.GasCost) 
//				 {
//						//debug_printf("����:%d          ����:%d \r\n",GasPower_Manage.Firepower,GasPower_Manage.Hot_water_flow);
//						//debug_printf("����:%d*0.001M3  ��ˮ:%ld*0.1L \r\n",dat,CostofWater);
//				 }
				 
				 //dat += 0x1234;
				 //������
					GasPower_Manage.ALL_DATA[38] = f((dat & 0x0f)%16);  
					dat /= 16;
					GasPower_Manage.ALL_DATA[37] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GasPower_Manage.ALL_DATA[36] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GasPower_Manage.ALL_DATA[35] = f((dat & 0x0f)%16);
				 //��ˮ��
				  dat = CostofWater;
					GasPower_Manage.ALL_DATA[42] = f((dat & 0x0f)%16);  
					dat /= 16;
					GasPower_Manage.ALL_DATA[41] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GasPower_Manage.ALL_DATA[40] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GasPower_Manage.ALL_DATA[39] = f((dat & 0x0f)%16);
				 
				  //�õ�
					dat =  (uint32_t)(CostofElect/60);//��λ���ΪWH
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


