#include "temper_fifo.h"
//	�¿ذ����ָ��� 
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
		if(pos!=fifo_temper._tail)               //ջ��û������Ҳ���Դ���                                           
    {
			memcpy(&fifo_temper.send_fifo[fifo_temper._head],temper_send,sizeof(TemperCommand)); 
        fifo_temper._head = pos; //ͷָ��ָ���Ѵ����ݵ���һλ
    }
}

void TemperFiFoPop(TemperCommand* temper_send)
{
    if(fifo_temper._tail!=fifo_temper._head)        //ջ�ڱ��������ݲ���ȡ��                                  
    {
			memcpy(temper_send,&fifo_temper.send_fifo[fifo_temper._tail],sizeof(TemperCommand)); 
			fifo_temper._tail = (fifo_temper._tail+1)%TEMPER_FIFO_MAX_SIZE;//ָ����δ��ȡ������,%FIFO_MAX_SIZEʹfifo_temper._tail��fifo_temper._head�����ᳬ��FIFO_MAX_SIZE
    }     
}

uint16_t TemperFiFoSize(void)
{
    return ((fifo_temper._head+TEMPER_FIFO_MAX_SIZE-fifo_temper._tail)%TEMPER_FIFO_MAX_SIZE);
}





