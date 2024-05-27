#include "temper_fifo.h"
//	温控板控制指令缓存 
FifoTemper fifo_temper = {0,0,0};   
 
void setTemperCommand(TemperCommand *tc, uint8_t command, uint16_t param_length, uint8_t *params) {
    tc->command = command;
    tc->param_length = param_length; 
    if (tc->params == NULL) {
				tc->params=params; 
    }
}

void TemperFiFoReset(void)
{
    fifo_temper._head = fifo_temper._tail = 0;
}

void TemperFiFoPush(TemperCommand* temper_send)
{
    uint16_t pos = (fifo_temper._head+1)%TEMPER_FIFO_MAX_SIZE;
		if(pos!=fifo_temper._tail)               //栈内没有数，也可以存入                                           
    {
			memcpy(&fifo_temper.send_fifo[fifo_temper._head],temper_send,sizeof(TemperCommand)); 
        fifo_temper._head = pos; //头指针指向已存数据的下一位
    }
}

void TemperFiFoPop(TemperCommand* temper_send)
{
    if(fifo_temper._tail!=fifo_temper._head)        //栈内必须有数据才能取出                                  
    {
			memcpy(temper_send,&fifo_temper.send_fifo[fifo_temper._tail],sizeof(TemperCommand)); 
			fifo_temper._tail = (fifo_temper._tail+1)%TEMPER_FIFO_MAX_SIZE;//指向在未被取出数据,%FIFO_MAX_SIZE使fifo_temper._tail，fifo_temper._head都不会超过FIFO_MAX_SIZE
    }     
}

uint16_t TemperFiFoSize(void)
{
    return ((fifo_temper._head+TEMPER_FIFO_MAX_SIZE-fifo_temper._tail)%TEMPER_FIFO_MAX_SIZE);
}





