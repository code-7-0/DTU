#ifndef __TEMPER_CONTROL_H
#define __TEMPER_CONTROL_H
#include <string.h> 
#include <stdlib.h> 
#include "global.h" 
#include "serial_transceiver.h"
#include "linked_list.h" 
#include "temper_fifo.h"
#include "cJSON.h"
#include "topic_fifo.h"
#include "rtc.h"
 
typedef enum {
    CMD_ALLOW_JOIN_NETWORK = 0x01,   // �������豸����
    CMD_STOP_JOIN_NETWORK = 0x02,    // ֹͣ���豸����
    CMD_ADD_SUBDEVICE = 0x03,        // ���豸���
    CMD_DELETE_SUBDEVICE = 0x04,     // ���豸ɾ��
    CMD_QUERY_STATUS = 0x05,         // ״̬��ѯ
    CMD_ISSUE_COMMAND = 0x06,        // �����·�
    CMD_REPORT_STATUS = 0x07,        // ״̬�ϱ�
    CMD_GET_LOCAL_TIME = 0x08,       // ��ȡ����ʱ��
    CMD_DELETE_LOCAL_SUBDEVICE = 0x09, // ����ɾ�����豸
    CMD_QUERY_SUBDEVICE_LIST = 0x0A, // ��ѯ���豸�б�
    CMD_REPORT_SUBDEVICE_ONLINE_STATUS = 0x0B, // �ϱ����豸����״̬
    CMD_GET_NETWORK_STATUS = 0x0C,   // ��ȡ����״̬
    CMD_REMOTE_UPGRADE = 0x0D        // Զ������
} CommandCode;

 

#define FRAME_HEADER 0x55AA
#define VERSION 0x00        // �汾�ţ�����ʵ������趨
#define HEADER_SIZE 6


extern uint16_t response_timeout_cnt;
extern uint8_t waiting_for_response;
extern uint8_t expected_command;
extern uint8_t add_device_time_cnt;
extern void construct_send_command_to_fifo(uint8_t command, uint8_t* sub_id,DeviceData *device_data);
extern void Handle_Temper_Crontrol(void); 
extern void Temper_Rapid_Reply(void);

extern uint8_t	temper_rapif_reply_flag;
#endif 
