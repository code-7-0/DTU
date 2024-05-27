#include "rtc.h" 
uint8_t RtcUpdateFlag=0;
DateTime datetime={23,5,17,18,22,55,7}; // ȫ�ֱ����洢ʱ����Ϣ
 
// ����ʱ������
void updateTime(void) {
	if(RtcUpdateFlag==1){
		RtcUpdateFlag=0;
    datetime.sec++; // ���1

    // ����������60�����
    if (datetime.sec >= 60) {
        datetime.sec = 0;
        datetime.min++; // ���Ӽ�1

        // ���������60�����
        if (datetime.min >= 60) {
            datetime.min = 0;
            datetime.hour++; // Сʱ��1

            // ����Сʱ��24�����
            if (datetime.hour >= 24) {
                datetime.hour = 0;
                datetime.week = (datetime.week % 7) + 1; // ��������
                
                datetime.day++; // ������1

                // ������ĩ�����
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

                // �����������������������
                if (datetime.day > daysInMonth) {
                    datetime.day = 1;
                    datetime.month++; // �·ݼ�1

                    // ���������12�����
                    if (datetime.month > 12) {
                        datetime.month = 1;
                        datetime.year++; // ��ݼ�1
                    }
                }
            }
        }
    }
	}
}

 
 
