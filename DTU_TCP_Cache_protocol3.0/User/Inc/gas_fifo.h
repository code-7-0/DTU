#ifndef __GAS_FIFO_H
#define __GAS_FIFO_H
#include "main.h"
#include "string.h"

#define FIFO_MAX_SIZE		10
#define GAS_SEND_SIZE		11
//¸æ¾¯Êý¾Ý¶ÑÕ»
typedef struct{                                                                 
    uint16_t _head;                                                       
    uint16_t _tail;                                                     
    uint8_t send_fifo[FIFO_MAX_SIZE][GAS_SEND_SIZE];                                       
}FIFO_GAS; 

extern void GasFiFo_Push(uint8_t* gaspc_send);
extern void GasFiFo_Push(uint8_t* gaspc_send);
extern void GasFifo_Pop(uint8_t* gaspc_send);
extern uint16_t GasFifo_Size(void);

extern FIFO_GAS fifo_gas;   
 
#endif  

