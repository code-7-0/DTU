#include "serial_transceiver.h"
vModule g_tModule;
uint8_t mqtt_revflag=0;
//uint8_t Ec600_trans_Complete=1;//EC600������ɱ�־
//ʹ���ж� ����ʼDMA����

void USART_DMAstart(void)
{
	memset((uint8_t*)&g_tModule,0,sizeof(vModule)); 
	
	EC600_Usart_Read((uint8_t *)g_tModule.EC600_RevBuf,EC600_SIZE_MAX);
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
	
	Modbus_Usart_Read(g_tModule.Modbus_RevBuf,MODBUS_SIZE_MAX);
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
	
  Debug_Usart_Read(g_tModule.Debug_RevBuf,DEBUG_SIZE_MAX);
  __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);

}
#ifndef EC600_Usart
void EC600_Usart_Read(uint8_t *Data,uint16_t len)
{
	HAL_UART_Receive_DMA(&huart1,Data,len);
} 

void EC600_Usart_Send(uint8_t *buf,uint16_t len)
{
	memcpy(g_tModule.EC600_SendBuf,buf,len);
	HAL_UART_Transmit_DMA(&huart1, g_tModule.EC600_SendBuf,len);
//		 while(((&huart1)->gState) != HAL_UART_STATE_READY);

//	HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, 1000);
//		__HAL_UART_ENABLE_IT(&huart1, UART_IT_TC); 
//  while(__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TC)!=SET);		//�ȴ��������		
////	__HAL_UART_ENABLE_IT(&huart1, UART_IT_TC);  // ����TC�ж�
}

uint8_t file_end_flag=0;//���ܵ��ļ�����֡

void USART1_IRQHandler(void)
{
  uint32_t temp;	
	if(__HAL_UART_GET_FLAG(&huart1,UART_FLAG_IDLE)!= RESET)
	{ 
		__HAL_UART_CLEAR_IDLEFLAG(&huart1);
		HAL_UART_DMAStop(&huart1);
//		HAL_DMA_Abort(&hdma_usart2_rx);
		temp  =  __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);	 
		g_tModule.EC600_BufLen =  EC600_SIZE_MAX - temp; 
		g_tModule.EC600_RevFlag = 1;
//		if(strstr((const char*)g_tModule.EC600_RevBuf,"post_reply")!=NULL)
//			 mqtt_revflag=1;
		uint8_t file_end[]={0xA3,0x3A,0x08,0x00,0x69,0xA5,0x2C,0xC6 };
//		if(strstr((char *)g_tModule.EC600_RevBuf, (char *)file_end) != NULL)
		if(memcmp(g_tModule.EC600_RevBuf, file_end, sizeof(file_end))==0)
			file_end_flag=1;		
//    for (uint8_t i = 0; i < g_tModule.EC600_BufLen-sizeof(file_end); ++i) {
//        if (memcmp(g_tModule.EC600_RevBuf + i, file_end, sizeof(file_end)) == 0) { 
//            file_end_flag=1;
//        }
//    }			
		EC600_Usart_Read((uint8_t *)g_tModule.EC600_RevBuf,EC600_SIZE_MAX);
	 }
   HAL_UART_IRQHandler(&huart1);
}

 
#endif
#ifndef RS485_Usart
void Modbus_Usart_Read(uint8_t *Data,uint16_t len)
{
	HAL_UART_Receive_DMA(&huart2,Data,len);
} 

void Modbus_Usart_Send(uint8_t *buf,uint16_t len)
{ 
		HAL_GPIO_WritePin(RS485_EN2_GPIO_Port,RS485_EN2_Pin,GPIO_PIN_SET);
		HAL_UART_Transmit(&huart2, (uint8_t *)buf, len, 1000);
//		__HAL_UART_ENABLE_IT(&huart2, UART_IT_TC);
	//�ȼ���HAL_UART_Transmit_IT(&huart2, (uint8_t *)data, len, 1000);
		while(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_TC)!=SET);		//�ȴ��������	
		HAL_GPIO_WritePin(RS485_EN2_GPIO_Port,RS485_EN2_Pin,GPIO_PIN_RESET);//����rs485���ݽ���
}
////������ɻص�����
//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
//{
//        if(huart->Instance==USART2)//����Ǵ��� 2
//         {
//                HAL_GPIO_WritePin(RS485_EN2_GPIO_Port,RS485_EN2_Pin,GPIO_PIN_RESET); 
//         }      
//}

void USART2_IRQHandler(void)
{
  uint32_t temp;	
	if(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_IDLE)!= RESET)
	{ 
		__HAL_UART_CLEAR_IDLEFLAG(&huart2);
		HAL_UART_DMAStop(&huart2);
//		HAL_DMA_Abort(&hdma_usart2_rx);
		temp  =  __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);	 
		g_tModule.Modbus_BufLen =  MODBUS_SIZE_MAX - temp; 
		g_tModule.Modbus_RevFlag = 1;

		Modbus_Usart_Read((uint8_t *)g_tModule.Modbus_RevBuf,MODBUS_SIZE_MAX);
	 }
   HAL_UART_IRQHandler(&huart2);
} 

#endif

#ifndef Debug_Usart
void Debug_Usart_Read(uint8_t *Data,uint16_t len)
{
	HAL_UART_Receive_DMA(&huart3,Data,len);
} 

void Serial_SendByte(uint8_t Byte)
{
	HAL_UART_Transmit(&huart3, (uint8_t *)&Byte, 1, 100);	
//	while(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_TC)!=SET);	
}
//�ض���fputs����
int fputc(int ch, FILE *f)
{
	Serial_SendByte(ch);
	return ch;
}

void Debug_Usart_Send(uint8_t *buf,uint16_t len)
{ 
	HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 1000);
//		__HAL_UART_ENABLE_IT(&huart3, UART_IT_TC);
	//�ȼ���HAL_UART_Transmit_IT(&huart2, (uint8_t *)data, len, 1000);
//  while(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_TC)!=SET);		//�ȴ��������	
}

//����˫������
#define DEBUG_SEND_BUFF_SIZE 1024
static char buffer1[DEBUG_SEND_BUFF_SIZE];
static char buffer2[DEBUG_SEND_BUFF_SIZE];
static char* currentBuffer = buffer1;
static char* dmaBuffer = buffer2;
static volatile int bufferSwitchPending = 0; // ����ָʾ�������л��Ƿ������
static volatile int dmaBufferLength = 0; // ��ǰDMA�������е����ݳ���

#define DMA_TRANSFER_TIMEOUT 70000
//debug_printf�����ͻ������л��߼�
void debug_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(currentBuffer, DEBUG_SEND_BUFF_SIZE, fmt, args);
    va_end(args);

    if (len > 0) {
        // ����Ƿ���Ҫ�л�������
        if (bufferSwitchPending) {
            // �ȴ�ǰһ��DMA�������
           int timeout = DMA_TRANSFER_TIMEOUT;
            while (bufferSwitchPending && timeout-- > 0) {
                // ѭ���御�����ּ򵥣��Լ��ٵ��ε���ʱ��
            }
        }

        // ׼��DMA����
        dmaBuffer = currentBuffer;
        dmaBufferLength = len;
        bufferSwitchPending = 1;

        // �л���ǰ������
        currentBuffer = (currentBuffer == buffer1) ? buffer2 : buffer1;

        // ����DMA����
        HAL_UART_Transmit_DMA(&huart3, (uint8_t*)dmaBuffer, dmaBufferLength);
    }
}

void SendDataAsHex(uint8_t* data, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++) {
        debug_printf("%02X ", data[i]);
    }
    debug_printf("\r\n"); // �����ݷ�����Ϻ��ͻ��з����Ա��ڹ۲�
}
//������ɻص�����
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART1)//����Ǵ���EC600
		//�����������еȴ���Ӧ��ʱ�������ʱ��
	 {
//		 DMA_Usart_Read(gps_rbuff,GPS_RBUFF_SIZE);
//		 g_tModule.EC600_RevFlag = 1;
//		 Ec600_trans_Complete=1;
	 }
		if(huart->Instance==USART3)//����Ǵ��� DEBUG
	 { 
        // DMA������ɣ������־
        bufferSwitchPending = 0;		 
	 }	 
	 
} 
void USART3_4_IRQHandler(void)
{
  uint32_t temp;	
	if(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_IDLE)!= RESET)
	{ 
		__HAL_UART_CLEAR_IDLEFLAG(&huart3);
		HAL_UART_DMAStop(&huart3);
		temp  =  __HAL_DMA_GET_COUNTER(&hdma_usart3_rx);	 
		g_tModule.Debug_BufLen =  DEBUG_SIZE_MAX - temp; 
		g_tModule.Debug_RevFlag = 1;
		Debug_Usart_Read((uint8_t *)g_tModule.Debug_RevBuf,DEBUG_SIZE_MAX);
	 }
   HAL_UART_IRQHandler(&huart3);
}
#endif









 



