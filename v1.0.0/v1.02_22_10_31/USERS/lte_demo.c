/********************************************************************************
* @file    	  lte.c
* @function   LTE模块 AT命令配置
* @brief 	    用3.5.0版本库建的工程模板
* @attention	
* @version 	  V1.1
********************************************************************************/
#include "global.h"

#define LTE_POWER_ON_BEFOR_WAIT_TIME 2000 										//LTE开机等待时间
#define LTE_POWER_ON_WAIT_TIME 3000 										//LTE开机等待时间
#define SIGNALMIN 15                                    //信号质量最低阀值
#define SIGNALMAX 31                                    //信号质量最低阀值
#define SOCKET_BUF_SIZE 128                             //Socket数据缓存大小
char cSocketRecvBuf[SOCKET_BUF_SIZE] = {0};             //socket数据接收缓存
char cSocketSendBuf[SOCKET_BUF_SIZE] = {0};             //socket数据发送缓存
uint8_t ucStateNum = 0;                                 //命令执行顺序标识值
uint8_t retrytimes = 0;                                 //命令重试次数
uint16_t ucErrorTimes = 0;                               //错误次数累计值
uint8_t ucFlightModeTimes = 0;													//进入飞行模式次数

uint8_t ero_time; //错误次数大于2再跳转
char subs_keyword[50]; //接收到的命名
char subs_dat[50];     //接收到的字符串数据


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description:复位LTE模块
 * @param None
 * @return None
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int module_reset(void)
{
    switch (ucStateNum)
    {
    //拉低PEN引脚
    case 0://+= PEN_GPIO_SET_LOW;
		    HAL_GPIO_WritePin(GPIOB, EC600_PWK_Pin, GPIO_PIN_SET);
        ucStateNum++;
        break;
    case 1://拉高PEN引脚
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
 * @param str：   要检索的字符串
 * @param minval：要匹配信号质量区间最小值
 * @param minval：要匹配信号质量区间最大值
 * @return 0:信号质量不满足正常工作状态, 1:信号质量满足正常工作状态
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
    /* 判断信号质量是否在设置的区间内 */
    if (lpCsqVal >= minval && lpCsqVal <= maxval)
    {
        return 1;
    }
    return 0;
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description: 检测模块工作状态是否就绪
 * @param None
 * @return 0：检测未完成；MD_OK：模块已就绪；MD_ERR：错误，不满足工作状态
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int module_is_ready(void)
{
    switch (ucStateNum)
    {
    case 0x00://关闭AT命令回显
        if (fat_send_wait_cmdres_blocking("ATE0\r\n", 1000))
        {
						//收到OK
            if (fat_cmdres_keyword_matching("OK"))
            {
                ucErrorTimes = 0;
                ucStateNum++;
            }
            else
            {
								//发送10次得不到正确应答
                if (ucErrorTimes++ > 10)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    case 0x01://读卡
        if (fat_send_wait_cmdres_blocking("AT+CPIN?\r\n", 1000))
        {
						//收到+CPIN: READY
            if (fat_cmdres_keyword_matching("+CPIN: READY"))
            {
                ucErrorTimes = 0;
                ucStateNum++;
            }
            else
            {     
								//发送10次得不到正确应答
                if (ucErrorTimes++ > 10)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    case 0x02: //查询信号质量
        if (fat_send_wait_cmdres_blocking("AT+CSQ\r\n", 2000))
        {
						//收到OK
            if (fat_cmdres_keyword_matching("OK"))
            {
							//收到的是99（射频信号未初始化）
							if (fat_cmdres_keyword_matching("+CSQ: 99,99"))
								{
										//发送30次得不到正确应答
										if (ucErrorTimes++ > 30)
										{
												ucStateNum = MD_ERR;
										}
								}
              else
                {    
										//信号值在SIGNALMIN~SIGNALMAX这个区间
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
						//没收到应答
            else
            {
								//发送30次不应答
                if (ucErrorTimes++ > 30)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    case 0x03://查看当前GPRS附着状态
        if (fat_send_wait_cmdres_blocking("AT+CGATT?\r\n", 1000))
        {
						//收到+CGATT: 1
            if (fat_cmdres_keyword_matching("+CGATT: 1"))
            {
                ucErrorTimes = 0;
								ucStateNum = MD_OK;
            }
            else
            {   
								//发送30次得不到正确应答
                if (ucErrorTimes++ > 30)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
		case MD_ERR:  //错误跳至飞行模式
				ucStateNum = 0;
				return MD_ERR;
    //完成
    case MD_OK:
				ucStateNum = 0;
        return MD_OK;
    default:
        break;
    }
    return 0;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//===========================================复位数组===================================
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
 * @description: 连接和相关配置初始化
 * @param None
 * @return 0：检测未完成；MD_OK：模块已就绪；MD_ERR：错误，不满足工作状态
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int module_connet_parm_init(void)
{
    char bufer[255];
	  switch (ucStateNum)
    {
		//配置接收模式:从服务器接收的 MQTT 消息不以 URC 的形式上报
    case 0x00:
				//如果是阿里云服务器，执行此命令，否则跳过
				if(strcmp(Device_Flash.mqttHostUrl,"a15dLSmBATk.iot-as-mqtt.cn-shanghai.aliyuncs.com") ==0)
				{
						if (fat_send_wait_cmdres_blocking("AT+QMTCFG=\"ssl\",0,1,0\r\n", 1000))
						{
								//收到OK
								if (fat_cmdres_keyword_matching("OK"))
								{
										ucErrorTimes = 0;
										ucStateNum++;
								}
								else
								{
										//发送10次得不到正确应答
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
    //打开MQTT客户端网络
    case 0x01:
			  //拼接命令
			  clear_bufer(bufer,256);
		    strcat(bufer,"AT+QMTOPEN=0,\"");
		    strcat(bufer,Device_Flash.mqttHostUrl);
		    strcat(bufer,"\",");
		    strcat(bufer,Device_Flash.port);
		    strcat(bufer,"\r\n");
		    if (fat_send_wait_cmdres_blocking(bufer, 500))
        {
						//收到+QMTOPEN: 0,0
            if (fat_cmdres_keyword_matching("+QMTOPEN: 0,0"))
            {
                ucErrorTimes = 0;
                ucStateNum++;
            }
            else
            {
								//发送10次得不到正确应答
                if (ucErrorTimes++ > 10)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    case 0x02:  //将MQTT客户端连接服务器
			  //拼接命令
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
						//收到+QMTCONN: 0,0,0
            if (fat_cmdres_keyword_matching("+QMTCONN: 0,0,0"))
            {
                ucErrorTimes = 0;
                ucStateNum++;
							  ucStateNum++; //跳过订阅case 0x04:
            }
            else
            {
								//发送10次得不到正确应答
                if (ucErrorTimes++ > 10)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    case 0x03://订阅主题1
		    //拼接命令
			  clear_bufer(bufer,256);
		    strcat(bufer,"AT+QMTSUB=0,1,\"");
		    strcat(bufer,Device_Flash.QMTSUB1); 
		    strcat(bufer,"\",0\r\n");
		    if (fat_send_wait_cmdres_blocking(bufer, 500))
				{
					//收到+QMTSUB: 0,1,0,1
          if (fat_cmdres_keyword_matching("+QMTSUB: 0,1,0,"))
            {
                ucErrorTimes = 0;
                ucStateNum ++;
            }
            else
            {
						 //发送5次得不到正确应答
                if (ucErrorTimes++ > 5)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
		case 0x04:  //订阅主题2
		    //拼接命令
			  clear_bufer(bufer,256);
		    strcat(bufer,"AT+QMTSUB=0,1,\"");
		    strcat(bufer,Device_Flash.QMTSUB2); 
		    strcat(bufer,"\",0\r\n");
		    if (fat_send_wait_cmdres_blocking(bufer, 500))
				{
					//收到+QMTSUB: 0,1,0,1
          if (fat_cmdres_keyword_matching("+QMTSUB: 0,1,0,"))
            {
                ucErrorTimes = 0;
                ucStateNum = MD_OK;
            }
            else
            {
						 //发送5次得不到正确应答
                if (ucErrorTimes++ > 5)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    case MD_OK://完成
				ucStateNum = 0;
        return MD_OK;
    case MD_ERR://错误跳至飞行模式
				ucStateNum = 0;
        return MD_ERR;
    default:
        break;
    }
    return 0;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description: 截取socket接收缓存区的Topic
 * @param cmdres：命令响应数据
 * @param recvbuf：socket接收缓存
 * @return 接收Topic的长度
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
	
	  memset(subs_keyword,0,50); //清零内存
		memset(subs_dat,0,50); //清零内存
	
		//定位至应答的OK
    p = strstr(cmdres, "}}");
	
    if (p == NULL)
    {
        return 0;
    }
		//截取数据
		//p = p-1; //带引号的需要增加该行
		q = p;
		do
		{
        p--;
    }
    while (*p != 0x3A);//查询：地址
		
    length = q - p - 1;	
		
		if(length>50) length = 50; //限制大小
    memcpy(subs_dat, p + 1, length);
		printf("data:%s=",subs_dat);  //{"params":{"Summer_winterMode_Set":2}}
		
		//截取命令
		p -= 1;
		q = p;
		do
		{
        p--;
    }
    while (*p != 0x22);
    length = q - p - 1;	
		
		if(length>50) length = 50; //限制大小
    memcpy(subs_keyword, p + 1, length);
		printf("%s\r",subs_keyword);
    return 1;
}

/**
 * @brief 上报属性PowerSwitch到云端
 * @param dm_handle，dm句柄, 数据类型void *
 * @return 消息id:(>=1), 上报失败: <0
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description: 数据收发部分
 * @return 0：检测未完成；MD_ERR：错误
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int module_data(void)
{
    switch (ucStateNum)
    {
    //定长主题发布数据
			
			case SUB_MESG_GET:
				   ucStateNum++;
			break;
				
			case SUB_SEND1:
				//拼接命令
			  //printf("待发送");
				if(topic_state)
				{
					if(fat_send_wait_cmdres_blocking(topic_buf1,100))			
					{
							ucStateNum++;
					}
				}else
				{
				   ucStateNum = SUB_CK_SOCKET; //没有待发送命令，跳转到命令接收
					 //printf("NO\r");
				}
        break;
    case SUB_SEND2:

				if(fat_send_wait_cmdres_nonblocking(topic_buf2,6000))
				{
						ucStateNum++;
					  topic_state_last = topic_state;//记录当前发送直针
				    topic_state = 0;
					  topic_buf_ok = 0; //发送完成标识
					  topic_count++;
        }
        break;
		
		case SUB_CHECK:
				//收到OK
				if (fat_cmdres_keyword_matching("OK")||(fat_cmdres_keyword_matching("OK")))
				{
						printf("发送<OK>\r");
						ucErrorTimes = 0;
						ucStateNum = SUB_WAIT; //优先发送
					  ero_time = 0;
					  
					 Gaspowercontrol(GAS_SET_RSSI,3,Gaspower_index);
				}
				//发送3次得不到正确应答
				if (ucErrorTimes++ > 3)
						{
							ero_time++;
  						if(ero_time>2)	 
							ucStateNum = MD_ERR;
						}
        break;
		
    //查询socket缓存是否有数据
    case SUB_CK_SOCKET:
				if (fat_send_wait_cmdres_blocking("AT+QMTRECV?\r",1500))
        {
						//收到+QMTRECV:
            //if (fat_cmdres_keyword_matching("+QMTRECV:"))
					  if(fat_cmdres_keyword_matching("post_reply"))
            { 
								//查看数据
	              printf("查收<OK>");
							  if(match_recv_data(USART1_RX_BUF))
								{
                    handle_MQTTConvert();
								}
            }
            else
            {
								ucErrorTimes = 0;
							  ucStateNum = SUB_SEND1; //没有接收到命令，查看是否需要发送
            }
        }
        break;
		//截取Topic和Data
    case SUB_CK_TOPIC:
//		    if (fat_send_wait_cmdres_blocking("AT+QMTRECV=0\r\n", 1000))
//				{
//						if (fat_cmdres_keyword_matching("+QMTRECV:"))
//						{
////								//截取主题名
////								ret = match_recv_topic(USART1_RX_BUF, cSocketRecvBuf);
////								printf("socket recv topic:%d , %s\r",ret , cSocketRecvBuf);
////								//截取数据内容
////								ret = match_recv_data(USART1_RX_BUF, cSocketRecvBuf);

////								printf("socket recv data:%d , %s\r",ret , cSocketRecvBuf);
//							  
//								//截取主题名
//								ret = match_recv_topic(USART1_RX_BUF, cSocketRecvBuf);
//								printf("socket recv topic:%d , %s\r",ret , cSocketRecvBuf);
//								
//								ucErrorTimes = 0;
//								ucStateNum = 0x00;
//						}
//						else
//						{
//								//发送3次得不到正确应答
//                if (ucErrorTimes++ > 3)
//                {
//                    ucStateNum = SUB_WAIT;
//                }
//						}
//				}
//				break;
		case SUB_WAIT:
				//收到OK
		    if(wait_timeout(50))
				{
						//printf("WT");
					  ucStateNum = SUB_SEND1;
					//ucStateNum = SUB_CK_SOCKET;
				}
        break;
		//错误跳至飞行模式
    case SUB_ERR:
				ucStateNum = 0;
		    if(debug_onoff.EC600N)  printf("飞行&&\r");
		    
        return MD_ERR;
    default:
        break;
    }
    return 0;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description: 飞行模式处理函数
 * @param None
 * @return 0：检测未完成；MD_WORK_STA_CHK：重新开启网络跳至模块状态检测；MD_ERR：错误
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int module_flightmode()
{
		switch (ucStateNum)
    {
			case 0x00:
				ucFlightModeTimes++;
				ucStateNum++;
				if(debug_onoff.EC600N)  printf("FlightTime：%d\r\n",ucFlightModeTimes);
				break;
			case 0x01:
				if (ucFlightModeTimes == 2)
				{
						if(debug_onoff.EC600N)  printf("二次飞行模式，复位模块\r\n");
						ucStateNum = MD_ERR;
				}
				else
					ucStateNum++;
				break;
			case 0x02:
        if (fat_send_wait_cmdres_blocking("AT+CFUN=0\r\n", 2000))
        {
						//收到OK
            if (fat_cmdres_keyword_matching("OK"))
            {
								ucErrorTimes = 0;
                ucStateNum++;
            }
						else
            {
								//发送5次得不到正确应答，跳至MD_ERR
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
						//收到OK,状态更为MD_WORK_STA_CHK，跳至模块状态检测
            if (fat_cmdres_keyword_matching("OK"))
            {
								if(debug_onoff.EC600N)   printf("再开启功能\r\n");
								ucStateNum = 0;
								return MD_WORK_STA_CHK;
            }
						else
            {
								//发送5次得不到正确应答，跳至MD_ERR
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
 * @description: LTE模块MQTT协议
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
			
			case MD_WAIT://模块上电等待
				if(wait_timeout(LTE_POWER_ON_BEFOR_WAIT_TIME)) //等待2秒
				{
						state = MD_RESET;
				}
				break;
			case MD_RESET://复位模块，等待5秒
        if (module_reset())
        {
						wait_timeout(LTE_POWER_ON_WAIT_TIME); //复位等待//5s
					  //if(debug_onoff.EC600N)   
							printf("Reset-%d-!!!!!!!!!!!!!!!!!!!!!!\r\n",outlinetimes);
						outlinetimes++;
					  state = MD_AT_REQ;
					  Gaspowercontrol(GAS_SET_RSSI,1,Gaspower_index);
        }
        break;
    case MD_AT_REQ://AT握手
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
		case MD_AT_E0://ATE0关闭回显
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
			//模块状态检测
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
			//连接参数初始化
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
			//数据通信处理
			case MD_CONNETED:

						if (module_data() == MD_ERR)
							{	
								state = MD_FLIGHTMODE;
							}
					break;
			//飞行模式处理
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
			//错误
			case MD_ERR:
					ucErrorTimes = 0;
					state = MD_RESET;
					break;
			default:
					break;
    }
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description://接收到云端命令后执行命令透传
 * @param None
 * @return None
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void handle_MQTTConvert(void)
{
      unsigned int  dat,dat1;
	    if(strcmp(subs_keyword,"PowerSwitch") ==0)   					//电源开关
			{
					if(subs_dat[0] == '1') //开
					{
						 Gaspowercontrol(GAS_SET_ONOFF,1,Gaspower_index);
						 
					}
					if(subs_dat[0]  == '0')
					{
						 Gaspowercontrol(GAS_SET_ONOFF,0,Gaspower_index);
						 
				  }
			}
			
			if(strcmp(subs_keyword,"Summer_winterMode_Set") ==0)   	//夏季冬季模式设定
			{
				  if(subs_dat[0] == '1') //1 夏季模式
					{
						 Gaspowercontrol(GAS_SET_MODE,1,Gaspower_index);
					}
					if(subs_dat[0]  == '2') //2 冬季模式
					{
						 Gaspowercontrol(GAS_SET_MODE,2,Gaspower_index);
					}
			}
			if(strcmp(subs_keyword,"BathTemp_Set") ==0)   					//卫浴温度设定
			{
					dat = (subs_dat[0]-0x30)*10 + subs_dat[1]-0x30;
				  if((dat>= 30)&&(dat<= 75))
					{
							Gaspowercontrol(GAS_SET_Bathroom_TEMP,dat,Gaspower_index);
					}
			}
			if(strcmp(subs_keyword,"HeatingTemperature_Set") ==0)   //采暖温度设定
			{
					dat = (subs_dat[0]-0x30)*10 + subs_dat[1]-0x30;
				  if((dat>= 30)&&(dat<= 80))
					{
							Gaspowercontrol(GAS_SET_HEATINGTEMP,dat,Gaspower_index);
					}
			}
			if(strcmp(subs_keyword,"DeviceReset") ==0)      	 			//远程复位
			{
					if(subs_dat[0] == '1') //开
					{
						 Gaspowercontrol(GAS_SET_RST,1,Gaspower_index);
					}
			}
			if(strcmp(subs_keyword,"Hot_water_Circ_ON") ==0)   		//热水循环开关设定
			{
			    if(subs_dat[0] == '1') //开
					{
						 Gaspowercontrol(GAS_SET_HOT_W_R,1,Gaspower_index);
					}
					if(subs_dat[0]  == '0')
					{
						 Gaspowercontrol(GAS_SET_HOT_W_R,0,Gaspower_index);
				  }
			}
			if(strcmp(subs_keyword,"LockSwitch") ==0)     				//锁定
			{
					if(subs_dat[0] == '1') //开
					{
						 Gaspowercontrol(GAS_SET_LOCK,1,Gaspower_index);
					}
					if(subs_dat[0]  == '0')
					{
						 Gaspowercontrol(GAS_SET_LOCK,0,Gaspower_index);
				  }
			}
			if(strcmp(subs_keyword,"ECO_Set") ==0)       					//ECO设置
			{
					if(subs_dat[0] == '1') //开
					{
						 Gaspowercontrol(GAS_SET_ECO,1,Gaspower_index);
					}
					if(subs_dat[0]  == '0')
					{
						 Gaspowercontrol(GAS_SET_ECO,0,Gaspower_index);
				  }
			}
			if(strcmp(subs_keyword,"Realtime_Set") ==0)   				//时间校准
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
						CostofGas = 2;  //清零燃气积算
					  CostofWater = 2;
						CostofElect = 2;
					}
						
				  Gaspowercontrol(GAS_SET_TIME_CALIB,dat,Gaspower_index);
			}
}

