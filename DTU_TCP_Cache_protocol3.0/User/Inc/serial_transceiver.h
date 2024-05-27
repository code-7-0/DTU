#ifndef   _SERIAL_TRANSCEIVER_H
#define 	_SERIAL_TRANSCEIVER_H
#include "usart.h"  
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#define EC600_SIZE_MAX 	1000//512//1100
#define GATEWAY_SIZE_MAX 256
#define DEBUG_SIZE_MAX 	256
#define EC600_SEND_SIZE  256
#define GATEWAY_SEND_SIZE 256
typedef struct {
  uint8_t  	EC600_RevFlag;	//����1 EC600�������ݱ�־
	uint8_t  	Gateway_RevFlag;	//����2 Gateway�������ݱ�־
	uint8_t  	Debug_RevFlag;	//����3 DEBUG�������ݱ�־  
  uint8_t  	EC600_RevBuf[EC600_SIZE_MAX];	//EC600���ջ���
	uint8_t 	Gateway_RevBuf[GATEWAY_SIZE_MAX];//�¿����ؽ��ջ���
	uint8_t  	Debug_RevBuf[DEBUG_SIZE_MAX];	//DEBUG���ջ��� 
	
	uint8_t  	EC600_SendBuf[EC600_SEND_SIZE];//EC600���ͻ���
	uint8_t		Gateway_SendBuf[GATEWAY_SEND_SIZE];//�¿����ط��ͻ���
	
//	uint16_t 	EC600_SendLen;//EC600���ͳ���
//	uint16_t  Gateway_SendLen;
	uint16_t  EC600_BufLen;  //EC600�������ݳ���
	uint16_t  Gateway_BufLen;	//Gateway�������ݳ���
	uint16_t  Debug_BufLen;	//DEBUG�������ݳ���  
}vModule;

//���ڳ�ʼ��
extern void USART_DMAstart(void);
//ec600
void EC600_Usart_Read(uint8_t *Data,uint16_t len);
extern void EC600_Usart_Send(uint8_t *buf,uint16_t len);
//Gateway
void Gateway_Usart_Read(uint8_t *Data,uint16_t len);
extern void Gateway_Usart_Send(uint8_t *buf,uint16_t len);
//debug
void Debug_Usart_Read(uint8_t *Data,uint16_t len);
void Serial_SendByte(uint8_t Byte);
int fputc(int ch, FILE *f);
void Debug_Usart_Send(uint8_t *buf,uint16_t len);
extern void debug_printf(const char *fmt, ...);
extern void SendDataAsHex(uint8_t* data, uint16_t size);

extern DMA_HandleTypeDef hdma_usart1_tx; 
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart3_rx; 
extern vModule g_tModule;
extern uint8_t mqtt_revflag;
//extern uint8_t Ec600_trans_Complete;
extern uint8_t file_end_flag;
#endif 
