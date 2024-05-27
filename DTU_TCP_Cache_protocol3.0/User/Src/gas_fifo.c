#include "gas_fifo.h"
//	�ڹ�¯����ָ��棬��ȴ�ʱ��1050ms
 FIFO_GAS fifo_gas = {0,0,0};                                          

void GasFiFo_Reset(void)
{
    fifo_gas._head = fifo_gas._tail = 0;
}

void GasFiFo_Push(uint8_t* gaspc_send)
{
    uint16_t pos = (fifo_gas._head+1)%FIFO_MAX_SIZE;
	if(pos!=fifo_gas._tail)               //ջ��û������Ҳ���Դ���                                           
    {
			memcpy(fifo_gas.send_fifo[fifo_gas._head],gaspc_send,GAS_SEND_SIZE); 
        fifo_gas._head = pos; //ͷָ��ָ���Ѵ����ݵ���һλ
    }
}

void GasFifo_Pop(uint8_t* gaspc_send)
{
    if(fifo_gas._tail!=fifo_gas._head)        //ջ�ڱ��������ݲ���ȡ��                                  
    {
			memcpy(gaspc_send,fifo_gas.send_fifo[fifo_gas._tail],GAS_SEND_SIZE); 
			fifo_gas._tail = (fifo_gas._tail+1)%FIFO_MAX_SIZE;//ָ����δ��ȡ������,%FIFO_MAX_SIZEʹfifo_gas._tail��fifo_gas._head�����ᳬ��FIFO_MAX_SIZE
    }     
}

uint16_t GasFifo_Size(void)
{
    return ((fifo_gas._head+FIFO_MAX_SIZE-fifo_gas._tail)%FIFO_MAX_SIZE);
}





