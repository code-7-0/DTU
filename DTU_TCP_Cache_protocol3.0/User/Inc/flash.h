#ifndef __flash_H
#define __flash_H
#include "global.h"
#include "stdio.h"
#include "string.h"
#include "iwdg.h"
#include "serial_transceiver.h"
#define		STM32_FLASH_BASE  0x0801f800 
#define	APP2adress  		0x08005000				//要更新到的首地址
#define	APP3adress  		0x08011000				//更新数据暂存的首地址
#define UPGRADEaddr   	0x0801D000				//固件更新标识要写入的FLASH的首地址	 

#define PAGE_SIZE     2048
//FLASH存储基本信息
#pragma pack(1)
struct Device_Flash
{					
	  char  hello[64];  				    //欢迎词
		char  iotCode[64];              //物联码
	  char  clientId[128];  				//clientId
		char  username[128];  				//username
		char  mqttHostUrl[128];  	    //mqtt:url
		char  passwd[128];            //MQTT密码
		char  port[128];              //port
	  char  QMTSUB1[128];           //主题1 /thing/event/property/post
	  char  QMTSUB2[128];          	//主题2 /thing/event/property/post_reply
	  char  QMTSUB3[128];          	//主题3 /thing/event/fault/post
	  char  QMTSUB4[128];         	//主题4 /thing/event/firmware/post
	  char  QMTSUB5[128];         	//主题5 /ota/device/upgrade/ 
	  char  QMTSUB6[128];         	//主题6 thing/event/tempercontrol/distribution
	  char  QMTSUB7[128];         	//主题7 thing/event/tempercontrol/upload
	  char  QMTSUB8[128];         	//M14
	  char  QMTSUB9[128];         	//M15
	  char  QMTSUB10[128];         	//M16 ，定义不能超过此行
};
#pragma pack()



void bk_flash_WriteOnePage_uint64(uint32_t flash_addr,uint64_t *write_data,uint16_t data_size);
void bk_flash_WriteOnePage_uint16(uint32_t flash_addr,uint16_t *write_data,uint16_t data_size);
int bk_flash_WriteOnePage_uint8(uint32_t flash_addr, uint8_t *write_data, uint16_t data_size);

void bk_flash_Read_uint16(uint32_t flash_addr,uint16_t *read_data,uint16_t data_size);	

extern void Handle_Flash(void);
extern void Init_Flash(void);
//出厂参数设定
extern void Flash_Factory(void);
extern int flush_buffer_to_flash(uint32_t flash_addr, uint8_t *write_data, uint16_t data_size); 

//extern unsigned char  Flag_wr_flash_u16; //word写入标志 
extern unsigned char  Flag_fresh_flag; //字符串写入标志
extern struct   Device_Flash Device_Flash; //FLASH存储基本信息

extern int flash_mass_erase(uint32_t flash_addr, uint16_t size);
extern int flash_write(uint32_t addr, const uint8_t *buf, uint16_t size);
#endif /*__ flash_H */
