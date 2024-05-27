#include "rtc.h" 
uint8_t RtcUpdateFlag=0;
DateTime datetime={23,5,17,18,22,55,7}; // 全局变量存储时间信息
 
// 更新时间数据
void updateTime(void) {
	if(RtcUpdateFlag==1){
		RtcUpdateFlag=0;
    datetime.sec++; // 秒加1

    // 处理秒数满60的情况
    if (datetime.sec >= 60) {
        datetime.sec = 0;
        datetime.min++; // 分钟加1

        // 处理分钟满60的情况
        if (datetime.min >= 60) {
            datetime.min = 0;
            datetime.hour++; // 小时加1

            // 处理小时满24的情况
            if (datetime.hour >= 24) {
                datetime.hour = 0;
                datetime.week = (datetime.week % 7) + 1; // 更新星期
                
                datetime.day++; // 天数加1

                // 处理月末的情况
                uint8_t daysInMonth;
                switch (datetime.month) {
                    case 4:
                    case 6:
                    case 9:
                    case 11:
                        daysInMonth = 30;
                        break;
                    case 2:
                        if ((datetime.year % 4 == 0 && datetime.year % 100 != 0) || datetime.year % 400 == 0) {
                            daysInMonth = 29;
                        } else {
                            daysInMonth = 28;
                        }
                        break;
                    default:
                        daysInMonth = 31;
                        break;
                }

                // 处理日期满当月天数的情况
                if (datetime.day > daysInMonth) {
                    datetime.day = 1;
                    datetime.month++; // 月份加1

                    // 处理年份满12的情况
                    if (datetime.month > 12) {
                        datetime.month = 1;
                        datetime.year++; // 年份加1
                    }
                }
            }
        }
    }
	}
}

 
 
