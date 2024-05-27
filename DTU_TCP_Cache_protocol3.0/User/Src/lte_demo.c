#include "lte_demo.h"

typedef enum
{
    MD_WAIT,        				//复位模块
	  MD_RESET,        				//复位模块
    MD_AT_REQ,       				//AT握手
//	  MD_AT_E0,       				//关闭回显
    MD_WORK_STA_CHK, 				//工作状态检测
    MD_CONNETINIT_MQTT,   	//连接配置信息初始化
	  MD_CONNETINIT_TCP,
    MD_CONNETED_MQTT,     	//数据通信
		MD_CONNETED_TCP,
		MD_FLIGHTMODE,			   	//飞行模式
    MD_OK = 0xFE,    				//正常
    MD_ERR = 0xFF,   				//异常
} MD_RUN_STATE;

#define LTE_POWER_ON_BEFOR_WAIT_TIME 2000 										//LTE开机等待时间
#define LTE_POWER_ON_WAIT_TIME 3000 										//LTE开机等待时间
#define SIGNALMIN 15                                    //信号质量最低阀值
#define SIGNALMAX 31                                    //信号质量最低阀值 

uint8_t ucStateNum = 0;                                 //命令执行顺序标识值
uint8_t state = MD_RESET;
uint16_t ucErrorTimes = 0;                               //错误次数累计值
uint8_t ucFlightModeTimes = 0;													//进入飞行模式次数

 
//char subs_keyword[50]; //接收到的命名
//char subs_dat[50];     //接收到的字符串数据


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description:复位LTE模块
 * @param None
 * @return None
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//uint8_t module_reset(void)
//{
//    switch (ucStateNum)
//    {
//    //拉低RESET_N引脚
//    case 0://+= PEN_GPIO_SET_LOW;
//				HAL_GPIO_WritePin(GPIOB, EC600_RST_Pin, GPIO_PIN_SET);
////		    HAL_GPIO_WritePin(GPIOB, EC600_PWK_Pin, GPIO_PIN_SET);
//        ucStateNum++;
//        break;
//    case 1://拉高RESET_N引脚
//        if (wait_timeout(1000))
//        {
//					  HAL_GPIO_WritePin(GPIOB, EC600_RST_Pin, GPIO_PIN_RESET);
////					  HAL_GPIO_WritePin(GPIOB, EC600_PWK_Pin, GPIO_PIN_RESET);
//            ucStateNum = 0;
//            return 1;
//        }
//        break;
//    default:
//        break;
//    }
//    return 0;
//}
//uint8_t module_power_on(void)
//{
//    switch (ucStateNum)
//    { 
//			//30ms等待VBAT电压稳定
//    case 0://+= PEN_GPIO_SET_LOW;
//		if(wait_timeout(50))
//			ucStateNum++;
//      break;			
//    //拉低PWRKEY引脚
//    case 1://+= PEN_GPIO_SET_LOW;
//				HAL_GPIO_WritePin(GPIOB, EC600_PWK_Pin, GPIO_PIN_SET);
//        ucStateNum++;
//        break;
//    case 2://拉高PWRKEY引脚
//        if (wait_timeout(1000))
//        {
//						HAL_GPIO_WritePin(GPIOB, EC600_PWK_Pin, GPIO_PIN_RESET);
//            ucStateNum = 0;
//            return 1;
//        }
//        break;
//    default:
//        break;
//    }
//    return 0;
//}
uint8_t module_reset(void)
{
    switch (ucStateNum)
    {
		//30ms等待VBAT电压稳定
    case 0://+= PEN_GPIO_SET_LOW;
		if(wait_timeout(50))
			ucStateNum++;
      break;		
    //拉低PWRKEY引脚
    case 1://+= PEN_GPIO_SET_LOW;
		    HAL_GPIO_WritePin(GPIOB, EC600_PWK_Pin, GPIO_PIN_SET);
        ucStateNum++;
        break;
    case 2://拉高PWRKEY引脚
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
uint8_t match_csq(char *str, int minval, int maxval)//+CSQ: 25,99
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
    q = p;//2
    while (*p != ',')
    {
        p++;
    }
    memcpy(tmp, q, p - q);
		tmp[p - q] = '\0'; // 确保字符串以 null 字符结束###
    lpCsqVal = atoi(tmp);//25
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


// 发送AT命令并检查响应的函数
uint8_t send_at_and_check(char* cmd, char* expected_response, uint16_t timeout, 
														uint8_t max_retries, uint8_t nextState,uint8_t errorState) 
{
    if (fat_send_wait_cmdres_blocking(cmd, timeout)) {
        if (fat_cmdres_keyword_matching(expected_response)) {
//						debug_printf("uart recv : %s\r\n",g_tModule.EC600_RevBuf);
            ucErrorTimes = 0;
            return nextState;
        }
				else
				{
					if (ucErrorTimes++ > max_retries) 
							return errorState;
				}					
    }
    return ucStateNum; // 保持当前状态
}

void Extract_QCCID(const char* buffer, char* qccidString) {
    // 查找“+QCCID: ”这个子串，返回第一次出现的位置
    const char* ptr = strstr(buffer, "+QCCID: ");
    if (ptr) {
        // 移动指针，越过前缀
        ptr += strlen("+QCCID: ");
        // 将QCCID的数字部分复制到qccidString中
        while (*ptr != '\r' && *ptr != '\0') {
            *qccidString++ = *ptr++;
        }
        // 字符串结尾添加NULL字符
        *qccidString = '\0';
    }
} 
typedef enum {
    STATE_DISABLE_ECHO = 0,
    STATE_CHECK_PIN,
    STATE_CHECK_SIGNAL_QUALITY,
    STATE_CHECK_GPRS_ATTACH,
		STATE_QUERY_QCCID
} ModuleState;

uint8_t module_is_ready(void) {
//	  static uint8_t first_flag=1;
    switch (ucStateNum) {
        case STATE_DISABLE_ECHO:
            ucStateNum = send_at_and_check("ATE0\r\n", "OK", 1000, 10, STATE_CHECK_PIN, MD_ERR);
            break;
        case STATE_CHECK_PIN:
            ucStateNum = send_at_and_check("AT+CPIN?\r\n", "+CPIN: READY", 1000, 10, STATE_CHECK_SIGNAL_QUALITY, MD_ERR);
            break;
        case STATE_CHECK_SIGNAL_QUALITY:
            ucStateNum = send_at_and_check("AT+CSQ\r\n", "OK", 1000, 30, STATE_CHECK_GPRS_ATTACH, MD_ERR);
            // 特殊处理信号质量
            if (ucStateNum == STATE_CHECK_GPRS_ATTACH && 
								!match_csq((char*)g_tModule.EC600_RevBuf, SIGNALMIN, SIGNALMAX)){
                ucStateNum = MD_ERR;
            }
            break;
        case STATE_CHECK_GPRS_ATTACH:
            ucStateNum = send_at_and_check("AT+CGATT?\r\n", "+CGATT: 1", 1000, 30, STATE_QUERY_QCCID, MD_ERR);
            break;
        case STATE_QUERY_QCCID:
//            ucStateNum = send_at_and_check("AT+QCCID\r\n", "+QCCID: 1", 500, 10, MD_OK, MD_ERR);
						if(qccid_get_flag==0){
								if (fat_send_wait_cmdres_blocking("AT+QCCID\r\n", 500)) {
										if (fat_cmdres_keyword_matching("OK")) { 
												Extract_QCCID((char*)g_tModule.EC600_RevBuf,QCCID);//提取QCCID的值
											  qccid_get_flag=1; 
												ucErrorTimes = 0; 
												ucStateNum=MD_OK;
										}
										else
										{
											if (ucErrorTimes++ > 10)  
												ucStateNum=MD_ERR;
										}					
								}			
						}			
						else
							ucStateNum=MD_OK;
            break;				
        case MD_OK:
            ucStateNum = 0;
            return MD_OK;
        case MD_ERR:
            ucStateNum = 0;
            return MD_ERR;
    }
    return 0; // 如果没有完成，返回0继续等待
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
// 定义状态枚举
enum ModuleState { 
		STATE_SSL_CONFIG,
		STATE_MQTT_URCCFG, 
		STATE_MQTT_OPEN, 
		STATE_MQTT_CONN, 
		STATE_SUBSCRIBE_TOPIC2,
		STATE_SUBSCRIBE_TOPIC5,
		STATE_SUBSCRIBE_TOPIC6
};

uint8_t mqtt_connect_parm_init(void) 
{
	  char bufer[255];
    switch (ucStateNum) {
				case STATE_SSL_CONFIG:
						//如果是阿里云服务器，执行此命令，否则跳过
						if(strcmp(Device_Flash.mqttHostUrl, "a15dLSmBATk.iot-as-mqtt.cn-shanghai.aliyuncs.com") == 0) { 
								ucStateNum = send_at_and_check("AT+QMTCFG=\"ssl\",0,1,0\r\n", "OK", 1000, 10, STATE_MQTT_URCCFG, MD_ERR);
						} else {
								// If not matching, move to the next state directly
								ucStateNum++;
//								ucStateNum++;
						}
						break;
        case STATE_MQTT_URCCFG:
					//  配置参数2 接收模式为  0:URC   1:缓存读取
//						ucStateNum = send_at_and_check("AT+QMTCFG=\"recv/mode\",0,1,0\r\n", "OK", 1000, 10, STATE_MQTT_OPEN, MD_ERR);
						ucStateNum = send_at_and_check("AT+QMTCFG=\"recv/mode\",0,1,0\r\n", "OK", 1000, 10, STATE_MQTT_OPEN, MD_ERR);
            break;			
				//打开MQTT客户端网络
        case STATE_MQTT_OPEN:
            // 构建MQTT OPEN命令 
						snprintf(bufer, sizeof(bufer), "AT+QMTOPEN=0,\"%s\",%s\r\n", Device_Flash.mqttHostUrl, Device_Flash.port);
						ucStateNum = send_at_and_check(bufer, "+QMTOPEN: 0,0", 1000, 10, STATE_MQTT_CONN, MD_ERR);
            break;
				//将MQTT客户端连接服务器
        case STATE_MQTT_CONN:
            // 构建MQTT CONNECT命令 
						snprintf(bufer, sizeof(bufer), "AT+QMTCONN=0,\"%s\",\"%s\",\"%s\"\r\n", 
							       Device_Flash.clientId, Device_Flash.username, Device_Flash.passwd);
						ucStateNum = send_at_and_check(bufer, "+QMTCONN: 0,0,0", 1000, 10, STATE_SUBSCRIBE_TOPIC2, MD_ERR); 
            break;
				//订阅主题		
        case STATE_SUBSCRIBE_TOPIC2: 
						snprintf(bufer, sizeof(bufer), "AT+QMTSUB=0,1,\"%s\",0\r\n", Device_Flash.QMTSUB2); 
            ucStateNum = send_at_and_check(bufer, "+QMTSUB: 0,1,0,", 1000, 5, STATE_SUBSCRIBE_TOPIC5, MD_ERR); 
            break;  	
				//订阅OTA主题		
        case STATE_SUBSCRIBE_TOPIC5: 
						snprintf(bufer, sizeof(bufer), "AT+QMTSUB=0,1,\"%s\",0\r\n", Device_Flash.QMTSUB5); 
            ucStateNum = send_at_and_check(bufer, "+QMTSUB: 0,1,0,", 1000, 5, STATE_SUBSCRIBE_TOPIC6, MD_ERR); 
            break;  
        case STATE_SUBSCRIBE_TOPIC6: 
						snprintf(bufer, sizeof(bufer), "AT+QMTSUB=0,1,\"%s\",0\r\n", Device_Flash.QMTSUB6); 
            ucStateNum = send_at_and_check(bufer, "+QMTSUB: 0,1,0,", 1000, 5, MD_OK, MD_ERR); 
            break;  				
				//完成
        case MD_OK:
            ucStateNum = 0;
						Gaspower_Command(GAS_SET_RSSI,3,Gaspower_index);
            return MD_OK;
				//跳转到飞行模式
        case MD_ERR:
            ucStateNum = 0;
            return MD_ERR;
    }
    return 0; // 继续处理
}




// 定义状态码
#define VARIABLE_DEVICE_FOUND 1
#define FIXED_DEVICE_FOUND 2
#define CALIB_TIME_FOUND 3
void* device_malloc  = NULL;
char* extract_json(const char *input) {
    const char *start = strchr(input, '{');
    const char *end = strrchr(input, '}');//字符串中最后一个'}'字符的位置

    if (!start || !end || start >= end) {
        return NULL;  
    }
 
    size_t length = end - start + 1;
    char *json = (char*)malloc(length + 1); 

    if (json) {
        strncpy(json, start, length);
        json[length] = '\0';  
    }

    return json;  
}


uint8_t parse_fixed_json(const char *json_string,Fixed_Device *fixDevice) {
	 uint8_t state=0;
    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL) {
        debug_printf("Error before: %s\n", cJSON_GetErrorPtr());
        return state;
    }

    cJSON *params = cJSON_GetObjectItemCaseSensitive(json, "params");
    if (!cJSON_IsObject(params)) { 
        cJSON_Delete(json);
        return state;
    }

//    cJSON *device_id = cJSON_GetObjectItemCaseSensitive(params, "device_id");
//    if (device_id && cJSON_IsString(device_id)) { 
//				 Variable_Device *varDevice  =  malloc(sizeof(Variable_Device)); 
//				if(varDevice==NULL)
//					return state;
//        memset(varDevice, 0, sizeof(Variable_Device));
//        strncpy( varDevice->deviceID  , device_id->valuestring, sizeof(varDevice->deviceID) - 1);
//        
//        cJSON *current_element = NULL;
//        cJSON_ArrayForEach(current_element, params) {
//            if (strcmp(current_element->string, "device_id") != 0) { // Skip device_id itself
//                if (cJSON_IsNumber(current_element)) {
//                    strncpy(varDevice->key, current_element->string, sizeof(varDevice->key) - 1);
//                    varDevice->value = current_element->valueint;
//										if(debug_onoff.EC600N)
//											debug_printf("Device ID: %s, Key: %s, Value: %d\r\n", varDevice->deviceID, varDevice->key, varDevice->value);
//										state=VARIABLE_DEVICE_FOUND;
//										device_malloc= (void*)varDevice;
//                }
//            }
//        }
//    } else { 
        cJSON *current_element = NULL;
			//对象的键值对内部也是以链表的形式存储的，就像数组元素一样
        cJSON_ArrayForEach(current_element, params) {//遍历对象的键值对
////            Fixed_Device FixedDevice;
//						Fixed_Device *fixDevice = malloc(sizeof(Fixed_Device));
//						if(fixDevice==NULL)
//								return state;
            memset(fixDevice, 0, sizeof(Fixed_Device));
            if (cJSON_IsNumber(current_element)) {
                strncpy(fixDevice->key, current_element->string, sizeof(fixDevice->key) - 1);
							//sizeof(fixDevice->key) - 1:预留空间存放'\0's
                fixDevice->value = current_element->valueint;
								if(debug_onoff.EC600N)
										debug_printf("Key: %s, Value: %d\r\n", fixDevice->key, fixDevice->value); 
								state=1;
//								state=FIXED_DEVICE_FOUND;
								device_malloc=(void*)fixDevice;
						} else if (cJSON_IsString(current_element)) {
								// 判断是否为 "Realtime_Set" 键值对 
										strncpy(fixDevice->key, current_element->string, sizeof(fixDevice->key) - 1); // 将键存储到 fixDevice->key 中
						 
										// 从字符串中提取出 "1724" 并存储到 fixDevice->value 中
										fixDevice->value = extractNumber(current_element->valuestring, 8, 4); 
							
										// 将 Realtime_Set 字符串解析并存储到结构体中
										uint16_t year;
										sscanf(current_element->valuestring, "%4d%2d%2d%2d%2d%2d%2d",
													 &(year), &(datetime.month), &(datetime.day),
													 &(datetime.hour), &(datetime.min), &(datetime.sec),
													 &(datetime.week));
										datetime.year=year-2000;
								 
						}
        }
//    }

    cJSON_Delete(json);
		return state;
}

//void Extract_Command(char *keyword)
//{  
//    char *start, *end; 
//		uint8_t result=0;
//    start = (char*)g_tModule.EC600_RevBuf;//提取json语句
//    while ((start = strstr(start, keyword)) != NULL) {
//			//提取{}语句
//        start += strlen(keyword); // 移动到"QMTRECV"之后
//        if (*start == '{') {
//            end = strchr(start, '}'); // 查找匹配的'}'  ERROR
//            if (end != NULL) {
//                int length = end - start + 1; // 计算字符串长度，包括{}
//								char *json_string = (char*)malloc(length + 1);
//								 if (json_string) {
//										strncpy(json_string, start, length);
//										json_string[length] = '\0';  
//										 result = json_to_command(json_string);
//											free(json_string);//释放分配的内存
//								}   
//								 
//								
//								 //提取json语句中的键值
//								if (result == FIXED_DEVICE_FOUND) {
//									//壁挂炉设备，数值固定
//											Fixed_Device *fixDevice = (Fixed_Device*)device_malloc;
//											handle_Fixed_Convert(fixDevice);//结构体转换成控制命令
//											free(fixDevice); 
//								} else if (result == VARIABLE_DEVICE_FOUND ) {
//										//可变温控设备 
//											Variable_Device *varDevice = (Variable_Device*)device_malloc; 
//				//												handle_Variable_Convert(); 											
//											free(varDevice);  
//								}  		
//						}
//				 }
//				start = end + 1; // 移动到下一个字符，继续搜索
//				result=0;	 
//    }

//}

struct s_Ota_Addr
{					
	  char   OtaHostUrl[16];//255.255.255.255
		uint16_t   OtaPort;//0-65535
	
};
struct s_Ota_Addr Ota_Addr;
uint8_t ota_json_to_serverip(const char *json_string) { 
    // 解析JSON字符串
    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL) {
        debug_printf("Error before: %s\n", cJSON_GetErrorPtr());
        return 1;			 
    }

    // 提取OTA服务器地址
    cJSON *otaServerAddress = cJSON_GetObjectItemCaseSensitive(json, "OTA_address");
    if (cJSON_IsString(otaServerAddress) && (otaServerAddress->valuestring != NULL)) {
        snprintf(Ota_Addr.OtaHostUrl, sizeof(Ota_Addr.OtaHostUrl), "%s", otaServerAddress->valuestring);
    }

    // 提取OTA服务器端口号
    cJSON *serverPort = cJSON_GetObjectItemCaseSensitive(json, "OTA_port");
    if (cJSON_IsNumber(serverPort)) {
        Ota_Addr.OtaPort = (uint16_t)serverPort->valuedouble;
    }

    // 打印结果，验证是否正确解析和存储
		if(debug_onoff.EC600N){
			debug_printf("OTA Host URL: %s\n", Ota_Addr.OtaHostUrl);
			debug_printf("OTA Port: %u\n", Ota_Addr.OtaPort);
		}
    // 释放cJSON对象占用的内存
    cJSON_Delete(json);

    return 0;
}

char calib_time_buf[6];//12,34
uint8_t extract_realtime_set(const char *msg) {
    const char *keyword = "\"Realtime_Set\":";
    char *start = strstr(msg, keyword);// {"params":{"Realtime_Set":02,00}}
    if (start != NULL) {
//				memcpy(zero_buf,g_tModule.EC600_RevBuf,(size_t)g_tModule.EC600_BufLen);
				zero_cnt++;
        start += strlen(keyword); // 移动到关键字后的数字开始处
        char *end = strchr(start, '}'); // 假设数字后紧跟的是'}'
        if (end != NULL ) {
            // 拷贝数字到result数组中，注意保留逗号
            strncpy(calib_time_buf, start, end - start);
//            calib_time_buf[end - start] = '\0'; // 确保字符串以'\0'结尾
						zero_cnt++;
						return CALIB_TIME_FOUND; 
        }
    } 
		return 0;
}

 

void handle_subid(const char *sub_id,uint8_t *sub_id_array) 
{ 
    // 检查 sub_id 长度是否正确
    if (strlen(sub_id) != 8) {
        return;
    }

//    // 定义一个 uint8_t 数组用于存储转换后的值
//    uint8_t sub_id_array[4];

    // 转换 sub_id 字符串为 uint8_t 数组
    for (int i = 0; i < 4; i++) {
        char byte_str[3] = {sub_id[2*i], sub_id[2*i + 1], '\0'};
        sub_id_array[i] = (uint8_t)strtol(byte_str, NULL, 16);
    } 
}
 
 
 
// 处理函数实现
void handle_control(const char *sub_id, double temp_set, int sw, int mode) {
			// 定义一个 uint8_t 数组用于存储转换后的值
		uint8_t sub_id_array[4];
		handle_subid(sub_id,sub_id_array); 
	
		DeviceData device_data;
		device_data.temp_set=(uint16_t)(temp_set*10);
		device_data.mode=mode;
		device_data.sw=sw;
		// 入栈
		construct_send_command_to_fifo(0x06,sub_id_array,&device_data); 
}

 
// 主解析函数
void parse_variable_json(const char *json_string) {
    cJSON *root = cJSON_Parse(json_string);
    if (root == NULL) {
        return;
    }

    cJSON *action_item = cJSON_GetObjectItem(root, "action");
    if (action_item == NULL) {
        cJSON_Delete(root); 
        return;
    }

    const char *action = action_item->valuestring;
    cJSON *params = cJSON_GetObjectItem(root, "params"); 
		
		 //switch(action) 分条件进行解析 
    if (strcmp(action, "networkIn") == 0) {
        construct_send_command_to_fifo(0x01,NULL,NULL);//入网
    } else if (strcmp(action, "networkOut") == 0) {
        construct_send_command_to_fifo(0x02,NULL,NULL);//出网
    } else if (strcmp(action, "delete") == 0) {
        if (params != NULL) {
            cJSON *sub_id_item = cJSON_GetObjectItem(params, "sub_id");
            if (sub_id_item != NULL) {
							    // 定义一个 uint8_t 数组用于存储转换后的值
								uint8_t sub_id_array[4];
                handle_subid(sub_id_item->valuestring,sub_id_array); 
								// 入栈
								construct_send_command_to_fifo(0x04,sub_id_array,NULL);
            }
        }
    }  else if (strcmp(action, "status") == 0) {
        if (params != NULL) { 
							// 入栈
							construct_send_command_to_fifo(0x05,NULL,NULL);  
        }
    } else if (strcmp(action, "control") == 0) {
        if (params != NULL) {
            cJSON *sub_id_item = cJSON_GetObjectItem(params, "sub_id");
            cJSON *temp_set_item = cJSON_GetObjectItem(params, "temp_set");
            cJSON *sw_item = cJSON_GetObjectItem(params, "sw");
            cJSON *mode_item = cJSON_GetObjectItem(params, "mode");
            if (sub_id_item != NULL && temp_set_item != NULL && sw_item != NULL && mode_item != NULL) {
                handle_control(sub_id_item->valuestring, temp_set_item->valuedouble, sw_item->valueint, mode_item->valueint);
            }  
						//缩短上报时间
						
        }
    } else if (strcmp(action, "idlist") == 0) {
					// 入栈
					construct_send_command_to_fifo(0x0A,NULL,NULL);
    } else if (strcmp(action, "update") == 0) {
        if (params != NULL) {
            cJSON *sub_id_item = cJSON_GetObjectItem(params, "sub_id");
            cJSON *OTA_address_item = cJSON_GetObjectItem(params, "OTA_address");
            cJSON *OTA_port_item = cJSON_GetObjectItem(params, "OTA_port");
            if (sub_id_item != NULL && OTA_address_item != NULL && OTA_port_item != NULL) {
//                handle_update(sub_id_item->valuestring, OTA_address_item->valuestring, OTA_port_item->valueint);
            }
        }
    } 
		
    cJSON_Delete(root);
}

#define MAX_MESSAGES 5 // 总的最大消息数为5
uint8_t	temper_rapif_reply_flag=0;
uint8_t	temper_rapif_reply_cnt=0;
void process_messages(char *messages) {
//    char *msg_copy = strdup(messages); // 创建消息的副本
    char *line;
    char *saveptr;
    int index = 0; // 用于追踪总的已处理消息数

    line = strtok_r(messages, "\r\n", &saveptr); // 分割字符串，将\r替换成\0
    while (line != NULL && index < MAX_MESSAGES) {
        if (strstr(line, "property/post_reply") != NULL) {//壁挂炉主题
							const char *json_start = strchr(line, '{');
							if (json_start != NULL) { 
//									//提取json语句中的键值 
//										uint8_t result = extract_realtime_set(json_start);//时间校准解析
//										if(result==0)
									uint8_t result;
									Fixed_Device fixDevice;
									result=parse_fixed_json(json_start,&fixDevice);//cjson解析 
									 
									if (result == 1) {
										//壁挂炉设备，数值固定
//												Fixed_Device *fixDevice = (Fixed_Device*)device_malloc;
												handle_Fixed_Convert(&fixDevice);//结构体转换成控制命令
//												free(fixDevice); 
									} 
//									else if(result == CALIB_TIME_FOUND ){
//												handle_Fixed_Calib_Convert();
//									}else if (result == VARIABLE_DEVICE_FOUND ) {
//											//可变温控设备 
//												Variable_Device *varDevice = (Variable_Device*)device_malloc; 
//	//											handle_Variable_Convert(); 											
//												free(varDevice);  
//									}  		 
							}
        } else if(strstr(line, "tempercontrol/distribution") != NULL){//温控器主题
							//上报时间缩短
							temper_rapif_reply_flag=1;
							temper_rapif_reply_cnt=120;//120;//2分钟没有接收到命令，停止快速上报
							const char *json_start = strchr(line, '{');
							if (json_start != NULL) {  
									uint8_t result; 
									 parse_variable_json(json_start);//cjson解析  
							}			 
				}else if (strstr(line, "/ota/device/upgrade") != NULL ) {//ota升级主题
            // 处理OTA消息
            const char *json_start = strchr(line, '{');
            if (json_start != NULL) {
								if(ota_json_to_serverip(json_start)==0)//解析ota地址和端口号
									update_flag=1;//准备OTA升级标志
									//OTA存储地址
									OTA_store_addr=*(uint32_t*)UPGRADEaddr;
									if(OTA_store_addr==APP3adress)
										OTA_store_addr=APP2adress;
									else  
										OTA_store_addr=APP3adress; 								
						}		
        }
				index++;
        line = strtok_r(NULL, "\r\n", &saveptr);
    }

//    free(msg_copy); // 释放副本
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description: 数据收发部分
 * @return 0：检测未完成；MD_ERR：错误
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//发布流程标识
typedef enum
{
//    SUB_MESG_GET = 0,
	  SUB_SEND1=0,        				//发送topic命令
	  SUB_SEND2,        				   //发送message命令 
		SUB_GET,										//接收命令 
    SUB_SOCKET,       				 //查询Socket 
  	SUB_DISC,
//		SUB_CLOSE,                  //关闭连接
	  SUB_WAIT    								//连接配置信息初始化 
} SUB_STATE;

uint8_t mqtt_data(void)
{
    switch (ucStateNum)
    {
			case SUB_SEND1: 
//			  if(mqtt_revflag==1)
//				{
//					ucStateNum = SUB_GET;
//					mqtt_revflag=0;
//				} 
//				else 
//				{
						if(topic_send_wait!=0)
						{
							if(fat_send_wait_cmdres_nonblocking(topic_buf,1000))			
							{
//								debug_printf("uart recv OK: %s\r\n",g_tModule.EC600_RevBuf);
								//收到>
								if (fat_cmdres_keyword_matching(">"))
								{
										ucErrorTimes = 0;
										ucStateNum++;
								}
								else
								{
										//发送3次得不到正确应答
										if (ucErrorTimes++ > 3)
										{
												ucStateNum = MD_ERR;
										}
								} 
							}
						}
						else
						{
								ucStateNum = SUB_SOCKET;
//							ucStateNum = SUB_SEND1;
						} 
//					} 
        break;
    case SUB_SEND2:
				if(fat_send_wait_cmdres_nonblocking(message_buf,1000))
				{
						//收到OK
            if (fat_cmdres_keyword_matching("OK"))
            {
//							debug_printf("uart recv OK: %s\r\n",g_tModule.EC600_RevBuf);
								if(topic_send_wait!=15)
									topic_send_last = topic_send_wait;//记录当前发送直针
								topic_send_wait = 0;
								topic_send_ok = 1; //发送完成标识
								topic_count++;		
							
//								debug_printf("发送<OK>\r");
                ucErrorTimes = 0;
                ucStateNum=SUB_SOCKET;
//								ucStateNum = SUB_SEND1;
								
            }
            else
            {
//							debug_printf("uart recv ERROR: %s\r\n",g_tModule.EC600_RevBuf);
								//发送3次得不到正确应答
                if (ucErrorTimes++ > 3)
                {
                    ucStateNum = MD_ERR;
                }
            }					
        }
        break;
		
 
    //查询socket缓存是否有数据
    case SUB_GET:
//			  if (fat_send_wait_cmdres_blocking("AT+QMTCFG=\"recv/mode\",0\r\n", 1000))		
//				if (fat_send_wait_cmdres_blocking("AT+QMTRECV?\r", 1000))
//        if (fat_send_wait_cmdres_blocking("AT+QMTRECV?\r\n", 1000))	
//				if (fat_send_wait_cmdres_blocking("AT+QMTRECV=0\r\n", 1000))
//        {
//					debug_printf("uart recv OK: %s\r\n",g_tModule.EC600_RevBuf);
//						//收到+QMTRECV:
////				  fat_cmdres_keyword_matching("+QMTRECV:"); 
//            if (fat_cmdres_keyword_matching("post_reply"))
//            { 
//								//收到1表示有数据。返回的参数1为clientID，后5参数为<store_status>
//								if (fat_cmdres_keyword_matching("1"))
//								{
//									ucErrorTimes = 0;
//									ucStateNum = SUB_SOCKET;
//								}
//								else
//								{ 
//									ucErrorTimes = 0;
//									ucStateNum = SUB_SEND1;
//								}	
//            }
//            else
//            { 
//								//发送3次得不到正确应答
//                if (ucErrorTimes++ > 3)
//                {
//                    ucStateNum = MD_ERR;
//                }
//            }
//        }		
				if (fat_send_wait_cmdres_blocking("AT+QMTRECV=0\r\n", 1000))
        {
					debug_printf("uart recv OK: %s\r\n",g_tModule.EC600_RevBuf);
						//收到+QMTRECV:
//				  fat_cmdres_keyword_matching("+QMTRECV:"); 
            if (fat_cmdres_keyword_matching("post_reply"))
            { 
//									Extract_Command("post_reply\",\""); 
									ucErrorTimes = 0;
									ucStateNum = SUB_SEND1;
//								}	
            }
            else
            {
							
							ucStateNum = SUB_SEND1;
//								//发送3次得不到正确应答
//                if (ucErrorTimes++ > 3)
//                {
//                    ucStateNum = MD_ERR;
//                }
            }
        }
        break;
		//截取Topic和Data
//    case SUB_SOCKET:
////				if (fat_send_wait_cmdres_blocking("AT+QMTRECV=0,0\r\n", 1000))
////				if (fat_send_wait_cmdres_blocking("AT+QMTRECV?\r", 1000))
//				if (fat_send_wait_cmdres_blocking("AT+QMTRECV=0\r\n", 1000))
//				{
////					debug_printf("uart recv OK: %s\r\n",g_tModule.EC600_RevBuf);
//						if(fat_cmdres_keyword_matching("post_reply"))					
////						if (fat_cmdres_keyword_matching("+QMTRECV:"))
//						{
////							debug_printf("uart recv OK: %s",g_tModule.EC600_RevBuf);
//// 							  if(match_recv_data((char*)g_tModule.EC600_RevBuf))
////								{
////                    handle_MQTTConvert();
////								}
//								
//								char *json_string = extract_json((char*)g_tModule.EC600_RevBuf);//提取json语句
//								if (json_string) {
//									uint8_t result = json_to_command(json_string);
//										free(json_string);//释放分配的内存
//										if (result == FIXED_DEVICE_FOUND) {
//											//壁挂炉设备，数值固定
//													Fixed_Device *fixDevice = (Fixed_Device*)device_malloc;
// 													handle_Fixed_Convert(fixDevice);//结构体转换成控制命令
//													free(fixDevice); 
//										} else if (result == VARIABLE_DEVICE_FOUND ) {
//												//可变温控设备 
//											    Variable_Device *varDevice = (Variable_Device*)device_malloc; 
////												handle_Variable_Convert(); 											
//													free(varDevice);  
//										}   
//								} 				 
//								ucErrorTimes = 0;
////								ucStateNum = SUB_GET;
////								ucStateNum = SUB_SOCKET; //没有接收到命令，查看是否需要发送
//						} 
//						if(topic_send_wait==1) 
//								ucStateNum=SUB_SEND1;
//								//发送3次得不到正确应答
////                if (ucErrorTimes++ > 3)
////                {
////                    ucStateNum = MD_ERR;
////                } 
//				}
//				break;

		case SUB_SOCKET:
				if (fat_send_wait_cmdres_nonblocking("AT+QMTRECV=0\r\n", 1000))
//				if (wait_timeout_nonblocking(100))
				{
						if(fat_cmdres_keyword_matching("+QMTRECV")){
//								memcpy(zero_buf,g_tModule.EC600_RevBuf,(size_t)g_tModule.EC600_BufLen);
								process_messages((char*)g_tModule.EC600_RevBuf);  
								zero_cnt++;
						}
//						if(fat_cmdres_keyword_matching("property/post_reply"))		//设备控制		
//						{						
//								char *json_string = extract_json((char*)g_tModule.EC600_RevBuf);//提取json语句
//								if (json_string) {
//									uint8_t result = json_to_command(json_string);
//										free(json_string);//释放分配的内存
//										if (result == FIXED_DEVICE_FOUND) {
//											//壁挂炉设备，数值固定
//													Fixed_Device *fixDevice = (Fixed_Device*)device_malloc;
// 													handle_Fixed_Convert(fixDevice);//结构体转换成控制命令
//													free(fixDevice); 
//										} else if (result == VARIABLE_DEVICE_FOUND ) {
//												//可变温控设备 
//											    Variable_Device *varDevice = (Variable_Device*)device_malloc; 
////												handle_Variable_Convert(); 											
//													free(varDevice);  
//										}   
//								} 				 

//						}
//						else if(fat_cmdres_keyword_matching("/ota/device/upgrade"))//ota升级
//						{
//							//提取cjson语句
//							char *json_string = extract_json((char*)g_tModule.EC600_RevBuf);//提取json语句
//							//cjson中的OTA地址、端口号
//							if (json_string) {
//									ota_json_to_serverip(json_string); //解析OTA地址和端口号
//									free(json_string);//释放分配的内存 
//							} 								
//							update_flag=1;
//						}
						
						ucErrorTimes = 0;
						//update_flag写入flash
						if(update_flag==1){		
							ucStateNum = SUB_DISC; //断开连接
						}else if(topic_send_wait>0) 
							ucStateNum = SUB_SEND1; //没有接收到命令，查看是否需要发送
						else
							ucStateNum = SUB_WAIT;

				}
				break;


		case SUB_WAIT:				
				//收到OK
		    if(wait_timeout(300))
				{ 
					 //ucStateNum = SUB_SEND1;
					ucStateNum = SUB_SOCKET;
				}
        break; 
		 
				// 构建MQTT DISCONNECT命令  
//			if (fat_send_wait_cmdres_blocking("AT+QMTDISC=0\r\n",  500)) {
//					if (fat_cmdres_keyword_matching("+QMTDISC: 0,0")) {
//							ucErrorTimes = 0;
//							return MD_OK;
//					}
//					else
//					{
//						if (ucErrorTimes++ > 10) 
//								return MD_ERR;
//					}					
//			}		

//		case SUB_CLOSE:		
//				ucStateNum = send_at_and_check("AT+QMTCLOSE=0\r\n", "+QMTCLOSE: 0,0", 1000, 10,SUB_DISC ); 		
//				break;	
		case SUB_DISC:
				// 构建MQTT DISCONNECT命令  
				if (fat_send_wait_cmdres_blocking("AT+QMTDISC=0\r\n",  500)) {
						if (fat_cmdres_keyword_matching("+QMTDISC: 0,0")) {
								ucErrorTimes = 0;
								ucStateNum = MD_OK;
						}
						else
						{
							if (ucErrorTimes++ > 10) 
									ucStateNum = MD_ERR;
						}					
				}					
//				ucStateNum = send_at_and_check("AT+QMTDISC=0\r\n", "+QMTDISC: 0,0", 1000, 10, MD_OK, MD_ERR); 		
				break;		
		//错误跳至飞行模式
    case MD_ERR:
				ucStateNum = 0;
		    if(debug_onoff.EC600N)  debug_printf("飞行&&\r"); 
        return MD_ERR;
		case MD_OK:
				ucStateNum = 0; 
        return MD_OK;			
    default:
        break;
    }
    return 0;
}


/**
 * @description: Socket连接相关配置初始化
 * @param None
 * @return 0：检测未完成；MD_OK：模块已就绪；MD_ERR：错误，不满足工作状态
 */ 
enum TcpConnectionState { STATE_APN_CONFIG, STATE_ACTIVATE_APN,STATE_DEACTIVATE_APN, STATE_TCP_CONNECT,STATE_ERROR };
  
uint8_t tcp_connet_parm_init(void) {
    char buffer[255];
    switch (ucStateNum) {
        case STATE_APN_CONFIG://配置场景1，APN 配置为"CMNET"
            ucStateNum = send_at_and_check("AT+QICSGP=1,1,\"CMNET\",\"\",\"\",1\r\n", "OK", 1000, 10, STATE_DEACTIVATE_APN, MD_ERR);
            break;

				case STATE_DEACTIVATE_APN://反激活场景1
//            ucStateNum = send_at_and_check("AT+DEACT=1\r\n", "OK", 2000, 3, STATE_ACTIVATE_APN, MD_ERR);
						if (fat_send_wait_cmdres_blocking("AT+DEACT=1\r\n", 1000)) {
								if (fat_cmdres_keyword_matching("OK")) { 
										ucErrorTimes = 0; 
//										ucStateNum=0;
//									  state=STATE_ACTIVATE_APN;
										ucStateNum=STATE_ACTIVATE_APN;
								}
								else
								{
//									if(fat_cmdres_keyword_matching("ERROR"))
//										ucStateNum = STATE_ERROR;
									if (ucErrorTimes++ > 0)  
										{
											ucErrorTimes=0;
//											ucStateNum = MD_ERR;
											ucStateNum =	STATE_ACTIVATE_APN;
										}
								}					
						} 				
            break;	
						
        case STATE_ACTIVATE_APN://激活场景1
//            ucStateNum = send_at_and_check("AT+QIACT=1\r\n", "OK", 2000, 10, STATE_TCP_CONNECT, STATE_DEACTIVATE_APN);
						if (fat_send_wait_cmdres_blocking("AT+QIACT=1\r\n", 1000)) {
//								if(fat_cmdres_keyword_matching("ERROR"))
//										ucStateNum = STATE_ERROR;
								if (fat_cmdres_keyword_matching("OK")) { 
										ucErrorTimes = 0;
										ucStateNum = STATE_TCP_CONNECT;
								}
								else
								{
									if (ucErrorTimes++ > 0) 
										{
											ucErrorTimes = 0;
//											ucStateNum = STATE_DEACTIVATE_APN;//反激活
											ucStateNum = STATE_TCP_CONNECT;
									}
								}					
						}				
            break;

//        case STATE_ERROR://错误查询
////            ucStateNum = send_at_and_check("AT+QIACT=1\r\n", "OK", 1000, 10, STATE_TCP_CONNECT, STATE_DEACTIVATE_APN);
//						if (fat_send_wait_cmdres_blocking("AT+QIGETERROR\r\n", 1000)) {
//								if (fat_cmdres_keyword_matching("+QIGETERROR:")) { 
//										ucErrorTimes = 0;
////										ucStateNum = MD_ERR;
//								}
//								else
//								{ 
//									if (ucErrorTimes++ > 3) {
//											ucErrorTimes = 0;
//											ucStateNum = MD_ERR;//反激活
//									}
//								}					
//						}				
//            break;				
//        case STATE_ERROR://错误查询
//						if (fat_send_wait_cmdres_blocking("AT+QISTATE=0,1\r\n", 1000)) {
//								if (fat_cmdres_keyword_matching("OK")) { 
//										ucErrorTimes = 0;
////										ucStateNum = MD_ERR;
//								}
//								else
//								{ 
//									if (ucErrorTimes++ > 3) {
//											ucErrorTimes = 0;
//											ucStateNum = MD_ERR;//反激活
//									}
//								}					
//						}				
//            break;							
        case STATE_TCP_CONNECT://建立连接 透传模式
					  //////////////  
//						snprintf(buffer, sizeof(buffer), "AT+QIOPEN=1,0,\"TCP\",\"%s\",%d,0,2\r\n",Ota_Addr.OtaHostUrl, Ota_Addr.OtaPort);
            snprintf(buffer, sizeof(buffer), "AT+QIOPEN=1,0,\"TCP\",\"47.95.4.163\",8066,0,2\r\n");
//            ucStateNum = send_at_and_check(buffer, "CONNECT", 1000, 5, MD_OK);
						if (fat_send_wait_cmdres_nonblocking(buffer, 3000))
						{
								if (fat_cmdres_keyword_matching("CONNECT"))
								{
										ucErrorTimes = 0;
										ucStateNum = MD_OK;
								}
								else
								{
//									if(fat_cmdres_keyword_matching("ERROR"))
//										ucStateNum = STATE_ERROR;									
										//发送5次得不到正确应答
										if (ucErrorTimes++ > 5)
										{ 
												if(++tcp_connect_cnt>3){
													update_flag=0;
													tcp_connect_cnt=0;
												}
												ucErrorTimes = 0;
//												ucStateNum = STATE_DEACTIVATE_APN;//反激活
												ucStateNum = MD_ERR;
										}
								}
						}
						break;				
        case MD_OK:
            ucStateNum = 0;
            return MD_OK;
        case MD_ERR:
            ucStateNum = 0;
            return MD_ERR;
        default:
            break;
    }
    return 0;  
}

uint16_t update_idex=0;//升级包序号
uint16_t packege_sum=0;//升级包总包数
uint32_t flash_page_addr; // Flash写入的起始地址
uint8_t flash_buffer[PAGE_SIZE]; // 数据缓存
uint16_t flash_buffer_index; // 缓存中当前数据的索引
int save_ota_file(const uint8_t* data, uint16_t size) {
	  uint8_t  save_flag=0;
		uint16_t save_size;
		uint16_t flash_buffer_idex_back=flash_buffer_index;//存储失败时恢复
//		static uint8_t err=0;
    while (size > 0) {
        uint16_t space_left = PAGE_SIZE - flash_buffer_index; // 计算缓存剩余空间 
        if (size <= space_left) {
            // 如果剩余空间足够，直接复制数据到缓存
            memcpy(&flash_buffer[flash_buffer_index], data, size);
            flash_buffer_index += size;
            size = 0; // 已处理完所有接收的数据
						if(update_idex==(packege_sum-1)){	//接收到最后一包
								save_flag=3; 
						}
        } else {
            // 如果剩余空间不足，先填满缓存，然后尝试写入Flash
            memcpy(&flash_buffer[flash_buffer_index], data, space_left);
            flash_buffer_index += space_left;
            data += space_left;
            size -= space_left;
						save_flag=1; 
						if(update_idex==(packege_sum-1)){	//接收到最后一包
								save_flag=2; 
						}
				}
				
				if(save_flag>0){ 
							// 使用flush_buffer_to_flash函数尝试写入Flash
							save_size=(save_flag<3?PAGE_SIZE:flash_buffer_index);//存储大小 
							int write_status = flush_buffer_to_flash(flash_page_addr, flash_buffer, save_size);
//							if(update_idex==8 && err<3){
//								write_status=1;
//								err++; 
//							}					
							if (write_status == HAL_OK) {
									if(debug_onoff.EC600N)
										debug_printf("存储地址:%d，包序号%d\r\n",flash_page_addr,update_idex);
									flash_page_addr += PAGE_SIZE; // 成功写入，更新Flash地址
									flash_buffer_index = 0; // 重置缓存索引 
							} else {
									// 如果写入失败，处理失败逻辑，可能需要记录错误、发送警报等
									debug_printf("flash存储失败");
									flash_buffer_index=flash_buffer_idex_back;//恢复buffer存储序号
									return write_status; // 根据实际情况可能需要返回错误码
							}
							
							
							// 将剩余数据复制到缓存的开头
							if(size>0){
								memcpy(flash_buffer, data, size);
								flash_buffer_index = size;
								if(save_flag==2){//刚存储完1页正好接收到最后一包
									write_status = flush_buffer_to_flash(flash_page_addr, flash_buffer, flash_buffer_index);	
									if(write_status!=0) {
										flash_buffer_index-=size;//最后一包数据没有跨页的特殊处理
										return write_status;	
									}										
								}									
								size=0;//漏掉
							}
							save_flag=0;
				}
       
    }
		return 0;
} 
// 定义状态枚举
typedef enum {
    WAIT_FOR_START,
    SEND_RESPONSE,
    RECEIVE_FILE, 
		RECEIVE_BREAK, 
    DISCONNECT,
		RECONNECT
} ConnectionState;


#define INDEX_HEAD                        0   ///
#define INDEX_LEN                         2   ///
#define INDEX_CMD                        	4   ///
#define INDEX_IDEX                        6   ///
#define INDEX_RESULT                      8  
#define INDEX_CRC                      		10   ///
//uint8_t file_start_rev[] = {0xA3,0x3A,0x0C,0x00,0x3B,0xA5,0x00,0x00,0x00,0x00,0x00,0x00 };
//uint8_t file_transmit_rev[]={0xA3,0x3A,0x0C,0x00,0x5A,0xA5,0x00,0x00,0x00,0x00,0x00,0x00};
//uint8_t file_restart_rev[]=	{0xA3,0x3A,0x0C,0x00,0x3B,0xA5,0x00,0x00,0x01,0x00,0x00,0x00};
const uint8_t file_end_rev[]={0xA3,0x3A,0x0A,0x00,0x7A,0xA5,0x00,0x00,0x00,0x00 };
//const uint8_t file_end[]={0xA3,0x3A,0xAA,0x00,0x69,0xA5,0x00,0x00,0x00,0x00 };


// 假设的发送和接收函数原型 
#define FILE_Header 0x3AA3
#define FILE_TRANSMIT 0xA55A  
#define FILE_END 0xA569
#define FILE_ERROR 0xA56A   
#define FILE_TIMEOUT 0xA56B   
#define FILE_INFO 	0xA53B   
#define FILE_REV_SIZE 12
//#define FILE_REV_TIMEOUT 0xA57A

// 使用枚举类型替代宏定义来表示返回值和命令代码
typedef enum {
		DATA_CLIENT_DISCONNECT = 0,
    DATA_OK = 1,
		DATA_RESTART =2,
    DATA_SEND_END = 3,
    DATA_SERVER_END = 4
} DataStatus;

//	 file_start_rev[INDEX_IDEX]=update_idex&0x00FF; 
//	 file_start_rev[INDEX_IDEX+1]=(update_idex>>8)&0x00FF; 			
//	 uint16_t crc=CRC16(file_start_rev,FILE_REV_SIZE-2);
//	 file_start_rev[INDEX_CRC]=crc&0x00FF;
//	 file_start_rev[INDEX_CRC+1]=crc>>8;
//		EC600_Usart_Send((uint8_t*)file_start_rev,sizeof(file_start_rev)); 
//		if(debug_onoff.EC600N)
//			 SendDataAsHex(file_start_rev,FILE_REV_SIZE);
typedef struct {
    uint16_t header;
    uint16_t length;
    uint16_t cmd;
    uint16_t index;
    uint16_t result; // 数据部分的最大长度
    uint16_t crc;
} RevProtocol;
void construct_revmessage( uint16_t cmd, uint16_t index, uint16_t result,void (*send_func)(uint8_t*, uint16_t))  
{
    RevProtocol msg;
    msg.header = FILE_Header;
    msg.cmd = cmd;
    msg.index = index;
    msg.result = result; 
    msg.length = sizeof(RevProtocol); // 这里包括了整个结构体的大小
    msg.crc = CRC16((uint8_t*)&msg, msg.length - 2); // 假设CRC不包括CRC字段本身

    // 使用提供的发送函数发送消息
    send_func((uint8_t*)&msg, msg.length);
		if(debug_onoff.EC600N)
			SendDataAsHex((uint8_t*)&msg,msg.length);
}

uint8_t verify_message(uint8_t* buffer, uint16_t bufLen) {
    // 检查消息头
    uint16_t header = (buffer[INDEX_HEAD] & 0x00FF) + ((buffer[INDEX_HEAD+1] << 8) & 0xFF00);
    if (header != FILE_Header) return 0;

    // 检查长度是否匹配
    uint16_t length = (buffer[INDEX_LEN] & 0x00FF) + ((buffer[INDEX_LEN+1] << 8) & 0xFF00);
    if (length != bufLen) return 0;

    // 检查CRC
    if (CRC16(buffer, bufLen) != 0) return 0;

    // 所有检查通过
    return 1;
}

uint8_t ote_transport_protocol(void)
{
	uint8_t ret=DATA_RESTART;
	uint16_t  temp;
	static uint8_t err=0,step=0;
	/* 命令有响应，在响应数据中查找匹配项目 */
	if (g_tModule.EC600_RevFlag)
	{ 
			if(debug_onoff.EC600N)
					SendDataAsHex(g_tModule.EC600_RevBuf,g_tModule.EC600_BufLen);
			g_tModule.EC600_RevFlag=0;	
		
			if(file_end_flag==1)
			{ 
					err=0;
					step=0;		
					file_end_flag=0;				
					return DATA_SEND_END;//文件传输结束
			}		
			else
			{
					// 使用简化的验证函数
					if (!verify_message(g_tModule.EC600_RevBuf, g_tModule.EC600_BufLen)) {
							ret=DATA_RESTART; // 验证失败
					}
					else{ 
							// 根据CMD处理消息
							uint16_t cmd = (g_tModule.EC600_RevBuf[INDEX_CMD] & 0x00FF) + ((g_tModule.EC600_RevBuf[INDEX_CMD+1] << 8) & 0xFF00);						
							switch (cmd) {
									case FILE_TIMEOUT:
									case FILE_ERROR:
											err=0;
											step=0;
											return DATA_SERVER_END;
									default:
									 ///////////////包序号idex/包总数sum
									temp=((g_tModule.EC600_RevBuf[INDEX_IDEX]&0x00FF)+((g_tModule.EC600_RevBuf[INDEX_IDEX+1]<<8)&0xFF00));	
									if(cmd==FILE_INFO) {
												packege_sum=temp;//存储总文件包
//											flash_mass_erase(OTA_store_addr,48*1024);//整块擦除  
												construct_revmessage(FILE_INFO,update_idex,stringToUint16(Device_Flash.iotCode),EC600_Usart_Send); 
												step=1;
												return DATA_OK;
									} 										
									else if(cmd==FILE_TRANSMIT){ 
											if(temp==update_idex){
												 ///////////////FLASH:分页边擦边存
												 if(save_ota_file(g_tModule.EC600_RevBuf+8, g_tModule.EC600_BufLen-10)==0)
													//全部擦除，直接存储
//											 if(flash_write(flash_page_addr+256*update_idex,g_tModule.EC600_RevBuf+8,g_tModule.EC600_BufLen-10)==(-1))
												 {
													 construct_revmessage(FILE_TRANSMIT,update_idex,0,EC600_Usart_Send);  
//													 update_idex++;   
													 err=0;
													 return DATA_OK;
												 }
											 //回应服务器
//												HAL_Delay(100);     
										 } 
								 }												
								 break;										
							}						
						
					}
			 }
	}
	
	if(ret==DATA_RESTART){
		if(step==0 || ++err>=3)
		{
			//发送错误指令，让服务器主动断开连接
				EC600_Usart_Send((uint8_t*)file_end_rev,sizeof(file_end_rev)); 
				if(debug_onoff.EC600N)
					SendDataAsHex((uint8_t*)file_end_rev,0x0A);		
				err=0; 
				step=0;
			return DATA_CLIENT_DISCONNECT;
		}		
		//step=1时 接收存在不正确/未接受到/存储错误： 重发指令
			construct_revmessage(FILE_INFO,update_idex,1,EC600_Usart_Send);  
			return ret;
	}
 
} 

/**
 * @description: TCP数据收发部分
 * @return 0：检测未完成；MD_ERR：错误，不满足工作状态
 */
uint8_t tcp_data(void){
		uint8_t ret=0;
		static uint8_t first_flag=1;
    switch (ucStateNum) { 	
				case WAIT_FOR_START:
					if(wait_timeout_nonblocking(10000))
					{ 
						ret=ote_transport_protocol();
						if (ret==DATA_OK){
							if(first_flag==1)//初始化变量
							{
								flash_buffer_index=0;//flash缓存序号
								flash_page_addr=OTA_store_addr; 
								update_idex=0;//升级包序号清零		
								first_flag=0;
							}								
							ucStateNum=RECEIVE_FILE;
						}
						else
						{
							ucStateNum=DISCONNECT;
//							if( ret==DATA_SEND_END)//不需要升级
//							{
//								update_flag=0;  
//							}
						}
					} 
					break;				
        case RECEIVE_FILE:
					// 实现接收文件逻辑
						if(wait_timeout_nonblocking(5000))
            { 
								ret=ote_transport_protocol();
								if (ret==DATA_OK){ //传送数据
										update_idex++;  
//									ucStateNum = RECEIVE_BREAK;
								}else if( ret==DATA_SEND_END){//升级成功
									//传输数据结束
											ucStateNum=DISCONNECT;
											update_flag=0; 
											if(update_idex==packege_sum)
											{
												bk_flash_WriteOnePage_uint8(UPGRADEaddr, (uint8_t*)&OTA_store_addr, 4);
												debug_printf("传输成功\r\n");
												HAL_Delay(300);
												NVIC_SystemReset(); 
											}
								}else if(ret==DATA_SERVER_END){//服务器主动断开
											ucStateNum = DISCONNECT;											
								}else if(ret==DATA_CLIENT_DISCONNECT){//客户端主动断开
											ucStateNum = DISCONNECT;									
								}else if(ret==DATA_RESTART){//不处理
										ucStateNum = RECEIVE_FILE;
//									update_idex=0;								
								}
						}	 
            break; 
//        case RECEIVE_BREAK:
//						if(wait_timeout(200)==1)
//							ucStateNum = RECEIVE_FILE;	
//            break; 						
				//退出透传模式
//				case MD_ERR:				
//						if (fat_send_wait_cmdres_nonblocking("+++", 1000))
//						{
//								//收到OK
//								if (fat_cmdres_keyword_matching("OK"))
//								{
//		//                FAT_DBGPRT_INFO("uart recv: %s\r\n", cFatUartRecvBuf);
//										ucErrorTimes = 0;
//										ucStateNum = DISCONNECT;
//								}
//								else
//								{
//										//发送5次得不到正确应答
//										if (ucErrorTimes++ > 5)
//										{
//												ucStateNum = MD_ERR;
//										}
//								}
//						}
//						break;   
				case DISCONNECT:
						// 断开连接处理
						if (fat_send_wait_cmdres_blocking("AT+QICLOSE=0\r\n", 500))
						{
								//收到OK
								if (fat_cmdres_keyword_matching("OK"))
								{ 
										ucErrorTimes = 0; 
									  if(update_flag==1)//重新连接
										{
											state=MD_CONNETINIT_TCP;
											ucStateNum =STATE_TCP_CONNECT; 
										}
										else//连接mqtt
										{ 
											ucStateNum =MD_OK; 										
										}
								}
								else
								{
										//发送5次得不到正确应答
										if (ucErrorTimes++ > 5)
										{ 
												ucErrorTimes=0;
												ucStateNum = MD_ERR;
										}
								}
						}				 
						break; 
				case RECONNECT:
						if(update_flag==0){//更新完毕
								debug_printf("传输成功\r\n");
								HAL_Delay(500);
								NVIC_SystemReset(); 
						}else{					
							state=MD_CONNETINIT_TCP;
							ucStateNum =STATE_TCP_CONNECT; 
						}	
				case MD_OK:
						ucStateNum=0;
						return MD_OK;							
				case MD_ERR:
						ucStateNum=0;
						return MD_ERR;				
    }
		return 0;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description: 飞行模式处理函数
 * @param None
 * @return 0：检测未完成；MD_WORK_STA_CHK：重新开启网络跳至模块状态检测；MD_ERR：错误
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
uint8_t module_flightmode(void) {
    switch (ucStateNum) {
        case 0x00:
            ucFlightModeTimes++;
            if(debug_onoff.EC600N) debug_printf("进入飞行模式次数：%d\r\n",ucFlightModeTimes);
            ucStateNum = (ucFlightModeTimes == 2) ? MD_ERR : 0x02; // Skip to sending AT+CFUN=0 if not reset condition
            break;
        case 0x02:
            ucStateNum = send_at_and_check("AT+CFUN=0\r\n", "OK", 2000, 5, 0x03, MD_ERR);
            break;
        case 0x03:
            if (wait_timeout(2000)) 
							ucStateNum++;
            break;
        case 0x04:
            ucStateNum = send_at_and_check("AT+CFUN=1\r\n", "OK", 2000, 5, 0, MD_ERR);
            if (ucStateNum == 0 ) {
								if(debug_onoff.EC600N) debug_printf("再开启功能\r\n");										 
								ucStateNum = 0;
								return MD_WORK_STA_CHK;  
            }
            break;
        case MD_ERR:
						if (ucFlightModeTimes == 2){//已经两次进入飞行模式，则复位
								if(debug_onoff.EC600N)  debug_printf("二次飞行模式，复位模块\r\n");
						}					
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
//uint32_t outlinetimes = 0;//断线重连次数
void  Handle_Lte(void)
{
//    static uint8_t state = MD_RESET;
    uint8_t ret = 0;
    switch (state)
    {
//			case MD_WAIT://模块上电等待 
//				if(module_power_on()) //开机
//				{ 
//						wait_timeout(LTE_POWER_ON_WAIT_TIME); //复位等待//5s 
//						state = MD_AT_REQ;
//				}
//				break;
//			case MD_RESET://复位模块，等待5秒
//        if (module_reset())
//        {
//						wait_timeout(LTE_POWER_ON_WAIT_TIME); //复位等待//5s
//					  //if(debug_onoff.EC600N)   
//							debug_printf("Reset-%d-!!!!!!!!!!!!!!!!!!!!!!\r\n",++outlinetimes); 
//					  state = MD_AT_REQ;
//					  Gaspower_Command(GAS_SET_RSSI,1,Gaspower_index);
//        }
//        break;
			
			

//		case MD_RESET://复位模块，等待5秒
//			if (module_reset())
//			{
////					wait_timeout(LTE_POWER_ON_WAIT_TIME); //复位等待//5s
//					//if(debug_onoff.EC600N)   
//						debug_printf("Reset-%d-!!!!!!!!!!!!!!!!!!!!!!\r\n",outlinetimes);
//					outlinetimes++; 
//					Gaspower_Command(GAS_SET_RSSI,1,Gaspower_index);
////					if(outlinetimes==1)
//						state = MD_WAIT;
////					else
////						state = MD_AT_REQ;
//			}
//			break;	
//		case MD_WAIT://模块上电等待
//			if(wait_timeout(LTE_POWER_ON_WAIT_TIME)) //等待2秒
//			{
//					state = MD_AT_REQ;
//			}
//			break;
//	


		case MD_RESET://复位模块，等待5秒
			if (module_reset())
			{
					wait_timeout(LTE_POWER_ON_WAIT_TIME); //复位等待//5s
					//if(debug_onoff.EC600N)   
						debug_printf("Reset-%d-!!!!!!!!!!!!!!!!!!!!!!\r\n",outlinetimes);
					outlinetimes++; 
					Gaspower_Command(GAS_SET_RSSI,1,Gaspower_index);
					state = MD_AT_REQ;
			}
			break;	
//		case MD_WAIT:
//			if(wait_timeout_nonblocking(5000))
//			{
//				if (fat_cmdres_keyword_matching("RDY\r\n"))
//					state = MD_AT_REQ;
//			}
//			break;
    case MD_AT_REQ://AT握手
        if (fat_send_wait_cmdres_blocking("AT\r\n", 500))
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
									debug_printf("握手失败");
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
						if(update_flag==1)
							state = MD_CONNETINIT_TCP;
						else 
							state = MD_CONNETINIT_MQTT;
					}
					else if (ret == MD_ERR)
					{
							state = MD_FLIGHTMODE;
					}
					break;
			//MQTT连接参数初始化
			case MD_CONNETINIT_MQTT:
					ret = mqtt_connect_parm_init();
					if (ret == MD_OK)
					{
							state = MD_CONNETED_MQTT;
					}
					else if (ret == MD_ERR)
					{
							state = MD_FLIGHTMODE;
					}
					break;
			//MQTT数据通信处理
			case MD_CONNETED_MQTT:
					ret = mqtt_data();
					if (ret == MD_OK)
					{
//							state = MD_FLIGHTMODE;
							state = MD_CONNETINIT_TCP;//OTA升级
//						state =MD_AT_REQ;
					}
					else if (ret == MD_ERR)
					{
							state = MD_FLIGHTMODE;
					}			 
					break;					
					
			//TCP连接参数初始化
			case MD_CONNETINIT_TCP:
					ret = tcp_connet_parm_init();
					if (ret == MD_OK)
					{
							state = MD_CONNETED_TCP;
					}
					else if (ret == MD_ERR)
					{
							state = MD_FLIGHTMODE;
					}
					break;		
			//TCP数据通信处理					
			case MD_CONNETED_TCP:
					ret = tcp_data();
					if (ret == MD_OK)
					{
						state = MD_CONNETINIT_MQTT;//mqtt连接
					}
					else if (ret == MD_ERR)
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

//uint8_t match_recv_data(char *cmdres)
//{
//    int length = 0;
//	  //unsigned char i;
//    char *p = NULL, *q = NULL;
//	
//	  memset(subs_keyword,0,50); //清零内存
//		memset(subs_dat,0,50); //清零内存
//	
//		//定位至应答的OK
//    p = strstr(cmdres, "}}");
//	
//    if (p == NULL)
//    {
//        return 0;
//    }
//		//截取数据
//		//p = p-1; //带引号的需要增加该行
//		q = p;
//		do
//		{
//        p--;
//    }
//    while (*p != 0x3A);//查询冒号：地址
//		
//    length = q - p - 1;	
//		
//		if(length>50) length = 50; //限制大小
//    memcpy(subs_dat, p + 1, length);//:到}之间
////		debug_printf("data:%s=",subs_dat);  //{"params":{"Summer_winterMode_Set":2}}
//		
//		//截取命令
//		p -= 1;
//		q = p;
//		do
//		{
//        p--;
//    }
//    while (*p != 0x22);// 查询 '"' 地址
//    length = q - p - 1;	
//		
//		if(length>50) length = 50; //限制大小
//    memcpy(subs_keyword, p + 1, length);//""之间
//		debug_printf("%s:%s\r",subs_dat,subs_keyword);
//    return 1;
//}

//void handle_MQTTConvert(void)
//{
//      unsigned int  dat,dat1;
//	    if(strcmp(subs_keyword,"PowerSwitch") ==0)   					//电源开关
//			{
//					if(subs_dat[0] == '1') //开
//					{
//						 Gaspower_Command(GAS_SET_ONOFF,1,Gaspower_index);
//						 
//					}
//					if(subs_dat[0]  == '0')
//					{
//						 Gaspower_Command(GAS_SET_ONOFF,0,Gaspower_index);
//						 
//				  }
//			}
//			
//			if(strcmp(subs_keyword,"Summer_winterMode_Set") ==0)   	//夏季冬季模式设定
//			{
//				  if(subs_dat[0] == '1') //1 夏季模式
//					{
//						 Gaspower_Command(GAS_SET_MODE,1,Gaspower_index);
//					}
//					if(subs_dat[0]  == '2') //2 冬季模式
//					{
//						 Gaspower_Command(GAS_SET_MODE,2,Gaspower_index);
//					}
//			}
//			if(strcmp(subs_keyword,"BathTemp_Set") ==0)   					//卫浴温度设定
//			{
//					dat = (subs_dat[0]-0x30)*10 + subs_dat[1]-0x30;
//				  if((dat>= 30)&&(dat<= 75))
//					{
//							Gaspower_Command(GAS_SET_BATHROOM_TEMP,dat,Gaspower_index);
//					}
//			}
//			if(strcmp(subs_keyword,"HeatingTemperature_Set") ==0)   //采暖温度设定
//			{
//					dat = (subs_dat[0]-0x30)*10 + subs_dat[1]-0x30;
//				  if((dat>= 30)&&(dat<= 80))
//					{
//							Gaspower_Command(GAS_SET_HEATINGTEMP,dat,Gaspower_index);
//					}
//			}
//			if(strcmp(subs_keyword,"DeviceReset") ==0)      	 			//远程复位
//			{
//					if(subs_dat[0] == '1') //开
//					{
//						 Gaspower_Command(GAS_SET_RST,1,Gaspower_index);
//					}
//			}
//			if(strcmp(subs_keyword,"Hot_water_Circ_ON") ==0)   		//热水循环开关设定
//			{
//			    if(subs_dat[0] == '1') //开
//					{
//						 Gaspower_Command(GAS_SET_HOT_WATER_CIRC,1,Gaspower_index);
//					}
//					if(subs_dat[0]  == '0')
//					{
//						 Gaspower_Command(GAS_SET_HOT_WATER_CIRC,0,Gaspower_index);
//				  }
//			}
//			if(strcmp(subs_keyword,"LockSwitch") ==0)     				//锁定
//			{
//					if(subs_dat[0] == '1') //开
//					{
//						 Gaspower_Command(GAS_SET_LOCK,1,Gaspower_index);
//					}
//					if(subs_dat[0]  == '0')
//					{
//						 Gaspower_Command(GAS_SET_LOCK,0,Gaspower_index);
//				  }
//			}
//			if(strcmp(subs_keyword,"ECO_Set") ==0)       					//ECO设置
//			{
//					if(subs_dat[0] == '1') //开
//					{
//						 Gaspower_Command(GAS_SET_ECO,1,Gaspower_index);
//					}
//					if(subs_dat[0]  == '0')
//					{
//						 Gaspower_Command(GAS_SET_ECO,0,Gaspower_index);
//				  }
//			}
//			if(strcmp(subs_keyword,"Realtime_Set") ==0)   				//时间校准
//			{
//			    dat = subs_dat[3]-0x30;
//				  dat *=1000;
//				  dat1 = subs_dat[4]-0x30;
//				  dat1 *=100;
//				  dat += dat1;
//				  01,34
//				  dat1 = subs_dat[0]-0x30;
//				  dat1 *=10;
//				  dat += dat1;
//				  dat += subs_dat[1]-0x30;
//				3401
//				  if(dat<5)
//					{
//						CostofGas = 2;  //清零燃气积算
//					  CostofWater = 2;
//						CostofElect = 2;
//					}
//						
//				  Gaspower_Command(GAS_SET_TIME_CALIB,dat,Gaspower_index);
//			}
//}

void handle_Fixed_Convert(Fixed_Device* fixDevice) { 
    if (strcmp(fixDevice->key, "PowerSwitch") == 0) {	//电源开关
				if(fixDevice->value==0 || fixDevice->value==1)
						Gaspower_Command(GAS_SET_ONOFF, fixDevice->value, Gaspower_index);
				
    } else if (strcmp(fixDevice->key, "Summer_winterMode_Set") == 0) {//夏季冬季模式设定
				if(fixDevice->value==1 || fixDevice->value==2)
						Gaspower_Command(GAS_SET_MODE, fixDevice->value, Gaspower_index);
				
    } else if (strcmp(fixDevice->key, "BathTemp_Set") == 0) {//卫浴温度设定
				if(fixDevice->value>=30 && fixDevice->value<= 75)
        Gaspower_Command(GAS_SET_BATHROOM_TEMP, fixDevice->value, Gaspower_index);
				
    } else if (strcmp(fixDevice->key, "HeatingTemperature_Set") == 0) {//采暖温度设定
				if(fixDevice->value>=30 && fixDevice->value<= 80)
        Gaspower_Command(GAS_SET_HEATINGTEMP, fixDevice->value, Gaspower_index);
				
    } else if (strcmp(fixDevice->key, "DeviceReset") == 0) {//远程复位
				if(fixDevice->value==1)
        Gaspower_Command(GAS_SET_RST, fixDevice->value, Gaspower_index);
				update_flag=1;
    } else if (strcmp(fixDevice->key, "Hot_water_Circ_ON") == 0) { //热水循环开关设定
				if(fixDevice->value==0 || fixDevice->value==1)
        Gaspower_Command(GAS_SET_HOT_WATER_CIRC, fixDevice->value, Gaspower_index);
			  
				
    } else if (strcmp(fixDevice->key, "LockSwitch") == 0) {//锁定
				if(fixDevice->value==0 || fixDevice->value==1)
        Gaspower_Command(GAS_SET_LOCK, fixDevice->value, Gaspower_index);
				
    } else if (strcmp(fixDevice->key, "ECO_Set") == 0) {//ECO设置
				if(fixDevice->value==0 || fixDevice->value==1)
        Gaspower_Command(GAS_SET_ECO, fixDevice->value, Gaspower_index);
				
		} 
		else if (strcmp(fixDevice->key, "Realtime_Set") == 0) {//时间校准3412→1234 12:34
//				fixDevice->value=fixDevice->value/100+fixDevice->value%100*100;
//					if(fixDevice->value < 5)  01,34  3401   分钟，小时
//					zero_value=fixDevice->value;
//						zero_cnt++;
					if(fixDevice->value < 5)
					{
						CostofGas = 0;  //清零燃气积算
					  CostofWater = 0;
						CostofElect = 0;
					}
        Gaspower_Command(GAS_SET_TIME_CALIB, fixDevice->value, Gaspower_index);
    } 
}

//时间校准
void handle_Fixed_Calib_Convert(void)
{ 
			uint16_t  dat,dat1;
			dat = calib_time_buf[3]-0x30;
			dat *=1000;
			dat1 = calib_time_buf[4]-0x30;
			dat1 *=100;
			dat += dat1;
		 // 01,34  3401
			dat1 = calib_time_buf[0]-0x30;
			dat1 *=10;
			dat += dat1;
			dat += calib_time_buf[1]-0x30;

			zero_value=dat;
			zero_cnt++;	
			if(dat<5)
			{
				CostofGas = 0;  //清零燃气积算
				CostofWater = 0;
				CostofElect = 0;
			}
				
			Gaspower_Command(GAS_SET_TIME_CALIB,dat,Gaspower_index);
}
