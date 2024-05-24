
#include "main.h"
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __modbus_H
#define __modbus_H
#ifdef __cplusplus
 extern "C" {
#endif

	 

void RS485_Send_Data(uint8_t *buf,uint8_t len);

extern	void Handle_Modbus(void);


#ifdef __cplusplus
}
#endif
#endif /*__ usart_H */

