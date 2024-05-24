#include "global.h"

struct Device_Manage Device_Manage; //设备信息
struct debug_onoff debug_onoff = {0,1,1,1};     //调试信息开关

//USART3中断接收处理
unsigned int   usart3delay;           //通信超时时间，定义大于5ms,为通信超时
unsigned char  FLAG_uart3;			      //UART2开始新的一帧数据接收前先使能接收允许标志，接受过程中保持为1，在定时器发生接收超时时被清零
unsigned char  Debug_Shandle = 0;	//允许帧处理标志

extern UART_HandleTypeDef huart3;

unsigned char USART3RXD_loop = 0; 			//USART2 接收存储索引
 char USART3_RX_BUF[200];
 char USART3_TX_BUF[256];

#ifdef __GNUC__  
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)  
#else  
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE* f)  
#endif /* __GNUC__ */  
   
#ifdef __cplusplus  
extern "C" {  
#endif //__cplusplus  
   
	PUTCHAR_PROTOTYPE  
	{  
			HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xFFFF);  //调用STM32的HAL库，发送一个字节
		return (ch);  
	}  
		 
	#ifdef __cplusplus  
	}  
#endif //__cplusplus  


/**
  * @brief This function handles USART3 and USART4 interrupts.
  */
void USART3_4_IRQHandler(void)
{
  unsigned char datatemp;
	/* USER CODE BEGIN USART3_4_IRQn 0 */
	
  /* USER CODE END USART3_4_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_4_IRQn 1 */
	
	//HAL_UART_Receive_IT(&huart2,USART2_RX_BUF, 1);
	if ( (__HAL_UART_GET_FLAG (&huart3, UART_FLAG_RXNE) != RESET) )//接收数据
	 {
				datatemp = huart3.Instance->RDR;//注意这个是RDR，以前用的大部分都是DR;
			if(!FLAG_uart3)	//开始新的一帧诗句接收前先使能接收允许标志，在定时器发生接收超时时被清零
				 {
					 FLAG_uart3 = 1;//帧接收标志
					 USART3RXD_loop = 0;	//复位索引准备接收新的一帧
				 }
				 if(FLAG_uart3)
				 {
						if(USART3RXD_loop < 200)  //防止溢出
						{
							USART3_RX_BUF[USART3RXD_loop]  = datatemp;//接收到的数据存入缓冲器
							USART3RXD_loop++;
						}
					usart3delay = delaycount;//更新定时数据
				 }
	 }	
  /* USER CODE END USART3_4_IRQn 1 */
}


void Handle_Debug(void)
{ 
     if(Device_Manage.run_start ==0)
		 {
			 printf("%s\r\n", Device_Flash.hello);
			 printf("*****硬件版本V1.0.0*****\r\n");  // \n表示换行
			 printf("*****软件版本V1.0.0*****\r\n");  // \n表示换行
			 printf("ClientId:    %s\r\n", Device_Flash.clientId);
			 printf("MqttHostUrl: %s\r\n", Device_Flash.mqttHostUrl);
			 printf("Port:        %s\r\n", Device_Flash.port);
			 printf("Passwd:      %s\r\n", Device_Flash.passwd);
			 printf("Username:    %s\r\n", Device_Flash.username);
			 printf("QMTSUB1:     %s\r\n", Device_Flash.QMTSUB1);
			 printf("QMTSUB2:     %s\r\n", Device_Flash.QMTSUB2);
			 printf("QMTSUB3:     %s\r\n", Device_Flash.QMTSUB3);
		 }
		 
		 if(Debug_Shandle)
		 {
		   Debug_Shandle = 0; 
			 if(strcmp(USART3_RX_BUF,"gasp_closed") ==0) //手动关闭燃气壁挂炉
			 {
					Gaspowercontrol(GAS_SET_ONOFF,0,Gaspower_index);
			 }
			 
			 if(strcmp(USART3_RX_BUF,"gasp_open") ==0) //手动开启燃气壁挂炉
			 {
					Gaspowercontrol(GAS_SET_ONOFF,1,Gaspower_index);
			 }
			 
			  if(strcmp(USART3_RX_BUF,"debug_Gaspower") ==0) //燃气壁挂炉调试信息开关
			 {
					if(debug_onoff.Gaspower) 
					{
					  debug_onoff.Gaspower = 0;
						printf("壁挂炉信息打印-OFF");
					}
				  else 
					{
					  debug_onoff.Gaspower = 1;
						printf("壁挂炉信息打印-ON");
					}
			 }
			 
			  if(strcmp(USART3_RX_BUF,"Realtime_Set") ==0) //时间校准
			 {
					 Gaspowercontrol(GAS_SET_TIME_CALIB,1505,Gaspower_index);
			 }
			 
			 
			 if(strcmp(USART3_RX_BUF,"system_RST=1") ==0) //手动关闭燃气壁挂炉
			 {
					//__set_FAULTMASK(1);//关闭所有中断
					NVIC_SystemReset();//复位函数
			 }
			 
			 if(strcmp(USART3_RX_BUF,"debug_EC600N") ==0) //燃气壁挂炉调试信息开关
			 {
					if(debug_onoff.EC600N) 
						{debug_onoff.EC600N = 0;
						 printf("EC600N信息打印-OFF");
						}
				  else 
					{
						debug_onoff.EC600N = 1;
						printf("EC600N信息打印-ON");
					}
			 }
			 
			 if(strcmp(USART3_RX_BUF,"GasCost") ==0) //燃气积算打印开关
			 {
					if(debug_onoff.GasCost) 
					{
						debug_onoff.GasCost = 0;
						printf("燃气积算信息打印-OFF");
					}
				  else 
					{
						debug_onoff.GasCost = 1;
						printf("燃气积算信息打印-ON");
					}
			 }
			 
			 
			  if(strcmp(USART3_RX_BUF,"MQ_send") ==0) //燃气积算打印开关
			 {
					if(debug_onoff.MQ_send) debug_onoff.MQ_send = 0;
				  else debug_onoff.MQ_send = 1;
			 }
			 
			 if(strcmp(USART3_RX_BUF,"TEST_ER") ==0) //调试故障命令发送
			 {
				  if(GASwall_Manage[0].ERRO_CODE) GASwall_Manage[0].ERRO_CODE = 0;
				  else GASwall_Manage[0].ERRO_CODE = ERROR_FIRE;
			 }
			 
			 
			 if(strcmp(USART3_RX_BUF,"flash_save") ==0) //手动存储数据
			 {	
				 Flag_wr_flash_u8 = 1;
			 }
			 
			 	if(strcmp(USART3_RX_BUF,"flash_Factory") ==0) //恢复出厂参数
			 {	
				 flash_Factory();
			 }
			 
			 
			 if(strncmp(USART3_RX_BUF,"MQTT_username=",strlen("MQTT_username=")) ==0) //设置MQTT_username
			 {
					memset(Device_Flash.username,0,sizeof(Device_Flash.username)); //清空
				  strcat(Device_Flash.username,(USART3_RX_BUF+strlen("MQTT_username=")));
				  printf("MQTT_setting_username:    %s\r\n", Device_Flash.username);
			 }
			 
			 if(strncmp(USART3_RX_BUF,"MQTT_clientId=",strlen("MQTT_clientId=")) ==0) //MQTT_clientId
			 {
					memset(Device_Flash.clientId,0,sizeof(Device_Flash.clientId)); //清空
				  strcat(Device_Flash.clientId,(USART3_RX_BUF+strlen("MQTT_clientId=")));
				  printf("MQTT_setting_clientId:    %s\r\n", Device_Flash.clientId);
			 }
			 
			 if(strncmp(USART3_RX_BUF,"MQTT_mqttHostUrl=",strlen("MQTT_mqttHostUrl=")) ==0) //MQTT_mqttHostUrl
			 {
					memset(Device_Flash.mqttHostUrl,0,sizeof(Device_Flash.mqttHostUrl)); //清空
				  strcat(Device_Flash.mqttHostUrl,(USART3_RX_BUF+strlen("MQTT_mqttHostUrl=")));
				  printf("MQTT_setting_mqttHostUrl:    %s\r\n", Device_Flash.mqttHostUrl);
			 }
			 
			 if(strncmp(USART3_RX_BUF,"MQTT_passwd=",strlen("MQTT_passwd=")) ==0) //MQTT_passwd
			 {
					memset(Device_Flash.passwd,0,sizeof(Device_Flash.passwd)); //清空
				  strcat(Device_Flash.passwd,(USART3_RX_BUF+strlen("MQTT_passwd=")));
				  printf("MQTT_setting_passwd:    %s\r\n", Device_Flash.passwd);
			 }
			 
			 if(strncmp(USART3_RX_BUF,"MQTT_port=",strlen("MQTT_port=")) ==0) //MQTT_passwd
			 {
					memset(Device_Flash.port,0,sizeof(Device_Flash.port)); //清空
				  strcat(Device_Flash.port,(USART3_RX_BUF+strlen("MQTT_port=")));
				  printf("MQTT_setting_port:    %s\r\n", Device_Flash.port);
			 }
			 
			 if(strncmp(USART3_RX_BUF,"MQTT_QMTSUB1=",strlen("MQTT_QMTSUB1=")) ==0) //MQTT_QMTSUB1=
			 {
					memset(Device_Flash.QMTSUB1,0,sizeof(Device_Flash.QMTSUB1)); //清空
				  strcat(Device_Flash.QMTSUB1,(USART3_RX_BUF+strlen("MQTT_QMTSUB1=")));
				  printf("MQTT_setting_QMTSUB1:    %s\r\n", Device_Flash.QMTSUB1);
			 }
			 
			 if(strncmp(USART3_RX_BUF,"MQTT_QMTSUB2=",strlen("MQTT_QMTSUB2=")) ==0) //MQTT_QMTSUB1=
			 {
					memset(Device_Flash.QMTSUB2,0,sizeof(Device_Flash.QMTSUB2)); //清空
				  strcat(Device_Flash.QMTSUB2,(USART3_RX_BUF+strlen("MQTT_QMTSUB2=")));
				  printf("MQTT_setting_QMTSUB2:    %s\r\n", Device_Flash.QMTSUB2);
			 }
			 if(strncmp(USART3_RX_BUF,"MQTT_QMTSUB3=",strlen("MQTT_QMTSUB3=")) ==0) //MQTT_QMTSUB1=
			 {
					memset(Device_Flash.QMTSUB3,0,sizeof(Device_Flash.QMTSUB3)); //清空
				  strcat(Device_Flash.QMTSUB3,(USART3_RX_BUF+strlen("MQTT_QMTSUB3=")));
				  printf("MQTT_setting_QMTSUB3:    %s\r\n", Device_Flash.QMTSUB3);
			 }
			 memset(USART3_RX_BUF,0,sizeof(USART3_RX_BUF));
		 }
}

void printfArry(unsigned char *arry,unsigned char num) //打印数组
{
    unsigned char i;
	  for(i=0;i<num;i++)
		{
			printf("%d;",arry[i]);  // \n表示换行
			//printf("单总线发送等待");  // \n表示换行
		}
}
