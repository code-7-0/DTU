/********************************************************************************
* @file    	  lte.c
* @function   LTEģ�� AT��������
* @brief 	    ��3.5.0�汾�⽨�Ĺ���ģ��
* @attention	
* @version 	  V1.1
********************************************************************************/
#include "global.h"

#define LTE_POWER_ON_BEFOR_WAIT_TIME 2000 										//LTE�����ȴ�ʱ��
#define LTE_POWER_ON_WAIT_TIME 3000 										//LTE�����ȴ�ʱ��
#define SIGNALMIN 15                                    //�ź�������ͷ�ֵ
#define SIGNALMAX 31                                    //�ź�������ͷ�ֵ
#define SOCKET_BUF_SIZE 128                             //Socket���ݻ����С
char cSocketRecvBuf[SOCKET_BUF_SIZE] = {0};             //socket���ݽ��ջ���
char cSocketSendBuf[SOCKET_BUF_SIZE] = {0};             //socket���ݷ��ͻ���
uint8_t ucStateNum = 0;                                 //����ִ��˳���ʶֵ
uint8_t retrytimes = 0;                                 //�������Դ���
uint16_t ucErrorTimes = 0;                               //��������ۼ�ֵ
uint8_t ucFlightModeTimes = 0;													//�������ģʽ����

uint8_t ero_time; //�����������2����ת
char subs_keyword[50]; //���յ�������
char subs_dat[50];     //���յ����ַ�������


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description:��λLTEģ��
 * @param None
 * @return None
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int module_reset(void)
{
    switch (ucStateNum)
    {
    //����PEN����
    case 0://+= PEN_GPIO_SET_LOW;
		    HAL_GPIO_WritePin(GPIOB, EC600_PWK_Pin, GPIO_PIN_SET);
        ucStateNum++;
        break;
    case 1://����PEN����
        if (wait_timeout(1000))
        {
					  HAL_GPIO_WritePin(GPIOB, EC600_PWK_Pin, GPIO_PIN_RESET);
            ucStateNum = 0;
            return 1;
        }
        break;
    default:
        break;
    }
    return 0;
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description:
 * @param str��   Ҫ�������ַ���
 * @param minval��Ҫƥ���ź�����������Сֵ
 * @param minval��Ҫƥ���ź������������ֵ
 * @return 0:�ź�������������������״̬, 1:�ź�����������������״̬
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int match_csq(char *str, int minval, int maxval)
{
    int lpCsqVal = 0;
    char tmp[5] = {0};
    char *p = NULL, *q = NULL;
    p = strstr(str, "+CSQ:");
    if (p == NULL)
    {
        return 0;
    }
    p = p + 5;
    while (*p < 0x30 || *p > 0x39)
    {
        p++;
    }
    q = p;
    while (*p != ',')
    {
        p++;
    }
    memcpy(tmp, q, p - q);
    lpCsqVal = atoi(tmp);
    /* �ж��ź������Ƿ������õ������� */
    if (lpCsqVal >= minval && lpCsqVal <= maxval)
    {
        return 1;
    }
    return 0;
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description: ���ģ�鹤��״̬�Ƿ����
 * @param None
 * @return 0�����δ��ɣ�MD_OK��ģ���Ѿ�����MD_ERR�����󣬲����㹤��״̬
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int module_is_ready(void)
{
    switch (ucStateNum)
    {
    case 0x00://�ر�AT�������
        if (fat_send_wait_cmdres_blocking("ATE0\r\n", 1000))
        {
						//�յ�OK
            if (fat_cmdres_keyword_matching("OK"))
            {
                ucErrorTimes = 0;
                ucStateNum++;
            }
            else
            {
								//����10�εò�����ȷӦ��
                if (ucErrorTimes++ > 10)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    case 0x01://����
        if (fat_send_wait_cmdres_blocking("AT+CPIN?\r\n", 1000))
        {
						//�յ�+CPIN: READY
            if (fat_cmdres_keyword_matching("+CPIN: READY"))
            {
                ucErrorTimes = 0;
                ucStateNum++;
            }
            else
            {     
								//����10�εò�����ȷӦ��
                if (ucErrorTimes++ > 10)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    case 0x02: //��ѯ�ź�����
        if (fat_send_wait_cmdres_blocking("AT+CSQ\r\n", 2000))
        {
						//�յ�OK
            if (fat_cmdres_keyword_matching("OK"))
            {
							//�յ�����99����Ƶ�ź�δ��ʼ����
							if (fat_cmdres_keyword_matching("+CSQ: 99,99"))
								{
										//����30�εò�����ȷӦ��
										if (ucErrorTimes++ > 30)
										{
												ucStateNum = MD_ERR;
										}
								}
              else
                {    
										//�ź�ֵ��SIGNALMIN~SIGNALMAX�������
                    if (match_csq(USART1_RX_BUF, SIGNALMIN, SIGNALMAX))
                    {
                        ucErrorTimes = 0;
												ucStateNum++;
                    }
										else 
										{
												ucStateNum = MD_ERR;
										}
                }
            }
						//û�յ�Ӧ��
            else
            {
								//����30�β�Ӧ��
                if (ucErrorTimes++ > 30)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    case 0x03://�鿴��ǰGPRS����״̬
        if (fat_send_wait_cmdres_blocking("AT+CGATT?\r\n", 1000))
        {
						//�յ�+CGATT: 1
            if (fat_cmdres_keyword_matching("+CGATT: 1"))
            {
                ucErrorTimes = 0;
								ucStateNum = MD_OK;
            }
            else
            {   
								//����30�εò�����ȷӦ��
                if (ucErrorTimes++ > 30)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
		case MD_ERR:  //������������ģʽ
				ucStateNum = 0;
				return MD_ERR;
    //���
    case MD_OK:
				ucStateNum = 0;
        return MD_OK;
    default:
        break;
    }
    return 0;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//===========================================��λ����===================================
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void clear_bufer(char *arry,unsigned int dat)
{
    unsigned int i;
	for(i =0;i<dat;i++)
	{
	    arry[i] = 0;
	}
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description: ���Ӻ�������ó�ʼ��
 * @param None
 * @return 0�����δ��ɣ�MD_OK��ģ���Ѿ�����MD_ERR�����󣬲����㹤��״̬
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int module_connet_parm_init(void)
{
    char bufer[255];
	  switch (ucStateNum)
    {
		//���ý���ģʽ:�ӷ��������յ� MQTT ��Ϣ���� URC ����ʽ�ϱ�
    case 0x00:
				//����ǰ����Ʒ�������ִ�д������������
				if(strcmp(Device_Flash.mqttHostUrl,"a15dLSmBATk.iot-as-mqtt.cn-shanghai.aliyuncs.com") ==0)
				{
						if (fat_send_wait_cmdres_blocking("AT+QMTCFG=\"ssl\",0,1,0\r\n", 1000))
						{
								//�յ�OK
								if (fat_cmdres_keyword_matching("OK"))
								{
										ucErrorTimes = 0;
										ucStateNum++;
								}
								else
								{
										//����10�εò�����ȷӦ��
										if (ucErrorTimes++ > 10)
										{
												ucStateNum = MD_ERR;
										}
								}
						}
				}else
				{
				  ucStateNum++;
				}	
        break;
    //��MQTT�ͻ�������
    case 0x01:
			  //ƴ������
			  clear_bufer(bufer,256);
		    strcat(bufer,"AT+QMTOPEN=0,\"");
		    strcat(bufer,Device_Flash.mqttHostUrl);
		    strcat(bufer,"\",");
		    strcat(bufer,Device_Flash.port);
		    strcat(bufer,"\r\n");
		    if (fat_send_wait_cmdres_blocking(bufer, 500))
        {
						//�յ�+QMTOPEN: 0,0
            if (fat_cmdres_keyword_matching("+QMTOPEN: 0,0"))
            {
                ucErrorTimes = 0;
                ucStateNum++;
            }
            else
            {
								//����10�εò�����ȷӦ��
                if (ucErrorTimes++ > 10)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    case 0x02:  //��MQTT�ͻ������ӷ�����
			  //ƴ������
			  clear_bufer(bufer,256);
		    strcat(bufer,"AT+QMTCONN=0,\"");
		    strcat(bufer,Device_Flash.clientId);
		    strcat(bufer,"\",\"");
		    strcat(bufer,Device_Flash.username);
		    strcat(bufer,"\",\"");
		    strcat(bufer,Device_Flash.passwd);
		    strcat(bufer,"\"\r\n");
		    if (fat_send_wait_cmdres_blocking(bufer, 500))
				{
						//�յ�+QMTCONN: 0,0,0
            if (fat_cmdres_keyword_matching("+QMTCONN: 0,0,0"))
            {
                ucErrorTimes = 0;
                ucStateNum++;
							  ucStateNum++; //��������case 0x04:
            }
            else
            {
								//����10�εò�����ȷӦ��
                if (ucErrorTimes++ > 10)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    case 0x03://��������1
		    //ƴ������
			  clear_bufer(bufer,256);
		    strcat(bufer,"AT+QMTSUB=0,1,\"");
		    strcat(bufer,Device_Flash.QMTSUB1); 
		    strcat(bufer,"\",0\r\n");
		    if (fat_send_wait_cmdres_blocking(bufer, 500))
				{
					//�յ�+QMTSUB: 0,1,0,1
          if (fat_cmdres_keyword_matching("+QMTSUB: 0,1,0,"))
            {
                ucErrorTimes = 0;
                ucStateNum ++;
            }
            else
            {
						 //����5�εò�����ȷӦ��
                if (ucErrorTimes++ > 5)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
		case 0x04:  //��������2
		    //ƴ������
			  clear_bufer(bufer,256);
		    strcat(bufer,"AT+QMTSUB=0,1,\"");
		    strcat(bufer,Device_Flash.QMTSUB2); 
		    strcat(bufer,"\",0\r\n");
		    if (fat_send_wait_cmdres_blocking(bufer, 500))
				{
					//�յ�+QMTSUB: 0,1,0,1
          if (fat_cmdres_keyword_matching("+QMTSUB: 0,1,0,"))
            {
                ucErrorTimes = 0;
                ucStateNum = MD_OK;
            }
            else
            {
						 //����5�εò�����ȷӦ��
                if (ucErrorTimes++ > 5)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    case MD_OK://���
				ucStateNum = 0;
        return MD_OK;
    case MD_ERR://������������ģʽ
				ucStateNum = 0;
        return MD_ERR;
    default:
        break;
    }
    return 0;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description: ��ȡsocket���ջ�������Topic
 * @param cmdres��������Ӧ����
 * @param recvbuf��socket���ջ���
 * @return ����Topic�ĳ���
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int match_recv_topic(char *cmdres, char *recvbuf)
{
    int length = 0;
    char *p = NULL, *q = NULL;
    p = strstr(cmdres, "+QMTRECV: 0,0,\"");
    if (p == NULL)
    {
        return 0;
    }
    p = p + 15;
    q = p;
    while (*p != 0x2C)
    {
        p++;
    }
    length = p - q - 1;
    memset(recvbuf, 0, SOCKET_BUF_SIZE);
    memcpy(recvbuf, q, length);
		
    return length;
}


int match_recv_data(char *cmdres)
{
    int length = 0;
	  //unsigned char i;
    char *p = NULL, *q = NULL;
	
	  memset(subs_keyword,0,50); //�����ڴ�
		memset(subs_dat,0,50); //�����ڴ�
	
		//��λ��Ӧ���OK
    p = strstr(cmdres, "}}");
	
    if (p == NULL)
    {
        return 0;
    }
		//��ȡ����
		//p = p-1; //�����ŵ���Ҫ���Ӹ���
		q = p;
		do
		{
        p--;
    }
    while (*p != 0x3A);//��ѯ����ַ
		
    length = q - p - 1;	
		
		if(length>50) length = 50; //���ƴ�С
    memcpy(subs_dat, p + 1, length);
		printf("data:%s=",subs_dat);  //{"params":{"Summer_winterMode_Set":2}}
		
		//��ȡ����
		p -= 1;
		q = p;
		do
		{
        p--;
    }
    while (*p != 0x22);
    length = q - p - 1;	
		
		if(length>50) length = 50; //���ƴ�С
    memcpy(subs_keyword, p + 1, length);
		printf("%s\r",subs_keyword);
    return 1;
}

/**
 * @brief �ϱ�����PowerSwitch���ƶ�
 * @param dm_handle��dm���, ��������void *
 * @return ��Ϣid:(>=1), �ϱ�ʧ��: <0
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description: �����շ�����
 * @return 0�����δ��ɣ�MD_ERR������
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int module_data(void)
{
    switch (ucStateNum)
    {
    //�������ⷢ������
			
			case SUB_MESG_GET:
				   ucStateNum++;
			break;
				
			case SUB_SEND1:
				//ƴ������
			  //printf("������");
				if(topic_state)
				{
					if(fat_send_wait_cmdres_blocking(topic_buf1,100))			
					{
							ucStateNum++;
					}
				}else
				{
				   ucStateNum = SUB_CK_SOCKET; //û�д����������ת���������
					 //printf("NO\r");
				}
        break;
    case SUB_SEND2:

				if(fat_send_wait_cmdres_nonblocking(topic_buf2,6000))
				{
						ucStateNum++;
					  topic_state_last = topic_state;//��¼��ǰ����ֱ��
				    topic_state = 0;
					  topic_buf_ok = 0; //������ɱ�ʶ
					  topic_count++;
        }
        break;
		
		case SUB_CHECK:
				//�յ�OK
				if (fat_cmdres_keyword_matching("OK")||(fat_cmdres_keyword_matching("OK")))
				{
						printf("����<OK>\r");
						ucErrorTimes = 0;
						ucStateNum = SUB_WAIT; //���ȷ���
					  ero_time = 0;
					  
					 Gaspowercontrol(GAS_SET_RSSI,3,Gaspower_index);
				}
				//����3�εò�����ȷӦ��
				if (ucErrorTimes++ > 3)
						{
							ero_time++;
  						if(ero_time>2)	 
							ucStateNum = MD_ERR;
						}
        break;
		
    //��ѯsocket�����Ƿ�������
    case SUB_CK_SOCKET:
				if (fat_send_wait_cmdres_blocking("AT+QMTRECV?\r",1500))
        {
						//�յ�+QMTRECV:
            //if (fat_cmdres_keyword_matching("+QMTRECV:"))
					  if(fat_cmdres_keyword_matching("post_reply"))
            { 
								//�鿴����
	              printf("����<OK>");
							  if(match_recv_data(USART1_RX_BUF))
								{
                    handle_MQTTConvert();
								}
            }
            else
            {
								ucErrorTimes = 0;
							  ucStateNum = SUB_SEND1; //û�н��յ�����鿴�Ƿ���Ҫ����
            }
        }
        break;
		//��ȡTopic��Data
    case SUB_CK_TOPIC:
//		    if (fat_send_wait_cmdres_blocking("AT+QMTRECV=0\r\n", 1000))
//				{
//						if (fat_cmdres_keyword_matching("+QMTRECV:"))
//						{
////								//��ȡ������
////								ret = match_recv_topic(USART1_RX_BUF, cSocketRecvBuf);
////								printf("socket recv topic:%d , %s\r",ret , cSocketRecvBuf);
////								//��ȡ��������
////								ret = match_recv_data(USART1_RX_BUF, cSocketRecvBuf);

////								printf("socket recv data:%d , %s\r",ret , cSocketRecvBuf);
//							  
//								//��ȡ������
//								ret = match_recv_topic(USART1_RX_BUF, cSocketRecvBuf);
//								printf("socket recv topic:%d , %s\r",ret , cSocketRecvBuf);
//								
//								ucErrorTimes = 0;
//								ucStateNum = 0x00;
//						}
//						else
//						{
//								//����3�εò�����ȷӦ��
//                if (ucErrorTimes++ > 3)
//                {
//                    ucStateNum = SUB_WAIT;
//                }
//						}
//				}
//				break;
		case SUB_WAIT:
				//�յ�OK
		    if(wait_timeout(50))
				{
						//printf("WT");
					  ucStateNum = SUB_SEND1;
					//ucStateNum = SUB_CK_SOCKET;
				}
        break;
		//������������ģʽ
    case SUB_ERR:
				ucStateNum = 0;
		    if(debug_onoff.EC600N)  printf("����&&\r");
		    
        return MD_ERR;
    default:
        break;
    }
    return 0;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description: ����ģʽ������
 * @param None
 * @return 0�����δ��ɣ�MD_WORK_STA_CHK�����¿�����������ģ��״̬��⣻MD_ERR������
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int module_flightmode()
{
		switch (ucStateNum)
    {
			case 0x00:
				ucFlightModeTimes++;
				ucStateNum++;
				if(debug_onoff.EC600N)  printf("FlightTime��%d\r\n",ucFlightModeTimes);
				break;
			case 0x01:
				if (ucFlightModeTimes == 2)
				{
						if(debug_onoff.EC600N)  printf("���η���ģʽ����λģ��\r\n");
						ucStateNum = MD_ERR;
				}
				else
					ucStateNum++;
				break;
			case 0x02:
        if (fat_send_wait_cmdres_blocking("AT+CFUN=0\r\n", 2000))
        {
						//�յ�OK
            if (fat_cmdres_keyword_matching("OK"))
            {
								ucErrorTimes = 0;
                ucStateNum++;
            }
						else
            {
								//����5�εò�����ȷӦ������MD_ERR
                if (ucErrorTimes++ > 5)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
			case 0x03:
        if (wait_timeout(5000))
						ucStateNum++;
        break;
			case 0x04:
        if (fat_send_wait_cmdres_blocking("AT+CFUN=1\r\n", 2000))
        {
						//�յ�OK,״̬��ΪMD_WORK_STA_CHK������ģ��״̬���
            if (fat_cmdres_keyword_matching("OK"))
            {
								if(debug_onoff.EC600N)   printf("�ٿ�������\r\n");
								ucStateNum = 0;
								return MD_WORK_STA_CHK;
            }
						else
            {
								//����5�εò�����ȷӦ������MD_ERR
                if (ucErrorTimes++ > 5)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
			case MD_ERR:
				ucStateNum = 0;
				return MD_ERR;
			default:
        break;
    }
    return 0;	
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description: LTEģ��MQTTЭ��
 * @param None
 * @return None
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
unsigned int outlinetimes = 0;
void module_MQTT(void)
{
    static int state = MD_RESET;
    int ret = 0;
    switch (state)
    {
			
			case MD_WAIT://ģ���ϵ�ȴ�
				if(wait_timeout(LTE_POWER_ON_BEFOR_WAIT_TIME)) //�ȴ�2��
				{
						state = MD_RESET;
				}
				break;
			case MD_RESET://��λģ�飬�ȴ�5��
        if (module_reset())
        {
						wait_timeout(LTE_POWER_ON_WAIT_TIME); //��λ�ȴ�//5s
					  //if(debug_onoff.EC600N)   
							printf("Reset-%d-!!!!!!!!!!!!!!!!!!!!!!\r\n",outlinetimes);
						outlinetimes++;
					  state = MD_AT_REQ;
					  Gaspowercontrol(GAS_SET_RSSI,1,Gaspower_index);
        }
        break;
    case MD_AT_REQ://AT����
        if (fat_send_wait_cmdres_blocking("AT\r\n", 500))
        {
            if (fat_cmdres_keyword_matching("OK"))
            {
                ucErrorTimes = 0;
                state = MD_AT_E0;
            }
            else
            {
                if (ucErrorTimes++ > 10)
                {
                    state = MD_ERR;
                }
            }
        }
        break;
		case MD_AT_E0://ATE0�رջ���
        if (fat_send_wait_cmdres_blocking("ATE0\r\n", 1000))
        {
            if (fat_cmdres_keyword_matching("OK"))
            {
                ucErrorTimes = 0;
                state = MD_WORK_STA_CHK;
            }
            else
            {
                if (ucErrorTimes++ > 10)
                {
                    state = MD_ERR;
                }
            }
        }
        break;
			//ģ��״̬���
			case MD_WORK_STA_CHK:
					ret = module_is_ready();
					if (ret == MD_OK)
					{
							state = MD_CONNETINIT;
					}
					else if (ret == MD_ERR)
					{
							state = MD_FLIGHTMODE;
					}
					break;
			//���Ӳ�����ʼ��
			case MD_CONNETINIT:
					ret = module_connet_parm_init();
					if (ret == MD_OK)
					{
							state = MD_CONNETED;
					}
					else if (ret == MD_ERR)
					{
							state = MD_FLIGHTMODE;
					}
					break;
			//����ͨ�Ŵ���
			case MD_CONNETED:

						if (module_data() == MD_ERR)
							{	
								state = MD_FLIGHTMODE;
							}
					break;
			//����ģʽ����
			case MD_FLIGHTMODE:
					ret = module_flightmode();
					if(ret == MD_WORK_STA_CHK)
					{
							state = MD_WORK_STA_CHK;
					}
					else if(ret == MD_ERR)
					{
							ucFlightModeTimes = 0;
							state = MD_ERR;
					}
					break;
			//����
			case MD_ERR:
					ucErrorTimes = 0;
					state = MD_RESET;
					break;
			default:
					break;
    }
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description://���յ��ƶ������ִ������͸��
 * @param None
 * @return None
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void handle_MQTTConvert(void)
{
      unsigned int  dat,dat1;
	    if(strcmp(subs_keyword,"PowerSwitch") ==0)   					//��Դ����
			{
					if(subs_dat[0] == '1') //��
					{
						 Gaspowercontrol(GAS_SET_ONOFF,1,Gaspower_index);
						 
					}
					if(subs_dat[0]  == '0')
					{
						 Gaspowercontrol(GAS_SET_ONOFF,0,Gaspower_index);
						 
				  }
			}
			
			if(strcmp(subs_keyword,"Summer_winterMode_Set") ==0)   	//�ļ�����ģʽ�趨
			{
				  if(subs_dat[0] == '1') //1 �ļ�ģʽ
					{
						 Gaspowercontrol(GAS_SET_MODE,1,Gaspower_index);
					}
					if(subs_dat[0]  == '2') //2 ����ģʽ
					{
						 Gaspowercontrol(GAS_SET_MODE,2,Gaspower_index);
					}
			}
			if(strcmp(subs_keyword,"BathTemp_Set") ==0)   					//��ԡ�¶��趨
			{
					dat = (subs_dat[0]-0x30)*10 + subs_dat[1]-0x30;
				  if((dat>= 30)&&(dat<= 75))
					{
							Gaspowercontrol(GAS_SET_Bathroom_TEMP,dat,Gaspower_index);
					}
			}
			if(strcmp(subs_keyword,"HeatingTemperature_Set") ==0)   //��ů�¶��趨
			{
					dat = (subs_dat[0]-0x30)*10 + subs_dat[1]-0x30;
				  if((dat>= 30)&&(dat<= 80))
					{
							Gaspowercontrol(GAS_SET_HEATINGTEMP,dat,Gaspower_index);
					}
			}
			if(strcmp(subs_keyword,"DeviceReset") ==0)      	 			//Զ�̸�λ
			{
					if(subs_dat[0] == '1') //��
					{
						 Gaspowercontrol(GAS_SET_RST,1,Gaspower_index);
					}
			}
			if(strcmp(subs_keyword,"Hot_water_Circ_ON") ==0)   		//��ˮѭ�������趨
			{
			    if(subs_dat[0] == '1') //��
					{
						 Gaspowercontrol(GAS_SET_HOT_W_R,1,Gaspower_index);
					}
					if(subs_dat[0]  == '0')
					{
						 Gaspowercontrol(GAS_SET_HOT_W_R,0,Gaspower_index);
				  }
			}
			if(strcmp(subs_keyword,"LockSwitch") ==0)     				//����
			{
					if(subs_dat[0] == '1') //��
					{
						 Gaspowercontrol(GAS_SET_LOCK,1,Gaspower_index);
					}
					if(subs_dat[0]  == '0')
					{
						 Gaspowercontrol(GAS_SET_LOCK,0,Gaspower_index);
				  }
			}
			if(strcmp(subs_keyword,"ECO_Set") ==0)       					//ECO����
			{
					if(subs_dat[0] == '1') //��
					{
						 Gaspowercontrol(GAS_SET_ECO,1,Gaspower_index);
					}
					if(subs_dat[0]  == '0')
					{
						 Gaspowercontrol(GAS_SET_ECO,0,Gaspower_index);
				  }
			}
			if(strcmp(subs_keyword,"Realtime_Set") ==0)   				//ʱ��У׼
			{
			    dat = subs_dat[3]-0x30;
				  dat *=1000;
				  dat1 = subs_dat[4]-0x30;
				  dat1 *=100;
				  dat += dat1;
				  
				  dat1 = subs_dat[0]-0x30;
				  dat1 *=10;
				  dat += dat1;
				  dat += subs_dat[1]-0x30;
				
				  if(dat<5)
					{
						CostofGas = 2;  //����ȼ������
					  CostofWater = 2;
						CostofElect = 2;
					}
						
				  Gaspowercontrol(GAS_SET_TIME_CALIB,dat,Gaspower_index);
			}
}

