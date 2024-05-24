
//ȼ���ڹ�¯
#include "global.h"

typedef enum
{
    BUSY_NULL=0,        //����
	  BUSY_WAIT,          //�ȴ�
		BUSY_SEND,        	//����æ
	  BUSY_REV            //����æ
}   BUS_STATES;

typedef enum
{
	  BYTE_STATE_0 = 0,   //������
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


unsigned int  Gaspower_index = 0;  //�������к�,δʹ��
unsigned char Single_bus_state= 0; //0���У�1�����У�2������  ����״̬
unsigned char Gaspowercontrol_send_flag = 0; //�ȴ����ͱ�־
unsigned char Single_wire_state= 0 ; //0���У�1�����У�2������

unsigned char Gaspsend_delaycount= 0; //���ͼ�ʱ
unsigned char Gasprev_delaycount = 0; //���ռ�ʱ

unsigned char EXTI4_15_Single_Wire_Pin_flag = 0; //���߷����ж�

unsigned char gaspc_send[11] = {0xAA,11,2,31,10,15,0,0,0,1,0};//��������

unsigned char gaspc_rev[30] = {0xAA,11,2,31,10,15,1,1,1,1,0}; //�豸��������

struct GASwall_Manage  GASwall_Manage[2] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
																						1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
																						};

//����ڴ�
void clear_arry(unsigned char *arry,unsigned char num)
{
    unsigned char i;
	  for(i=0;i<num;i++)
		{
			arry[i] = 0;
		}
}
//��������1��ֵ
void Gaspowercontrol(unsigned char command,unsigned int dat,unsigned int masg_index)
{
    unsigned int i = 0;
	  unsigned int check_sum = 0;
		gaspc_send[2] = masg_index>>8;   //ָ�����
		gaspc_send[3] = masg_index%256;
		gaspc_send[4] = command;   //ָ����
	
		switch(command)
		{
			case GAS_SET_RSSI:					//�ź�����״̬
//				   gaspc_send[5] = 2;     //0��û������·������WiFiָʾ����
//																	//1������������WiFiָʾ�ƿ�����
//																	//2������������·������WiFiָʾ��������
//																	//3�����������˷�������WiFiָʾ�Ƴ�����
				gaspc_send[5] = (unsigned char) dat;
				break;
			case GAS_SET_STATE:					//״̬��ѯ
			case GAS_SET_ONOFF:					//0:�ػ� 1:����
			case GAS_SET_MODE:					//1:�ļ�ģʽ2������ģʽ					
			case GAS_SET_Bathroom_TEMP:	//30-75�ֱ��Ӧ35��-60��						
			case GAS_SET_HEATINGTEMP:		//30-80�ֱ��Ӧ30��-xx��					
			case GAS_SET_RST:						//1:��λ����					
			case GAS_SET_HOT_W_R:				//0:�ر�1������					
			case GAS_SET_LOCK:					//0:�ر�1������						
			case GAS_SET_ECO:						//0:ECO����1��ECO�ر�
				  Trigger_MQ_T(1); //��һ��
			    live_flag_cunt = 600; //5����
					gaspc_send[5] = (unsigned char) dat;
				break;						
			case GAS_SET_NULL:
				
				break;
			case GAS_SET_TIME_CALIB:		//����0~59�ֱ��Ӧ0~59����,Сʱ0~24�ֱ��Ӧ0~24Сʱ
					 gaspc_send[5] = dat%100;   //����0
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
		gaspc_send[9] = 	check_sum/256;
		gaspc_send[10] = check_sum%256;
		Gaspowercontrol_send_flag = 1; //���͵ȴ�
		
}

unsigned int  byte_send_state = 0;//�ֶη���״̬��
unsigned int  bit_send_state = 0;//λ�η���״̬��
unsigned char DC_TX_flag;//���ڷ��ͱ�־
unsigned char DC_TX_state;//����״̬��
unsigned int  TX_PWM_DAT;//����״̬��


void Gaspsendwait(void)
//����һ���� 0/1
{
   switch(DC_TX_state)
   {
       case 0:  //��ʼ����ʱ
					Gaspsend_delaycount = 0;//��ʼ�������� 
          DC_TX_state++;
           
       break;
       case 1:  
         if(Gaspsend_delaycount>=63) //��һ��9msmS����
         {
             Gaspsend_delaycount = 0;//��ʼ��������
             DC_TX_state++;
         }
       break;
       case 2:  
         if(Gaspsend_delaycount>=7)
         {
							bit_send_state = 0;//��һ��λ
					    byte_send_state++;
              DC_TX_state = 0;
         }
       break;
       default: //���һ���ֽڷ���
         DC_TX_state = 0;
         break;
   }
}
void GaspsendBoot(void)
//����һ���� 0/1
{
   switch(DC_TX_state)
   {
       case 0:  //��ʼ����ʱ
					Gaspsend_delaycount = 0;//��ʼ�������� 
					HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin,GPIO_PIN_RESET); //�͵�ƽ9ms
          DC_TX_state++;
           
       break;
       case 1:  
         if(Gaspsend_delaycount>=63) //��һ��9msmS����
         {
             Gaspsend_delaycount = 0;//��ʼ��������
             HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin,GPIO_PIN_SET); //�͵�ƽ1ms
             DC_TX_state++;
         }
       break;
       case 2:  
         if(Gaspsend_delaycount>=7)
         {
							bit_send_state = 0;//��һ��λ
					    byte_send_state++;
              DC_TX_state = 0;
					    //printf("�ѷ���������: %d\r\n",Single_bus_state);	
         }
       break;
       default: //���һ���ֽڷ���
         DC_TX_state = 0;
         break;
   }
}

unsigned char  Gaspsendbit(unsigned char dat)
{
    unsigned char returndat = 0;
	  switch(DC_TX_state)
   {
       case 0:  //��ʼ����ʱ
					Gaspsend_delaycount = 0;//��ʼ�������� 
					HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin,GPIO_PIN_RESET); //�͵�ƽ
          DC_TX_state++;
           
       break;
       case 1:   //�͵�ƽ�ȴ�
				 if(dat)
				 {
						if(Gaspsend_delaycount>=21) //��һ��9msmS����
						 {
								 Gaspsend_delaycount = 0;//��ʼ��������
								 HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin,GPIO_PIN_SET); //�͵�ƽ1ms
								 DC_TX_state++;
						 }
				 }else
				 {
					 if(Gaspsend_delaycount>=7) //��һ��9msmS����
							 {
									 Gaspsend_delaycount = 0;//��ʼ��������
									 HAL_GPIO_WritePin(Single_Wire_GPIO_Port, Single_Wire_Pin,GPIO_PIN_SET); //�͵�ƽ3ms
									 DC_TX_state++;
							 }
				 }
       break;
       case 2:  
         if(Gaspsend_delaycount>=7)
         {
              
					    
					    if(bit_send_state<7)
								 bit_send_state++; //��һ��λ
							else  //�ֽڽ���
							{ 
							   bit_send_state = 0;
								 if(byte_send_state < BYTE_STATE_11)
								   byte_send_state++;
								 else   //�������
									 byte_send_state = BYTE_STATE_OFF;
							}
              DC_TX_state = 0;
							return returndat;
         }
       break;
       default: //���һ���ֽڷ���
         DC_TX_state = 0;
         break;
   }
	 return returndat;
}

const char maphexstr[16] = {"0123456789ABCDEF"};
unsigned int  byte_rev_state = 0;//�ֶν���״̬��
unsigned int  bit_rev_state = 0;//λ�ν���״̬��
unsigned int  bit_rev_flag = 0;//λ�ν��ձ�־
unsigned int  bit_rev_cunt = 0;//������
unsigned int  rev_outtime_cunt = 0;//���߳�ʱ��־
unsigned int  send_waittime_cunt = 0;//���͵ȴ���ʱ����
//���������·�
void  Handle_Gaspowercontrol(void)
{
	 unsigned int sum;
	 switch(Single_bus_state) //��������״̬����
	 {
     	case BUSY_NULL:
		      //���ݽ������
			    if(EXTI4_15_Single_Wire_Pin_flag) //�ж��Ƿ����½����ж�
					{
						EXTI4_15_Single_Wire_Pin_flag = 0;//�����־
						Single_bus_state = BUSY_REV; //����æ��λ
						Gasprev_delaycount = 0;
						bit_rev_state = 0;//λ�ν���״̬��
						//�ȴ�����������
						//printf("���߿���\r\n");
					}
			break;
			case BUSY_WAIT:		
					if(send_waittime_cunt == 0)
					{
						Single_bus_state = BUSY_SEND;
						Single_Wire_output(); //��ʼ���˿�
					}
			break;
		 case BUSY_REV:
					//�жϵȴ�
					if(EXTI4_15_Single_Wire_Pin_flag) //�ж��Ƿ����½����ж�
					{
						EXTI4_15_Single_Wire_Pin_flag = 0;//�����־
						Gasprev_delaycount = 0;
						bit_rev_flag = 1;  //�ȵ�����ȷ��
						rev_outtime_cunt = 0;//��־��λ
						//�ȴ�������һ��bit
					}else
					{
						//�����ͷŵȴ�
						if((HAL_GPIO_ReadPin(Single_Wire_GPIO_Port,Single_Wire_Pin)==GPIO_PIN_SET)&&(bit_rev_flag)) //�ж������Ƿ��ͷ�
						{
							 if((Gasprev_delaycount > 55)&&(Gasprev_delaycount<85))  //�ж�Ϊ������
								{
										//printf("������%d\r",Gasprev_delaycount);
										byte_rev_state = 0;//�ֶν���״̬��
										bit_rev_state = 0;//λ�ν���״̬��
										bit_rev_cunt = 0;
								}
								else if((Gasprev_delaycount>4)&&(Gasprev_delaycount<12)) //������0����
								{
										gaspc_rev[byte_rev_state] &= ~(1<< bit_rev_state);
										
									//printf("%d-%d,",Gasprev_delaycount,bit_rev_cunt);
									  
									  bit_rev_state++;
									  bit_rev_cunt++;
							      bit_rev_flag = 0; //���մ������
								}
								else if((Gasprev_delaycount>11)&&(Gasprev_delaycount<28)) //������1����
								{
										gaspc_rev[byte_rev_state] |= (1<< bit_rev_state);
										
									  //printf("%d-%d,",Gasprev_delaycount,bit_rev_cunt);
									  bit_rev_state++;
									  bit_rev_cunt++;
							      bit_rev_flag = 0; //���մ������
									  
								}
								else
								{
//										if(Gasprev_delaycount>5)
//										{
//										printf("�����߽����쳣: ��ʱ��%d--λ���%d--�����%d\r\n",Gasprev_delaycount,bit_rev_cunt,byte_rev_state);
//										Single_bus_state = BUSY_NULL; //���߱�־����
//									  bit_rev_cunt = 0;
//										}
										//printf("�쳣: ��ʱ��%d--λ���%d--�����%d\r\n",Gasprev_delaycount,bit_rev_cunt,byte_rev_state);
										Single_bus_state = BUSY_NULL; //���߱�־����
									  bit_rev_cunt = 0;
								}
								if(bit_rev_state>7)
								{
										bit_rev_state = 0;
									  //��ӡһ�������ֽ�
										//printf("%d-%d; ",byte_rev_state,gaspc_rev[byte_rev_state]);
									  byte_rev_state++;//�����ֽ��������
									
									  if(byte_rev_state>=26)
										{
											  char *p;
												int size,i;
											
												byte_rev_state = 0;
											  //������ɣ�У��
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
														//����ת��
														p = (char*)&GASwall_Manage[0];
														size = sizeof(GASwall_Manage[0]);
														//printf("�����+%d\r\n",sum);
													  //printf("����+%d\r\n",gaspc_rev[12]);
//													  printf("����+%d\r\n",gaspc_rev[19]);
													  //printf("����+%d\r\n",gaspc_rev[20]);
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
														Trigger_MQ_T(2); //��2��
														send_waittime_cunt = 50;//�趨��ʱ
														
														if(Gaspowercontrol_send_flag)   
															Single_bus_state = BUSY_WAIT; //���߱�־����
														else 
															Single_bus_state = BUSY_NULL; //���߱�־����
												}
												else
												{
														Single_bus_state = BUSY_NULL; //���߱�־����
														printf("У��+%d\r\n",sum);
												}
												//�ͷ�����
//												if(Gaspowercontrol_send_flag)   
//													Single_bus_state = BUSY_WAIT; //���߱�־����
//												else 
//													Single_bus_state = BUSY_NULL; //���߱�־����
										}
								}
						}
					}
			break;
		 case BUSY_SEND:
		 		   switch(byte_send_state)
						{
						  case BYTE_STATE_0:  //����������
								GaspsendBoot();  		
								break;
							case BYTE_STATE_1:  //��������
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
						case BYTE_STATE_OFF://��������
							
								Gaspower_index++;
						
						    if(debug_onoff.Gaspower)
								{
										printf("GAS�ѷ�");
								}
								
								
								Single_bus_state = BUSY_NULL; //���߱�־����
								if(Gaspowercontrol_send_flag>0)
								Gaspowercontrol_send_flag--;//���͵ȴ���־���
								Single_Wire_input(); //��ʼ���˿�
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

unsigned long CostofGas = 1;       //ȼ�����㣬��λ0.001m3
unsigned long CostofWater = 1;     //ˮ������ ��λL
unsigned long CostofElect = 1;     //�������� ��λWH
unsigned char IntegrationFlag = 0; //���ֱ�ʶ
unsigned char IntegrationDelay = 0; //���ֵ���ʱ


char f(int n)
{
    return "0123456789ABCDEF"[n];
}

//ȼ����������
void Handle_GasCost(void)
{
   unsigned int dat;

	 if(0) //0������
		{
		   
		
		}
		else
		{
		   if(IntegrationFlag) //��ʱ����
			 {
				 IntegrationFlag = 0; //�����ʶ
				 
				 dat = 108; //ÿ�����ֵ�λ����������0.0108 m3����10.8 ��0.001m3��
				 
				 dat *= GASwall_Manage[0].Firepower; //����ֵ0-100;
				 
				 CostofGas += dat; //
				 
				 if(GASwall_Manage[0].load_state & 0x20)//�жϷֶη���
				 {
					    IntegrationDelay = 12; //6��һ��
					    
					    CostofWater += GASwall_Manage[0].Hot_water_flow/10;
					    
					    if(GASwall_Manage[0].Firepower)
					    CostofElect += 6;//��λ0.5W*min  //�õ�
							else
							CostofElect += 1;//��λ0.5W*min  //�õ�	
				 }
				 else
				 {
					    IntegrationDelay = 30; //15��һ��,
					    CostofWater += GASwall_Manage[0].Hot_water_flow/4;
					 
					    if(GASwall_Manage[0].Firepower)
					    CostofElect += 15;//��λ0.5W*min  //�õ�
					    else
							CostofElect += 2;//��λ0.5W*min  //�õ�	
				 }
				 dat =  (unsigned int)(CostofGas/1000);//��λ���Ϊ0.001m3
				 
				 if(debug_onoff.GasCost) 
				 {
						printf("����:%d          ����:%d \r\n",GASwall_Manage[0].Firepower,GASwall_Manage[0].Hot_water_flow);
						printf("����:%d*0.001M3  ��ˮ:%ld*0.1L \r\n",dat,CostofWater);
				 }
				 
				 //dat += 0x1234;
				 //������
					GASwall_Manage[1].All_DAT[38] = f((dat & 0x0f)%16);  
					dat /= 16;
					GASwall_Manage[1].All_DAT[37] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GASwall_Manage[1].All_DAT[36] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GASwall_Manage[1].All_DAT[35] = f((dat & 0x0f)%16);
				 //��ˮ��
				  dat = CostofWater;
					GASwall_Manage[1].All_DAT[42] = f((dat & 0x0f)%16);  
					dat /= 16;
					GASwall_Manage[1].All_DAT[41] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GASwall_Manage[1].All_DAT[40] = f((dat & 0x0f)%16);  
				  dat /= 16;
					GASwall_Manage[1].All_DAT[39] = f((dat & 0x0f)%16);
				 
				  //�õ�
					dat =  (unsigned int)(CostofElect/60);//��λ���ΪWH
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

