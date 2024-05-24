/****************************************************************************
 * 版权信息：
 *
 * 系统名称：
 *
 * 文件名称：global.h
 *
 * 文件说明: 公用头文件
 *    
 *         
 * 其他说明：
 *
 * 作    者：rony
 *
 * 版本信息：V1.00
 *
 * 设计日期：2022-10-29
 *
 ***************************************************************************/

#ifndef __GLOBAL_H
#define __GLOBAL_H

//#include <includes.h>
#include <stdlib.h>
#include "stdio.h"
#include "string.h"
#include "stm32g0xx_it.h"
#include "ec600n.h"
#include "debug.h"
#include "modbus.h"
#include "Gaspower.h"
#include "flash.h"
#include "Mqtt_topic.h"
#include "lte_demo.h"
#include "main.h"
#include "usart.h"
#include "tim.h"
#include "gpio.h"


// 软件版本
#define VER_MAJOR				2
#define VER_MINOR				1//0 
#define VER_REVISION		5//4//3//2//1//0//4//3//2//1//0


//---------- 宏函数定义 -----------------------------------------
#define INC_LIMIT(x,max)	if(x < max) x++
#define DEC_LIMIT(x,min) 	if(x > min) x--

#define UP_LIMIT(x,max)		if(x > max) x = max
#define DN_LIMIT(x,min)		if(x < min) x = min

// 字高低字节交换
#define WORD_SWAP(a) 		a = ((a<<8)&0xFF00) | ((a>>8)&0x00FF)
// 字母小写转大写
#define char_tolower(c)     (char) ((c >= 'a' && c <= 'z') ? (c & ~0x20) : c)  






typedef signed long  s32;
typedef signed short s16;
typedef signed char  s8;

typedef signed long  const sc32;  /* Read Only */
typedef signed short const sc16;  /* Read Only */
typedef signed char  const sc8;   /* Read Only */

typedef volatile signed long  vs32;
typedef volatile signed short vs16;
typedef volatile signed char  vs8;

typedef volatile signed long  const vsc32;  /* Read Only */
typedef volatile signed short const vsc16;  /* Read Only */
typedef volatile signed char  const vsc8;   /* Read Only */

typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;

typedef unsigned long  const uc32;  /* Read Only */
typedef unsigned short const uc16;  /* Read Only */
typedef unsigned char  const uc8;   /* Read Only */

typedef volatile unsigned long  vu32;
typedef volatile unsigned short vu16;
typedef volatile unsigned char  vu8;

typedef volatile unsigned long  const vuc32;  /* Read Only */
typedef volatile unsigned short const vuc16;  /* Read Only */
typedef volatile unsigned char  const vuc8;   /* Read Only */

typedef enum {FALSE = 0, TRUE = !FALSE} bool;



#define U8_MAX     ((u8)255)
#define S8_MAX     ((s8)127)
#define S8_MIN     ((s8)-128)
#define U16_MAX    ((u16)65535u)
#define S16_MAX    ((s16)32767)
#define S16_MIN    ((s16)-32768)
#define U32_MAX    ((u32)4294967295uL)
#define S32_MAX    ((s32)2147483647)
#define S32_MIN    ((s32)-2147483648)
			



unsigned short CRC16(unsigned char *CRCData,unsigned short DataLentch);
unsigned short CheckSum(unsigned char *ChkBuf, unsigned short DataLen);
unsigned char BCD2HEX(unsigned char bcd_data);      
unsigned char HEX2BCD(unsigned char hex_data);

float Int2Float(int val);

void delay_us(u16 time);
void delay_ms(u16 time);

#endif


