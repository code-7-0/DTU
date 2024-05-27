#include "gas_fifo.h"
//	壁挂炉控制指令缓存，最长等待时间1050ms
 FIFO_GAS fifo_gas = {0,0,0};                                          

void GasFiFo_Reset(void)
{
    fifo_gas._head = fifo_gas._tail = 0;
}

void GasFiFo_Push(uint8_t* gaspc_send)
{
    uint16_t pos = (fifo_gas._head+1)%FIFO_MAX_SIZE;
	if(pos!=fifo_gas._tail)               //栈内没有数，也可以存入                                           
    {
			memcpy(fifo_gas.send_fifo[fifo_gas._head],gaspc_send,GAS_SEND_SIZE); 
        fifo_gas._head = pos; //头指针指向已存数据的下一位
    }
}

void GasFifo_Pop(uint8_t* gaspc_send)
{
    if(fifo_gas._tail!=fifo_gas._head)        //栈内必须有数据才能取出                                  
    {
			memcpy(gaspc_send,fifo_gas.send_fifo[fifo_gas._tail],GAS_SEND_SIZE); 
			fifo_gas._tail = (fifo_gas._tail+1)%FIFO_MAX_SIZE;//指向在未被取出数据,%FIFO_MAX_SIZE使fifo_gas._tail，fifo_gas._head都不会超过FIFO_MAX_SIZE
    }     
}

uint16_t GasFifo_Size(void)
{
    return ((fifo_gas._head+FIFO_MAX_SIZE-fifo_gas._tail)%FIFO_MAX_SIZE);
}





