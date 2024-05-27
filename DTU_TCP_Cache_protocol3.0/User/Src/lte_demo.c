#include "lte_demo.h"

typedef enum
{
    MD_WAIT,        				//��λģ��
	  MD_RESET,        				//��λģ��
    MD_AT_REQ,       				//AT����
//	  MD_AT_E0,       				//�رջ���
    MD_WORK_STA_CHK, 				//����״̬���
    MD_CONNETINIT_MQTT,   	//����������Ϣ��ʼ��
	  MD_CONNETINIT_TCP,
    MD_CONNETED_MQTT,     	//����ͨ��
		MD_CONNETED_TCP,
		MD_FLIGHTMODE,			   	//����ģʽ
    MD_OK = 0xFE,    				//����
    MD_ERR = 0xFF,   				//�쳣
} MD_RUN_STATE;

#define LTE_POWER_ON_BEFOR_WAIT_TIME 2000 										//LTE�����ȴ�ʱ��
#define LTE_POWER_ON_WAIT_TIME 3000 										//LTE�����ȴ�ʱ��
#define SIGNALMIN 15                                    //�ź�������ͷ�ֵ
#define SIGNALMAX 31                                    //�ź�������ͷ�ֵ 

uint8_t ucStateNum = 0;                                 //����ִ��˳���ʶֵ
uint8_t state = MD_RESET;
uint16_t ucErrorTimes = 0;                               //��������ۼ�ֵ
uint8_t ucFlightModeTimes = 0;													//�������ģʽ����

 
//char subs_keyword[50]; //���յ�������
//char subs_dat[50];     //���յ����ַ�������


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description:��λLTEģ��
 * @param None
 * @return None
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//uint8_t module_reset(void)
//{
//    switch (ucStateNum)
//    {
//    //����RESET_N����
//    case 0://+= PEN_GPIO_SET_LOW;
//				HAL_GPIO_WritePin(GPIOB, EC600_RST_Pin, GPIO_PIN_SET);
////		    HAL_GPIO_WritePin(GPIOB, EC600_PWK_Pin, GPIO_PIN_SET);
//        ucStateNum++;
//        break;
//    case 1://����RESET_N����
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
//			//30ms�ȴ�VBAT��ѹ�ȶ�
//    case 0://+= PEN_GPIO_SET_LOW;
//		if(wait_timeout(50))
//			ucStateNum++;
//      break;			
//    //����PWRKEY����
//    case 1://+= PEN_GPIO_SET_LOW;
//				HAL_GPIO_WritePin(GPIOB, EC600_PWK_Pin, GPIO_PIN_SET);
//        ucStateNum++;
//        break;
//    case 2://����PWRKEY����
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
		//30ms�ȴ�VBAT��ѹ�ȶ�
    case 0://+= PEN_GPIO_SET_LOW;
		if(wait_timeout(50))
			ucStateNum++;
      break;		
    //����PWRKEY����
    case 1://+= PEN_GPIO_SET_LOW;
		    HAL_GPIO_WritePin(GPIOB, EC600_PWK_Pin, GPIO_PIN_SET);
        ucStateNum++;
        break;
    case 2://����PWRKEY����
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
		tmp[p - q] = '\0'; // ȷ���ַ����� null �ַ�����###
    lpCsqVal = atoi(tmp);//25
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


// ����AT��������Ӧ�ĺ���
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
    return ucStateNum; // ���ֵ�ǰ״̬
}

void Extract_QCCID(const char* buffer, char* qccidString) {
    // ���ҡ�+QCCID: ������Ӵ������ص�һ�γ��ֵ�λ��
    const char* ptr = strstr(buffer, "+QCCID: ");
    if (ptr) {
        // �ƶ�ָ�룬Խ��ǰ׺
        ptr += strlen("+QCCID: ");
        // ��QCCID�����ֲ��ָ��Ƶ�qccidString��
        while (*ptr != '\r' && *ptr != '\0') {
            *qccidString++ = *ptr++;
        }
        // �ַ�����β���NULL�ַ�
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
            // ���⴦���ź�����
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
												Extract_QCCID((char*)g_tModule.EC600_RevBuf,QCCID);//��ȡQCCID��ֵ
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
    return 0; // ���û����ɣ�����0�����ȴ�
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
// ����״̬ö��
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
						//����ǰ����Ʒ�������ִ�д������������
						if(strcmp(Device_Flash.mqttHostUrl, "a15dLSmBATk.iot-as-mqtt.cn-shanghai.aliyuncs.com") == 0) { 
								ucStateNum = send_at_and_check("AT+QMTCFG=\"ssl\",0,1,0\r\n", "OK", 1000, 10, STATE_MQTT_URCCFG, MD_ERR);
						} else {
								// If not matching, move to the next state directly
								ucStateNum++;
//								ucStateNum++;
						}
						break;
        case STATE_MQTT_URCCFG:
					//  ���ò���2 ����ģʽΪ  0:URC   1:�����ȡ
//						ucStateNum = send_at_and_check("AT+QMTCFG=\"recv/mode\",0,1,0\r\n", "OK", 1000, 10, STATE_MQTT_OPEN, MD_ERR);
						ucStateNum = send_at_and_check("AT+QMTCFG=\"recv/mode\",0,1,0\r\n", "OK", 1000, 10, STATE_MQTT_OPEN, MD_ERR);
            break;			
				//��MQTT�ͻ�������
        case STATE_MQTT_OPEN:
            // ����MQTT OPEN���� 
						snprintf(bufer, sizeof(bufer), "AT+QMTOPEN=0,\"%s\",%s\r\n", Device_Flash.mqttHostUrl, Device_Flash.port);
						ucStateNum = send_at_and_check(bufer, "+QMTOPEN: 0,0", 1000, 10, STATE_MQTT_CONN, MD_ERR);
            break;
				//��MQTT�ͻ������ӷ�����
        case STATE_MQTT_CONN:
            // ����MQTT CONNECT���� 
						snprintf(bufer, sizeof(bufer), "AT+QMTCONN=0,\"%s\",\"%s\",\"%s\"\r\n", 
							       Device_Flash.clientId, Device_Flash.username, Device_Flash.passwd);
						ucStateNum = send_at_and_check(bufer, "+QMTCONN: 0,0,0", 1000, 10, STATE_SUBSCRIBE_TOPIC2, MD_ERR); 
            break;
				//��������		
        case STATE_SUBSCRIBE_TOPIC2: 
						snprintf(bufer, sizeof(bufer), "AT+QMTSUB=0,1,\"%s\",0\r\n", Device_Flash.QMTSUB2); 
            ucStateNum = send_at_and_check(bufer, "+QMTSUB: 0,1,0,", 1000, 5, STATE_SUBSCRIBE_TOPIC5, MD_ERR); 
            break;  	
				//����OTA����		
        case STATE_SUBSCRIBE_TOPIC5: 
						snprintf(bufer, sizeof(bufer), "AT+QMTSUB=0,1,\"%s\",0\r\n", Device_Flash.QMTSUB5); 
            ucStateNum = send_at_and_check(bufer, "+QMTSUB: 0,1,0,", 1000, 5, STATE_SUBSCRIBE_TOPIC6, MD_ERR); 
            break;  
        case STATE_SUBSCRIBE_TOPIC6: 
						snprintf(bufer, sizeof(bufer), "AT+QMTSUB=0,1,\"%s\",0\r\n", Device_Flash.QMTSUB6); 
            ucStateNum = send_at_and_check(bufer, "+QMTSUB: 0,1,0,", 1000, 5, MD_OK, MD_ERR); 
            break;  				
				//���
        case MD_OK:
            ucStateNum = 0;
						Gaspower_Command(GAS_SET_RSSI,3,Gaspower_index);
            return MD_OK;
				//��ת������ģʽ
        case MD_ERR:
            ucStateNum = 0;
            return MD_ERR;
    }
    return 0; // ��������
}




// ����״̬��
#define VARIABLE_DEVICE_FOUND 1
#define FIXED_DEVICE_FOUND 2
#define CALIB_TIME_FOUND 3
void* device_malloc  = NULL;
char* extract_json(const char *input) {
    const char *start = strchr(input, '{');
    const char *end = strrchr(input, '}');//�ַ��������һ��'}'�ַ���λ��

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
			//����ļ�ֵ���ڲ�Ҳ�����������ʽ�洢�ģ���������Ԫ��һ��
        cJSON_ArrayForEach(current_element, params) {//��������ļ�ֵ��
////            Fixed_Device FixedDevice;
//						Fixed_Device *fixDevice = malloc(sizeof(Fixed_Device));
//						if(fixDevice==NULL)
//								return state;
            memset(fixDevice, 0, sizeof(Fixed_Device));
            if (cJSON_IsNumber(current_element)) {
                strncpy(fixDevice->key, current_element->string, sizeof(fixDevice->key) - 1);
							//sizeof(fixDevice->key) - 1:Ԥ���ռ���'\0's
                fixDevice->value = current_element->valueint;
								if(debug_onoff.EC600N)
										debug_printf("Key: %s, Value: %d\r\n", fixDevice->key, fixDevice->value); 
								state=1;
//								state=FIXED_DEVICE_FOUND;
								device_malloc=(void*)fixDevice;
						} else if (cJSON_IsString(current_element)) {
								// �ж��Ƿ�Ϊ "Realtime_Set" ��ֵ�� 
										strncpy(fixDevice->key, current_element->string, sizeof(fixDevice->key) - 1); // �����洢�� fixDevice->key ��
						 
										// ���ַ�������ȡ�� "1724" ���洢�� fixDevice->value ��
										fixDevice->value = extractNumber(current_element->valuestring, 8, 4); 
							
										// �� Realtime_Set �ַ����������洢���ṹ����
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
//    start = (char*)g_tModule.EC600_RevBuf;//��ȡjson���
//    while ((start = strstr(start, keyword)) != NULL) {
//			//��ȡ{}���
//        start += strlen(keyword); // �ƶ���"QMTRECV"֮��
//        if (*start == '{') {
//            end = strchr(start, '}'); // ����ƥ���'}'  ERROR
//            if (end != NULL) {
//                int length = end - start + 1; // �����ַ������ȣ�����{}
//								char *json_string = (char*)malloc(length + 1);
//								 if (json_string) {
//										strncpy(json_string, start, length);
//										json_string[length] = '\0';  
//										 result = json_to_command(json_string);
//											free(json_string);//�ͷŷ�����ڴ�
//								}   
//								 
//								
//								 //��ȡjson����еļ�ֵ
//								if (result == FIXED_DEVICE_FOUND) {
//									//�ڹ�¯�豸����ֵ�̶�
//											Fixed_Device *fixDevice = (Fixed_Device*)device_malloc;
//											handle_Fixed_Convert(fixDevice);//�ṹ��ת���ɿ�������
//											free(fixDevice); 
//								} else if (result == VARIABLE_DEVICE_FOUND ) {
//										//�ɱ��¿��豸 
//											Variable_Device *varDevice = (Variable_Device*)device_malloc; 
//				//												handle_Variable_Convert(); 											
//											free(varDevice);  
//								}  		
//						}
//				 }
//				start = end + 1; // �ƶ�����һ���ַ�����������
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
    // ����JSON�ַ���
    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL) {
        debug_printf("Error before: %s\n", cJSON_GetErrorPtr());
        return 1;			 
    }

    // ��ȡOTA��������ַ
    cJSON *otaServerAddress = cJSON_GetObjectItemCaseSensitive(json, "OTA_address");
    if (cJSON_IsString(otaServerAddress) && (otaServerAddress->valuestring != NULL)) {
        snprintf(Ota_Addr.OtaHostUrl, sizeof(Ota_Addr.OtaHostUrl), "%s", otaServerAddress->valuestring);
    }

    // ��ȡOTA�������˿ں�
    cJSON *serverPort = cJSON_GetObjectItemCaseSensitive(json, "OTA_port");
    if (cJSON_IsNumber(serverPort)) {
        Ota_Addr.OtaPort = (uint16_t)serverPort->valuedouble;
    }

    // ��ӡ�������֤�Ƿ���ȷ�����ʹ洢
		if(debug_onoff.EC600N){
			debug_printf("OTA Host URL: %s\n", Ota_Addr.OtaHostUrl);
			debug_printf("OTA Port: %u\n", Ota_Addr.OtaPort);
		}
    // �ͷ�cJSON����ռ�õ��ڴ�
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
        start += strlen(keyword); // �ƶ����ؼ��ֺ�����ֿ�ʼ��
        char *end = strchr(start, '}'); // �������ֺ��������'}'
        if (end != NULL ) {
            // �������ֵ�result�����У�ע�Ᵽ������
            strncpy(calib_time_buf, start, end - start);
//            calib_time_buf[end - start] = '\0'; // ȷ���ַ�����'\0'��β
						zero_cnt++;
						return CALIB_TIME_FOUND; 
        }
    } 
		return 0;
}

 

void handle_subid(const char *sub_id,uint8_t *sub_id_array) 
{ 
    // ��� sub_id �����Ƿ���ȷ
    if (strlen(sub_id) != 8) {
        return;
    }

//    // ����һ�� uint8_t �������ڴ洢ת�����ֵ
//    uint8_t sub_id_array[4];

    // ת�� sub_id �ַ���Ϊ uint8_t ����
    for (int i = 0; i < 4; i++) {
        char byte_str[3] = {sub_id[2*i], sub_id[2*i + 1], '\0'};
        sub_id_array[i] = (uint8_t)strtol(byte_str, NULL, 16);
    } 
}
 
 
 
// ������ʵ��
void handle_control(const char *sub_id, double temp_set, int sw, int mode) {
			// ����һ�� uint8_t �������ڴ洢ת�����ֵ
		uint8_t sub_id_array[4];
		handle_subid(sub_id,sub_id_array); 
	
		DeviceData device_data;
		device_data.temp_set=(uint16_t)(temp_set*10);
		device_data.mode=mode;
		device_data.sw=sw;
		// ��ջ
		construct_send_command_to_fifo(0x06,sub_id_array,&device_data); 
}

 
// ����������
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
		
		 //switch(action) ���������н��� 
    if (strcmp(action, "networkIn") == 0) {
        construct_send_command_to_fifo(0x01,NULL,NULL);//����
    } else if (strcmp(action, "networkOut") == 0) {
        construct_send_command_to_fifo(0x02,NULL,NULL);//����
    } else if (strcmp(action, "delete") == 0) {
        if (params != NULL) {
            cJSON *sub_id_item = cJSON_GetObjectItem(params, "sub_id");
            if (sub_id_item != NULL) {
							    // ����һ�� uint8_t �������ڴ洢ת�����ֵ
								uint8_t sub_id_array[4];
                handle_subid(sub_id_item->valuestring,sub_id_array); 
								// ��ջ
								construct_send_command_to_fifo(0x04,sub_id_array,NULL);
            }
        }
    }  else if (strcmp(action, "status") == 0) {
        if (params != NULL) { 
							// ��ջ
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
						//�����ϱ�ʱ��
						
        }
    } else if (strcmp(action, "idlist") == 0) {
					// ��ջ
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

#define MAX_MESSAGES 5 // �ܵ������Ϣ��Ϊ5
uint8_t	temper_rapif_reply_flag=0;
uint8_t	temper_rapif_reply_cnt=0;
void process_messages(char *messages) {
//    char *msg_copy = strdup(messages); // ������Ϣ�ĸ���
    char *line;
    char *saveptr;
    int index = 0; // ����׷���ܵ��Ѵ�����Ϣ��

    line = strtok_r(messages, "\r\n", &saveptr); // �ָ��ַ�������\r�滻��\0
    while (line != NULL && index < MAX_MESSAGES) {
        if (strstr(line, "property/post_reply") != NULL) {//�ڹ�¯����
							const char *json_start = strchr(line, '{');
							if (json_start != NULL) { 
//									//��ȡjson����еļ�ֵ 
//										uint8_t result = extract_realtime_set(json_start);//ʱ��У׼����
//										if(result==0)
									uint8_t result;
									Fixed_Device fixDevice;
									result=parse_fixed_json(json_start,&fixDevice);//cjson���� 
									 
									if (result == 1) {
										//�ڹ�¯�豸����ֵ�̶�
//												Fixed_Device *fixDevice = (Fixed_Device*)device_malloc;
												handle_Fixed_Convert(&fixDevice);//�ṹ��ת���ɿ�������
//												free(fixDevice); 
									} 
//									else if(result == CALIB_TIME_FOUND ){
//												handle_Fixed_Calib_Convert();
//									}else if (result == VARIABLE_DEVICE_FOUND ) {
//											//�ɱ��¿��豸 
//												Variable_Device *varDevice = (Variable_Device*)device_malloc; 
//	//											handle_Variable_Convert(); 											
//												free(varDevice);  
//									}  		 
							}
        } else if(strstr(line, "tempercontrol/distribution") != NULL){//�¿�������
							//�ϱ�ʱ������
							temper_rapif_reply_flag=1;
							temper_rapif_reply_cnt=120;//120;//2����û�н��յ����ֹͣ�����ϱ�
							const char *json_start = strchr(line, '{');
							if (json_start != NULL) {  
									uint8_t result; 
									 parse_variable_json(json_start);//cjson����  
							}			 
				}else if (strstr(line, "/ota/device/upgrade") != NULL ) {//ota��������
            // ����OTA��Ϣ
            const char *json_start = strchr(line, '{');
            if (json_start != NULL) {
								if(ota_json_to_serverip(json_start)==0)//����ota��ַ�Ͷ˿ں�
									update_flag=1;//׼��OTA������־
									//OTA�洢��ַ
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

//    free(msg_copy); // �ͷŸ���
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @description: �����շ�����
 * @return 0�����δ��ɣ�MD_ERR������
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//�������̱�ʶ
typedef enum
{
//    SUB_MESG_GET = 0,
	  SUB_SEND1=0,        				//����topic����
	  SUB_SEND2,        				   //����message���� 
		SUB_GET,										//�������� 
    SUB_SOCKET,       				 //��ѯSocket 
  	SUB_DISC,
//		SUB_CLOSE,                  //�ر�����
	  SUB_WAIT    								//����������Ϣ��ʼ�� 
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
								//�յ�>
								if (fat_cmdres_keyword_matching(">"))
								{
										ucErrorTimes = 0;
										ucStateNum++;
								}
								else
								{
										//����3�εò�����ȷӦ��
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
						//�յ�OK
            if (fat_cmdres_keyword_matching("OK"))
            {
//							debug_printf("uart recv OK: %s\r\n",g_tModule.EC600_RevBuf);
								if(topic_send_wait!=15)
									topic_send_last = topic_send_wait;//��¼��ǰ����ֱ��
								topic_send_wait = 0;
								topic_send_ok = 1; //������ɱ�ʶ
								topic_count++;		
							
//								debug_printf("����<OK>\r");
                ucErrorTimes = 0;
                ucStateNum=SUB_SOCKET;
//								ucStateNum = SUB_SEND1;
								
            }
            else
            {
//							debug_printf("uart recv ERROR: %s\r\n",g_tModule.EC600_RevBuf);
								//����3�εò�����ȷӦ��
                if (ucErrorTimes++ > 3)
                {
                    ucStateNum = MD_ERR;
                }
            }					
        }
        break;
		
 
    //��ѯsocket�����Ƿ�������
    case SUB_GET:
//			  if (fat_send_wait_cmdres_blocking("AT+QMTCFG=\"recv/mode\",0\r\n", 1000))		
//				if (fat_send_wait_cmdres_blocking("AT+QMTRECV?\r", 1000))
//        if (fat_send_wait_cmdres_blocking("AT+QMTRECV?\r\n", 1000))	
//				if (fat_send_wait_cmdres_blocking("AT+QMTRECV=0\r\n", 1000))
//        {
//					debug_printf("uart recv OK: %s\r\n",g_tModule.EC600_RevBuf);
//						//�յ�+QMTRECV:
////				  fat_cmdres_keyword_matching("+QMTRECV:"); 
//            if (fat_cmdres_keyword_matching("post_reply"))
//            { 
//								//�յ�1��ʾ�����ݡ����صĲ���1ΪclientID����5����Ϊ<store_status>
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
//								//����3�εò�����ȷӦ��
//                if (ucErrorTimes++ > 3)
//                {
//                    ucStateNum = MD_ERR;
//                }
//            }
//        }		
				if (fat_send_wait_cmdres_blocking("AT+QMTRECV=0\r\n", 1000))
        {
					debug_printf("uart recv OK: %s\r\n",g_tModule.EC600_RevBuf);
						//�յ�+QMTRECV:
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
//								//����3�εò�����ȷӦ��
//                if (ucErrorTimes++ > 3)
//                {
//                    ucStateNum = MD_ERR;
//                }
            }
        }
        break;
		//��ȡTopic��Data
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
//								char *json_string = extract_json((char*)g_tModule.EC600_RevBuf);//��ȡjson���
//								if (json_string) {
//									uint8_t result = json_to_command(json_string);
//										free(json_string);//�ͷŷ�����ڴ�
//										if (result == FIXED_DEVICE_FOUND) {
//											//�ڹ�¯�豸����ֵ�̶�
//													Fixed_Device *fixDevice = (Fixed_Device*)device_malloc;
// 													handle_Fixed_Convert(fixDevice);//�ṹ��ת���ɿ�������
//													free(fixDevice); 
//										} else if (result == VARIABLE_DEVICE_FOUND ) {
//												//�ɱ��¿��豸 
//											    Variable_Device *varDevice = (Variable_Device*)device_malloc; 
////												handle_Variable_Convert(); 											
//													free(varDevice);  
//										}   
//								} 				 
//								ucErrorTimes = 0;
////								ucStateNum = SUB_GET;
////								ucStateNum = SUB_SOCKET; //û�н��յ�����鿴�Ƿ���Ҫ����
//						} 
//						if(topic_send_wait==1) 
//								ucStateNum=SUB_SEND1;
//								//����3�εò�����ȷӦ��
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
//						if(fat_cmdres_keyword_matching("property/post_reply"))		//�豸����		
//						{						
//								char *json_string = extract_json((char*)g_tModule.EC600_RevBuf);//��ȡjson���
//								if (json_string) {
//									uint8_t result = json_to_command(json_string);
//										free(json_string);//�ͷŷ�����ڴ�
//										if (result == FIXED_DEVICE_FOUND) {
//											//�ڹ�¯�豸����ֵ�̶�
//													Fixed_Device *fixDevice = (Fixed_Device*)device_malloc;
// 													handle_Fixed_Convert(fixDevice);//�ṹ��ת���ɿ�������
//													free(fixDevice); 
//										} else if (result == VARIABLE_DEVICE_FOUND ) {
//												//�ɱ��¿��豸 
//											    Variable_Device *varDevice = (Variable_Device*)device_malloc; 
////												handle_Variable_Convert(); 											
//													free(varDevice);  
//										}   
//								} 				 

//						}
//						else if(fat_cmdres_keyword_matching("/ota/device/upgrade"))//ota����
//						{
//							//��ȡcjson���
//							char *json_string = extract_json((char*)g_tModule.EC600_RevBuf);//��ȡjson���
//							//cjson�е�OTA��ַ���˿ں�
//							if (json_string) {
//									ota_json_to_serverip(json_string); //����OTA��ַ�Ͷ˿ں�
//									free(json_string);//�ͷŷ�����ڴ� 
//							} 								
//							update_flag=1;
//						}
						
						ucErrorTimes = 0;
						//update_flagд��flash
						if(update_flag==1){		
							ucStateNum = SUB_DISC; //�Ͽ�����
						}else if(topic_send_wait>0) 
							ucStateNum = SUB_SEND1; //û�н��յ�����鿴�Ƿ���Ҫ����
						else
							ucStateNum = SUB_WAIT;

				}
				break;


		case SUB_WAIT:				
				//�յ�OK
		    if(wait_timeout(300))
				{ 
					 //ucStateNum = SUB_SEND1;
					ucStateNum = SUB_SOCKET;
				}
        break; 
		 
				// ����MQTT DISCONNECT����  
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
				// ����MQTT DISCONNECT����  
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
		//������������ģʽ
    case MD_ERR:
				ucStateNum = 0;
		    if(debug_onoff.EC600N)  debug_printf("����&&\r"); 
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
 * @description: Socket����������ó�ʼ��
 * @param None
 * @return 0�����δ��ɣ�MD_OK��ģ���Ѿ�����MD_ERR�����󣬲����㹤��״̬
 */ 
enum TcpConnectionState { STATE_APN_CONFIG, STATE_ACTIVATE_APN,STATE_DEACTIVATE_APN, STATE_TCP_CONNECT,STATE_ERROR };
  
uint8_t tcp_connet_parm_init(void) {
    char buffer[255];
    switch (ucStateNum) {
        case STATE_APN_CONFIG://���ó���1��APN ����Ϊ"CMNET"
            ucStateNum = send_at_and_check("AT+QICSGP=1,1,\"CMNET\",\"\",\"\",1\r\n", "OK", 1000, 10, STATE_DEACTIVATE_APN, MD_ERR);
            break;

				case STATE_DEACTIVATE_APN://�������1
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
						
        case STATE_ACTIVATE_APN://�����1
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
//											ucStateNum = STATE_DEACTIVATE_APN;//������
											ucStateNum = STATE_TCP_CONNECT;
									}
								}					
						}				
            break;

//        case STATE_ERROR://�����ѯ
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
//											ucStateNum = MD_ERR;//������
//									}
//								}					
//						}				
//            break;				
//        case STATE_ERROR://�����ѯ
//						if (fat_send_wait_cmdres_blocking("AT+QISTATE=0,1\r\n", 1000)) {
//								if (fat_cmdres_keyword_matching("OK")) { 
//										ucErrorTimes = 0;
////										ucStateNum = MD_ERR;
//								}
//								else
//								{ 
//									if (ucErrorTimes++ > 3) {
//											ucErrorTimes = 0;
//											ucStateNum = MD_ERR;//������
//									}
//								}					
//						}				
//            break;							
        case STATE_TCP_CONNECT://�������� ͸��ģʽ
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
										//����5�εò�����ȷӦ��
										if (ucErrorTimes++ > 5)
										{ 
												if(++tcp_connect_cnt>3){
													update_flag=0;
													tcp_connect_cnt=0;
												}
												ucErrorTimes = 0;
//												ucStateNum = STATE_DEACTIVATE_APN;//������
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

uint16_t update_idex=0;//���������
uint16_t packege_sum=0;//�������ܰ���
uint32_t flash_page_addr; // Flashд�����ʼ��ַ
uint8_t flash_buffer[PAGE_SIZE]; // ���ݻ���
uint16_t flash_buffer_index; // �����е�ǰ���ݵ�����
int save_ota_file(const uint8_t* data, uint16_t size) {
	  uint8_t  save_flag=0;
		uint16_t save_size;
		uint16_t flash_buffer_idex_back=flash_buffer_index;//�洢ʧ��ʱ�ָ�
//		static uint8_t err=0;
    while (size > 0) {
        uint16_t space_left = PAGE_SIZE - flash_buffer_index; // ���㻺��ʣ��ռ� 
        if (size <= space_left) {
            // ���ʣ��ռ��㹻��ֱ�Ӹ������ݵ�����
            memcpy(&flash_buffer[flash_buffer_index], data, size);
            flash_buffer_index += size;
            size = 0; // �Ѵ��������н��յ�����
						if(update_idex==(packege_sum-1)){	//���յ����һ��
								save_flag=3; 
						}
        } else {
            // ���ʣ��ռ䲻�㣬���������棬Ȼ����д��Flash
            memcpy(&flash_buffer[flash_buffer_index], data, space_left);
            flash_buffer_index += space_left;
            data += space_left;
            size -= space_left;
						save_flag=1; 
						if(update_idex==(packege_sum-1)){	//���յ����һ��
								save_flag=2; 
						}
				}
				
				if(save_flag>0){ 
							// ʹ��flush_buffer_to_flash��������д��Flash
							save_size=(save_flag<3?PAGE_SIZE:flash_buffer_index);//�洢��С 
							int write_status = flush_buffer_to_flash(flash_page_addr, flash_buffer, save_size);
//							if(update_idex==8 && err<3){
//								write_status=1;
//								err++; 
//							}					
							if (write_status == HAL_OK) {
									if(debug_onoff.EC600N)
										debug_printf("�洢��ַ:%d�������%d\r\n",flash_page_addr,update_idex);
									flash_page_addr += PAGE_SIZE; // �ɹ�д�룬����Flash��ַ
									flash_buffer_index = 0; // ���û������� 
							} else {
									// ���д��ʧ�ܣ�����ʧ���߼���������Ҫ��¼���󡢷��;�����
									debug_printf("flash�洢ʧ��");
									flash_buffer_index=flash_buffer_idex_back;//�ָ�buffer�洢���
									return write_status; // ����ʵ�����������Ҫ���ش�����
							}
							
							
							// ��ʣ�����ݸ��Ƶ�����Ŀ�ͷ
							if(size>0){
								memcpy(flash_buffer, data, size);
								flash_buffer_index = size;
								if(save_flag==2){//�մ洢��1ҳ���ý��յ����һ��
									write_status = flush_buffer_to_flash(flash_page_addr, flash_buffer, flash_buffer_index);	
									if(write_status!=0) {
										flash_buffer_index-=size;//���һ������û�п�ҳ�����⴦��
										return write_status;	
									}										
								}									
								size=0;//©��
							}
							save_flag=0;
				}
       
    }
		return 0;
} 
// ����״̬ö��
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


// ����ķ��ͺͽ��պ���ԭ�� 
#define FILE_Header 0x3AA3
#define FILE_TRANSMIT 0xA55A  
#define FILE_END 0xA569
#define FILE_ERROR 0xA56A   
#define FILE_TIMEOUT 0xA56B   
#define FILE_INFO 	0xA53B   
#define FILE_REV_SIZE 12
//#define FILE_REV_TIMEOUT 0xA57A

// ʹ��ö����������궨������ʾ����ֵ���������
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
    uint16_t result; // ���ݲ��ֵ���󳤶�
    uint16_t crc;
} RevProtocol;
void construct_revmessage( uint16_t cmd, uint16_t index, uint16_t result,void (*send_func)(uint8_t*, uint16_t))  
{
    RevProtocol msg;
    msg.header = FILE_Header;
    msg.cmd = cmd;
    msg.index = index;
    msg.result = result; 
    msg.length = sizeof(RevProtocol); // ��������������ṹ��Ĵ�С
    msg.crc = CRC16((uint8_t*)&msg, msg.length - 2); // ����CRC������CRC�ֶα���

    // ʹ���ṩ�ķ��ͺ���������Ϣ
    send_func((uint8_t*)&msg, msg.length);
		if(debug_onoff.EC600N)
			SendDataAsHex((uint8_t*)&msg,msg.length);
}

uint8_t verify_message(uint8_t* buffer, uint16_t bufLen) {
    // �����Ϣͷ
    uint16_t header = (buffer[INDEX_HEAD] & 0x00FF) + ((buffer[INDEX_HEAD+1] << 8) & 0xFF00);
    if (header != FILE_Header) return 0;

    // ��鳤���Ƿ�ƥ��
    uint16_t length = (buffer[INDEX_LEN] & 0x00FF) + ((buffer[INDEX_LEN+1] << 8) & 0xFF00);
    if (length != bufLen) return 0;

    // ���CRC
    if (CRC16(buffer, bufLen) != 0) return 0;

    // ���м��ͨ��
    return 1;
}

uint8_t ote_transport_protocol(void)
{
	uint8_t ret=DATA_RESTART;
	uint16_t  temp;
	static uint8_t err=0,step=0;
	/* ��������Ӧ������Ӧ�����в���ƥ����Ŀ */
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
					return DATA_SEND_END;//�ļ��������
			}		
			else
			{
					// ʹ�ü򻯵���֤����
					if (!verify_message(g_tModule.EC600_RevBuf, g_tModule.EC600_BufLen)) {
							ret=DATA_RESTART; // ��֤ʧ��
					}
					else{ 
							// ����CMD������Ϣ
							uint16_t cmd = (g_tModule.EC600_RevBuf[INDEX_CMD] & 0x00FF) + ((g_tModule.EC600_RevBuf[INDEX_CMD+1] << 8) & 0xFF00);						
							switch (cmd) {
									case FILE_TIMEOUT:
									case FILE_ERROR:
											err=0;
											step=0;
											return DATA_SERVER_END;
									default:
									 ///////////////�����idex/������sum
									temp=((g_tModule.EC600_RevBuf[INDEX_IDEX]&0x00FF)+((g_tModule.EC600_RevBuf[INDEX_IDEX+1]<<8)&0xFF00));	
									if(cmd==FILE_INFO) {
												packege_sum=temp;//�洢���ļ���
//											flash_mass_erase(OTA_store_addr,48*1024);//�������  
												construct_revmessage(FILE_INFO,update_idex,stringToUint16(Device_Flash.iotCode),EC600_Usart_Send); 
												step=1;
												return DATA_OK;
									} 										
									else if(cmd==FILE_TRANSMIT){ 
											if(temp==update_idex){
												 ///////////////FLASH:��ҳ�߲��ߴ�
												 if(save_ota_file(g_tModule.EC600_RevBuf+8, g_tModule.EC600_BufLen-10)==0)
													//ȫ��������ֱ�Ӵ洢
//											 if(flash_write(flash_page_addr+256*update_idex,g_tModule.EC600_RevBuf+8,g_tModule.EC600_BufLen-10)==(-1))
												 {
													 construct_revmessage(FILE_TRANSMIT,update_idex,0,EC600_Usart_Send);  
//													 update_idex++;   
													 err=0;
													 return DATA_OK;
												 }
											 //��Ӧ������
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
			//���ʹ���ָ��÷����������Ͽ�����
				EC600_Usart_Send((uint8_t*)file_end_rev,sizeof(file_end_rev)); 
				if(debug_onoff.EC600N)
					SendDataAsHex((uint8_t*)file_end_rev,0x0A);		
				err=0; 
				step=0;
			return DATA_CLIENT_DISCONNECT;
		}		
		//step=1ʱ ���մ��ڲ���ȷ/δ���ܵ�/�洢���� �ط�ָ��
			construct_revmessage(FILE_INFO,update_idex,1,EC600_Usart_Send);  
			return ret;
	}
 
} 

/**
 * @description: TCP�����շ�����
 * @return 0�����δ��ɣ�MD_ERR�����󣬲����㹤��״̬
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
							if(first_flag==1)//��ʼ������
							{
								flash_buffer_index=0;//flash�������
								flash_page_addr=OTA_store_addr; 
								update_idex=0;//�������������		
								first_flag=0;
							}								
							ucStateNum=RECEIVE_FILE;
						}
						else
						{
							ucStateNum=DISCONNECT;
//							if( ret==DATA_SEND_END)//����Ҫ����
//							{
//								update_flag=0;  
//							}
						}
					} 
					break;				
        case RECEIVE_FILE:
					// ʵ�ֽ����ļ��߼�
						if(wait_timeout_nonblocking(5000))
            { 
								ret=ote_transport_protocol();
								if (ret==DATA_OK){ //��������
										update_idex++;  
//									ucStateNum = RECEIVE_BREAK;
								}else if( ret==DATA_SEND_END){//�����ɹ�
									//�������ݽ���
											ucStateNum=DISCONNECT;
											update_flag=0; 
											if(update_idex==packege_sum)
											{
												bk_flash_WriteOnePage_uint8(UPGRADEaddr, (uint8_t*)&OTA_store_addr, 4);
												debug_printf("����ɹ�\r\n");
												HAL_Delay(300);
												NVIC_SystemReset(); 
											}
								}else if(ret==DATA_SERVER_END){//�����������Ͽ�
											ucStateNum = DISCONNECT;											
								}else if(ret==DATA_CLIENT_DISCONNECT){//�ͻ��������Ͽ�
											ucStateNum = DISCONNECT;									
								}else if(ret==DATA_RESTART){//������
										ucStateNum = RECEIVE_FILE;
//									update_idex=0;								
								}
						}	 
            break; 
//        case RECEIVE_BREAK:
//						if(wait_timeout(200)==1)
//							ucStateNum = RECEIVE_FILE;	
//            break; 						
				//�˳�͸��ģʽ
//				case MD_ERR:				
//						if (fat_send_wait_cmdres_nonblocking("+++", 1000))
//						{
//								//�յ�OK
//								if (fat_cmdres_keyword_matching("OK"))
//								{
//		//                FAT_DBGPRT_INFO("uart recv: %s\r\n", cFatUartRecvBuf);
//										ucErrorTimes = 0;
//										ucStateNum = DISCONNECT;
//								}
//								else
//								{
//										//����5�εò�����ȷӦ��
//										if (ucErrorTimes++ > 5)
//										{
//												ucStateNum = MD_ERR;
//										}
//								}
//						}
//						break;   
				case DISCONNECT:
						// �Ͽ����Ӵ���
						if (fat_send_wait_cmdres_blocking("AT+QICLOSE=0\r\n", 500))
						{
								//�յ�OK
								if (fat_cmdres_keyword_matching("OK"))
								{ 
										ucErrorTimes = 0; 
									  if(update_flag==1)//��������
										{
											state=MD_CONNETINIT_TCP;
											ucStateNum =STATE_TCP_CONNECT; 
										}
										else//����mqtt
										{ 
											ucStateNum =MD_OK; 										
										}
								}
								else
								{
										//����5�εò�����ȷӦ��
										if (ucErrorTimes++ > 5)
										{ 
												ucErrorTimes=0;
												ucStateNum = MD_ERR;
										}
								}
						}				 
						break; 
				case RECONNECT:
						if(update_flag==0){//�������
								debug_printf("����ɹ�\r\n");
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
 * @description: ����ģʽ������
 * @param None
 * @return 0�����δ��ɣ�MD_WORK_STA_CHK�����¿�����������ģ��״̬��⣻MD_ERR������
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
uint8_t module_flightmode(void) {
    switch (ucStateNum) {
        case 0x00:
            ucFlightModeTimes++;
            if(debug_onoff.EC600N) debug_printf("�������ģʽ������%d\r\n",ucFlightModeTimes);
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
								if(debug_onoff.EC600N) debug_printf("�ٿ�������\r\n");										 
								ucStateNum = 0;
								return MD_WORK_STA_CHK;  
            }
            break;
        case MD_ERR:
						if (ucFlightModeTimes == 2){//�Ѿ����ν������ģʽ����λ
								if(debug_onoff.EC600N)  debug_printf("���η���ģʽ����λģ��\r\n");
						}					
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
//uint32_t outlinetimes = 0;//������������
void  Handle_Lte(void)
{
//    static uint8_t state = MD_RESET;
    uint8_t ret = 0;
    switch (state)
    {
//			case MD_WAIT://ģ���ϵ�ȴ� 
//				if(module_power_on()) //����
//				{ 
//						wait_timeout(LTE_POWER_ON_WAIT_TIME); //��λ�ȴ�//5s 
//						state = MD_AT_REQ;
//				}
//				break;
//			case MD_RESET://��λģ�飬�ȴ�5��
//        if (module_reset())
//        {
//						wait_timeout(LTE_POWER_ON_WAIT_TIME); //��λ�ȴ�//5s
//					  //if(debug_onoff.EC600N)   
//							debug_printf("Reset-%d-!!!!!!!!!!!!!!!!!!!!!!\r\n",++outlinetimes); 
//					  state = MD_AT_REQ;
//					  Gaspower_Command(GAS_SET_RSSI,1,Gaspower_index);
//        }
//        break;
			
			

//		case MD_RESET://��λģ�飬�ȴ�5��
//			if (module_reset())
//			{
////					wait_timeout(LTE_POWER_ON_WAIT_TIME); //��λ�ȴ�//5s
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
//		case MD_WAIT://ģ���ϵ�ȴ�
//			if(wait_timeout(LTE_POWER_ON_WAIT_TIME)) //�ȴ�2��
//			{
//					state = MD_AT_REQ;
//			}
//			break;
//	


		case MD_RESET://��λģ�飬�ȴ�5��
			if (module_reset())
			{
					wait_timeout(LTE_POWER_ON_WAIT_TIME); //��λ�ȴ�//5s
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
    case MD_AT_REQ://AT����
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
									debug_printf("����ʧ��");
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
			//MQTT���Ӳ�����ʼ��
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
			//MQTT����ͨ�Ŵ���
			case MD_CONNETED_MQTT:
					ret = mqtt_data();
					if (ret == MD_OK)
					{
//							state = MD_FLIGHTMODE;
							state = MD_CONNETINIT_TCP;//OTA����
//						state =MD_AT_REQ;
					}
					else if (ret == MD_ERR)
					{
							state = MD_FLIGHTMODE;
					}			 
					break;					
					
			//TCP���Ӳ�����ʼ��
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
			//TCP����ͨ�Ŵ���					
			case MD_CONNETED_TCP:
					ret = tcp_data();
					if (ret == MD_OK)
					{
						state = MD_CONNETINIT_MQTT;//mqtt����
					}
					else if (ret == MD_ERR)
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

//uint8_t match_recv_data(char *cmdres)
//{
//    int length = 0;
//	  //unsigned char i;
//    char *p = NULL, *q = NULL;
//	
//	  memset(subs_keyword,0,50); //�����ڴ�
//		memset(subs_dat,0,50); //�����ڴ�
//	
//		//��λ��Ӧ���OK
//    p = strstr(cmdres, "}}");
//	
//    if (p == NULL)
//    {
//        return 0;
//    }
//		//��ȡ����
//		//p = p-1; //�����ŵ���Ҫ���Ӹ���
//		q = p;
//		do
//		{
//        p--;
//    }
//    while (*p != 0x3A);//��ѯð�ţ���ַ
//		
//    length = q - p - 1;	
//		
//		if(length>50) length = 50; //���ƴ�С
//    memcpy(subs_dat, p + 1, length);//:��}֮��
////		debug_printf("data:%s=",subs_dat);  //{"params":{"Summer_winterMode_Set":2}}
//		
//		//��ȡ����
//		p -= 1;
//		q = p;
//		do
//		{
//        p--;
//    }
//    while (*p != 0x22);// ��ѯ '"' ��ַ
//    length = q - p - 1;	
//		
//		if(length>50) length = 50; //���ƴ�С
//    memcpy(subs_keyword, p + 1, length);//""֮��
//		debug_printf("%s:%s\r",subs_dat,subs_keyword);
//    return 1;
//}

//void handle_MQTTConvert(void)
//{
//      unsigned int  dat,dat1;
//	    if(strcmp(subs_keyword,"PowerSwitch") ==0)   					//��Դ����
//			{
//					if(subs_dat[0] == '1') //��
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
//			if(strcmp(subs_keyword,"Summer_winterMode_Set") ==0)   	//�ļ�����ģʽ�趨
//			{
//				  if(subs_dat[0] == '1') //1 �ļ�ģʽ
//					{
//						 Gaspower_Command(GAS_SET_MODE,1,Gaspower_index);
//					}
//					if(subs_dat[0]  == '2') //2 ����ģʽ
//					{
//						 Gaspower_Command(GAS_SET_MODE,2,Gaspower_index);
//					}
//			}
//			if(strcmp(subs_keyword,"BathTemp_Set") ==0)   					//��ԡ�¶��趨
//			{
//					dat = (subs_dat[0]-0x30)*10 + subs_dat[1]-0x30;
//				  if((dat>= 30)&&(dat<= 75))
//					{
//							Gaspower_Command(GAS_SET_BATHROOM_TEMP,dat,Gaspower_index);
//					}
//			}
//			if(strcmp(subs_keyword,"HeatingTemperature_Set") ==0)   //��ů�¶��趨
//			{
//					dat = (subs_dat[0]-0x30)*10 + subs_dat[1]-0x30;
//				  if((dat>= 30)&&(dat<= 80))
//					{
//							Gaspower_Command(GAS_SET_HEATINGTEMP,dat,Gaspower_index);
//					}
//			}
//			if(strcmp(subs_keyword,"DeviceReset") ==0)      	 			//Զ�̸�λ
//			{
//					if(subs_dat[0] == '1') //��
//					{
//						 Gaspower_Command(GAS_SET_RST,1,Gaspower_index);
//					}
//			}
//			if(strcmp(subs_keyword,"Hot_water_Circ_ON") ==0)   		//��ˮѭ�������趨
//			{
//			    if(subs_dat[0] == '1') //��
//					{
//						 Gaspower_Command(GAS_SET_HOT_WATER_CIRC,1,Gaspower_index);
//					}
//					if(subs_dat[0]  == '0')
//					{
//						 Gaspower_Command(GAS_SET_HOT_WATER_CIRC,0,Gaspower_index);
//				  }
//			}
//			if(strcmp(subs_keyword,"LockSwitch") ==0)     				//����
//			{
//					if(subs_dat[0] == '1') //��
//					{
//						 Gaspower_Command(GAS_SET_LOCK,1,Gaspower_index);
//					}
//					if(subs_dat[0]  == '0')
//					{
//						 Gaspower_Command(GAS_SET_LOCK,0,Gaspower_index);
//				  }
//			}
//			if(strcmp(subs_keyword,"ECO_Set") ==0)       					//ECO����
//			{
//					if(subs_dat[0] == '1') //��
//					{
//						 Gaspower_Command(GAS_SET_ECO,1,Gaspower_index);
//					}
//					if(subs_dat[0]  == '0')
//					{
//						 Gaspower_Command(GAS_SET_ECO,0,Gaspower_index);
//				  }
//			}
//			if(strcmp(subs_keyword,"Realtime_Set") ==0)   				//ʱ��У׼
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
//						CostofGas = 2;  //����ȼ������
//					  CostofWater = 2;
//						CostofElect = 2;
//					}
//						
//				  Gaspower_Command(GAS_SET_TIME_CALIB,dat,Gaspower_index);
//			}
//}

void handle_Fixed_Convert(Fixed_Device* fixDevice) { 
    if (strcmp(fixDevice->key, "PowerSwitch") == 0) {	//��Դ����
				if(fixDevice->value==0 || fixDevice->value==1)
						Gaspower_Command(GAS_SET_ONOFF, fixDevice->value, Gaspower_index);
				
    } else if (strcmp(fixDevice->key, "Summer_winterMode_Set") == 0) {//�ļ�����ģʽ�趨
				if(fixDevice->value==1 || fixDevice->value==2)
						Gaspower_Command(GAS_SET_MODE, fixDevice->value, Gaspower_index);
				
    } else if (strcmp(fixDevice->key, "BathTemp_Set") == 0) {//��ԡ�¶��趨
				if(fixDevice->value>=30 && fixDevice->value<= 75)
        Gaspower_Command(GAS_SET_BATHROOM_TEMP, fixDevice->value, Gaspower_index);
				
    } else if (strcmp(fixDevice->key, "HeatingTemperature_Set") == 0) {//��ů�¶��趨
				if(fixDevice->value>=30 && fixDevice->value<= 80)
        Gaspower_Command(GAS_SET_HEATINGTEMP, fixDevice->value, Gaspower_index);
				
    } else if (strcmp(fixDevice->key, "DeviceReset") == 0) {//Զ�̸�λ
				if(fixDevice->value==1)
        Gaspower_Command(GAS_SET_RST, fixDevice->value, Gaspower_index);
				update_flag=1;
    } else if (strcmp(fixDevice->key, "Hot_water_Circ_ON") == 0) { //��ˮѭ�������趨
				if(fixDevice->value==0 || fixDevice->value==1)
        Gaspower_Command(GAS_SET_HOT_WATER_CIRC, fixDevice->value, Gaspower_index);
			  
				
    } else if (strcmp(fixDevice->key, "LockSwitch") == 0) {//����
				if(fixDevice->value==0 || fixDevice->value==1)
        Gaspower_Command(GAS_SET_LOCK, fixDevice->value, Gaspower_index);
				
    } else if (strcmp(fixDevice->key, "ECO_Set") == 0) {//ECO����
				if(fixDevice->value==0 || fixDevice->value==1)
        Gaspower_Command(GAS_SET_ECO, fixDevice->value, Gaspower_index);
				
		} 
		else if (strcmp(fixDevice->key, "Realtime_Set") == 0) {//ʱ��У׼3412��1234 12:34
//				fixDevice->value=fixDevice->value/100+fixDevice->value%100*100;
//					if(fixDevice->value < 5)  01,34  3401   ���ӣ�Сʱ
//					zero_value=fixDevice->value;
//						zero_cnt++;
					if(fixDevice->value < 5)
					{
						CostofGas = 0;  //����ȼ������
					  CostofWater = 0;
						CostofElect = 0;
					}
        Gaspower_Command(GAS_SET_TIME_CALIB, fixDevice->value, Gaspower_index);
    } 
}

//ʱ��У׼
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
				CostofGas = 0;  //����ȼ������
				CostofWater = 0;
				CostofElect = 0;
			}
				
			Gaspower_Command(GAS_SET_TIME_CALIB,dat,Gaspower_index);
}
