#ifndef __TEMPER_FIFO_H
#define __TEMPER_FIFO_H
#include "main.h"
#include "string.h"

#define TEMPER_FIFO_MAX_SIZE		5 

//ÃüÁî²ÎÊý
typedef struct{                                                                 
    uint8_t command;                                                       
    uint16_t param_length;                                                     
    uint8_t* params;                                       
}TemperCommand; 
 
typedef struct{                                                                 
    uint16_t _head;                                                       
    uint16_t _tail;                                                     
    TemperCommand send_fifo[TEMPER_FIFO_MAX_SIZE];                                       
}FifoTemper; 



extern void TemperFiFoPush(TemperCommand* temper_send); 
extern void TemperFiFoPop(TemperCommand* temper_send);
extern uint16_t TemperFiFoSize(void);
extern void setTemperCommand(TemperCommand *tc, uint8_t command, uint16_t param_length, uint8_t *params); 
extern FifoTemper fifo_temper;   
 
#endif  

