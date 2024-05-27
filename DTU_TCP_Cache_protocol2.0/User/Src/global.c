#include "global.h"
struct s_debug_onoff debug_onoff = {1,1,1,1};     //调试信息开关
uint64_t timer_cnt=0;//测试
uint32_t outlinetimes = 0;//断线重连次数
uint8_t update_flag=1;//OTA升级标志
uint8_t tcp_connect_cnt=0;//连接tcp次数 ，连接失败3次，清除update_flag标志
uint32_t OTA_store_addr ;//OTA程序存储地址 
uint8_t qccid_get_flag=0;
uint16_t zero_value;
uint16_t zero_cnt;
uint8_t zero_buf[255];
////CRC16校验
uint16_t CRC16(uint8_t *ptr,  uint16_t len)
{		
	  uint16_t tmp = 0xffff;
//	  uint16_t crc16;
    int i = 0,n = 0;
    for(n = 0; n < len; n++){
        tmp = ptr[n] ^ tmp;
        for(i = 0;i < 8;i++){  /*每bit都要处理*/
            if(tmp & 0x01){
                tmp = tmp >> 1;
                tmp = tmp ^ 0xa001;
            }   
            else{
                tmp = tmp >> 1;
            }   
        }   
    }  
//		crc16 = tmp>>8;
//	  crc16 |= (tmp<<8);
   	return  tmp;		
}

uint16_t convertStringToUint16(const char* str) {
    char *end;
    unsigned long num = strtoul(str, &end, 10); // 基数为10
    
    // 检查转换是否成功
    if (*end == '\0' && num <= UINT16_MAX) {
        return (uint16_t)num;
    }
    
    // 在这里处理错误或不合法输入，例如返回0或其他特定值
    // 注意：选择返回值时要确保它不会与合法的转换值混淆
    return 0;
}
 

uint16_t stringToUint16(const char* str) 
{
    uint32_t result = 0; // 使用更大的类型以检查溢出
    while (*str) { // 遍历字符串直到遇到终止符'\0'
        if (*str < '0' || *str > '9') {
            // 非数字字符，处理错误或提前结束
            return 0; // 根据需要调整错误处理
        }
        int digit = *str - '0'; // 将字符转换为对应的数字
        result = result * 10 + digit;
        
//        if (result > UINT16_MAX) {
//            // 检查溢出
//            printf("数值超出uint16_t范围\n");
//            return 0; // 根据需要调整错误处理
//        }
        ++str;
    }
    return (uint16_t)result; // 安全转换，因为我们已经检查了溢出
}





