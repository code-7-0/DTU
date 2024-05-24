#include "main.h"
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __flash_H
#define __flash_H
#ifdef __cplusplus

 extern "C" {
#endif

#define		STM32_FLASH_BASE  0x0801f800


//FLASH�洢������Ϣ
#pragma pack(1)
struct Device_Flash
{					
	  char  hello[128];  				    //��ӭ��
	  char  clientId[128];  				//clientId
		char  username[128];  				//username
		char  mqttHostUrl[128];  	    //mqtt:url
		char  passwd[128];            //MQTT����
		char  port[128];              //port
	  char  QMTSUB1[128];           //����1 /thing/event/property/post
	  char  QMTSUB2[128];          	//����2 /thing/event/property/post_reply
	  char  QMTSUB3[128];          	//����3 
	  char  QMTSUB4[128];         	//M10
	  char  QMTSUB5[128];         	//M11
	  char  QMTSUB6[128];         	//M12
	  char  QMTSUB7[128];         	//M13
	  char  QMTSUB8[128];         	//M14
	  char  QMTSUB9[128];         	//M15
	  char  QMTSUB10[128];         	//M16 �����岻�ܳ�������
};
#pragma pack()


	 
void handle_flash(void);
void init_flash(void);
//���������趨
void flash_Factory(void);
	 
	 
extern const char QMTSUBstr1[];
extern const char QMTSUBstr2[];

extern unsigned char  Flag_wr_flash_u8; //�ַ���д���־
extern unsigned char  Flag_wr_flash_u16; //wordд���־
extern struct   Device_Flash Device_Flash; //FLASH�洢������Ϣ
#ifdef __cplusplus
}
#endif
#endif /*__ flash_H */
