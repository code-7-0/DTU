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
    CMD_ALLOW_JOIN_NETWORK = 0x01,   // 允许子设备入网
    CMD_STOP_JOIN_NETWORK = 0x02,    // 停止子设备入网
    CMD_ADD_SUBDEVICE = 0x03,        // 子设备添加
    CMD_DELETE_SUBDEVICE = 0x04,     // 子设备删除
    CMD_QUERY_STATUS = 0x05,         // 状态查询
    CMD_ISSUE_COMMAND = 0x06,        // 命令下发
    CMD_REPORT_STATUS = 0x07,        // 状态上报
    CMD_GET_LOCAL_TIME = 0x08,       // 获取本地时间
    CMD_DELETE_LOCAL_SUBDEVICE = 0x09, // 本地删除子设备
    CMD_QUERY_SUBDEVICE_LIST = 0x0A, // 查询子设备列表
    CMD_REPORT_SUBDEVICE_ONLINE_STATUS = 0x0B, // 上报子设备在线状态
    CMD_GET_NETWORK_STATUS = 0x0C,   // 获取网络状态
    CMD_REMOTE_UPGRADE = 0x0D        // 远程升级
} CommandCode;

 

#define FRAME_HEADER 0x55AA
#define VERSION 0x00        // 版本号，根据实际情况设定
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
