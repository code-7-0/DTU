#include "global.h"
struct s_debug_onoff debug_onoff = {1,1,1,1};     //������Ϣ����
uint64_t timer_cnt=0;//����
uint32_t outlinetimes = 0;//������������
uint8_t update_flag=1;//OTA������־
uint8_t tcp_connect_cnt=0;//����tcp���� ������ʧ��3�Σ����update_flag��־
uint32_t OTA_store_addr ;//OTA����洢��ַ 
uint8_t qccid_get_flag=0;
uint16_t zero_value;
uint16_t zero_cnt;
uint8_t zero_buf[255];
////CRC16У��
uint16_t CRC16(uint8_t *ptr,  uint16_t len)
{		
	  uint16_t tmp = 0xffff;
//	  uint16_t crc16;
    int i = 0,n = 0;
    for(n = 0; n < len; n++){
        tmp = ptr[n] ^ tmp;
        for(i = 0;i < 8;i++){  /*ÿbit��Ҫ����*/
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
    unsigned long num = strtoul(str, &end, 10); // ����Ϊ10
    
    // ���ת���Ƿ�ɹ�
    if (*end == '\0' && num <= UINT16_MAX) {
        return (uint16_t)num;
    }
    
    // �����ﴦ�����򲻺Ϸ����룬���緵��0�������ض�ֵ
    // ע�⣺ѡ�񷵻�ֵʱҪȷ����������Ϸ���ת��ֵ����
    return 0;
}
 

uint16_t stringToUint16(const char* str) 
{
    uint32_t result = 0; // ʹ�ø���������Լ�����
    while (*str) { // �����ַ���ֱ��������ֹ��'\0'
        if (*str < '0' || *str > '9') {
            // �������ַ�������������ǰ����
            return 0; // ������Ҫ����������
        }
        int digit = *str - '0'; // ���ַ�ת��Ϊ��Ӧ������
        result = result * 10 + digit;
        
//        if (result > UINT16_MAX) {
//            // ������
//            printf("��ֵ����uint16_t��Χ\n");
//            return 0; // ������Ҫ����������
//        }
        ++str;
    }
    return (uint16_t)result; // ��ȫת������Ϊ�����Ѿ���������
}





