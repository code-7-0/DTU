#include "main.h"
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __flash_H
#define __flash_H
#ifdef __cplusplus

 extern "C" {
#endif

#define		STM32_FLASH_BASE  0x0801f800


//FLASH存储基本信息
#pragma pack(1)
struct Device_Flash
{					
	  char  hello[128];  				    //欢迎词
	  char  clientId[128];  				//clientId
		char  username[128];  				//username
		char  mqttHostUrl[128];  	    //mqtt:url
		char  passwd[128];            //MQTT密码
		char  port[128];              //port
	  char  QMTSUB1[128];           //主题1 /thing/event/property/post
	  char  QMTSUB2[128];          	//主题2 /thing/event/property/post_reply
	  char  QMTSUB3[128];          	//主题3 
	  char  QMTSUB4[128];         	//M10
	  char  QMTSUB5[128];         	//M11
	  char  QMTSUB6[128];         	//M12
	  char  QMTSUB7[128];         	//M13
	  char  QMTSUB8[128];         	//M14
	  char  QMTSUB9[128];         	//M15
	  char  QMTSUB10[128];         	//M16 ，定义不能超过此行
};
#pragma pack()


	 
void handle_flash(void);
void init_flash(void);
//出厂参数设定
void flash_Factory(void);
	 
	 
extern const char QMTSUBstr1[];
extern const char QMTSUBstr2[];

extern unsigned char  Flag_wr_flash_u8; //字符串写入标志
extern unsigned char  Flag_wr_flash_u16; //word写入标志
extern struct   Device_Flash Device_Flash; //FLASH存储基本信息
#ifdef __cplusplus
}
#endif
#endif /*__ flash_H */
