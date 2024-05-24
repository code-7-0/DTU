#ifndef __LTE_H
#define __LTE_H

typedef enum
{
    MD_WAIT,        				//复位模块
	  MD_RESET,        				//复位模块
    MD_AT_REQ,       				//AT握手
	  MD_AT_E0,       				//关闭回显
    MD_WORK_STA_CHK, 				//工作状态检测
    MD_CONNETINIT,   				//连接配置信息初始化
    MD_CONNETED,     				//数据通信
		MD_FLIGHTMODE,			   	//飞行模式
    MD_OK = 0xFE,    				//正常
    MD_ERR = 0xFF,   				//异常
} MD_RUN_STATE;

//发布流程标识
typedef enum
{
    SUB_MESG_GET = 0,
	  SUB_SEND1,        				//发送命令
	  SUB_SEND2,        				   //发送命令
	  SUB_CHECK,        				  //查询返回状态
    SUB_CK_SOCKET,       				//查询Socket
    SUB_CK_TOPIC, 					    //工作状态检测
	  SUB_WAIT,   								//连接配置信息初始化
    SUB_CONNETED,     					//数据通信
		SUB_FLIGHTMODE,			   			//飞行模式
    SUB_OK = 0xFE,    					//正常
    SUB_ERR = 0xFF,   					//异常
} SUB_STATE;


void handle_MQTTConvert(void);//接收到云端命令后执行命令透传
void clear_bufer(char *arry,unsigned int dat);
void module_MQTT(void);

#endif
