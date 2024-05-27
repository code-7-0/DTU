#ifndef __RTC_H
#define __RTC_H
#include "global.h"   
// 定义存储时间信息的结构体
typedef struct {
	uint8_t year;//YEAR-2000
	uint8_t	month;//1-12
	uint8_t day;//1-31
	uint8_t hour;//0-23
	uint8_t min;//0-59
	uint8_t sec; //0-59
	uint8_t week;//1-7
}DateTime;

extern DateTime datetime;
extern uint8_t RtcUpdateFlag;
extern void updateTime(void);
#endif 




