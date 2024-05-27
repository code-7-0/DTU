#include "timer.h" 
uint16_t iwdg_cnt=1000;
uint8_t iwdg_flag=0;
uint16_t delaycount=499;
//1ms定时中断 回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
 if(htim==(&htim1))
 {
//	if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) 
//TIM_ClearITPendingBit(TIM1,TIM_IT_Update);  //标准库需要清除TIMx更新中断标志,HAL库不需要,直接写业务逻辑
	
	 timer_cnt++;//启动一个循环周期计时
		if(delaycount > 0)	 delaycount--;
		else  //秒级延时
		{
			delaycount = 499; 
			HAL_GPIO_TogglePin(GPIOB, RUN_LED_Pin);//测试基本时钟
			
			if(IntegrationDelay>0)  IntegrationDelay--;
			else IntegrationFlag = 1;
			
			topic_update_flag=1;//500ms
			
			
		}
 
		//单总线发送等待计时
		if(siglewire_wait_cnt)  siglewire_wait_cnt--;
		
		//EC600定时
		fat_tim_proc();
		
		//2s复位，1s喂狗
		if(iwdg_cnt>0){
			iwdg_cnt--;
			if(iwdg_cnt==0){
					iwdg_cnt=1000;
					iwdg_flag=1;
//					HAL_IWDG_Refresh(&hiwdg);//喂狗
			}
		}	

		
//	__HAL_TIM_DISABLE(htim);//关闭定时器
//			  HAL_TIM_Base_Stop_IT(&htim1);
//          HAL_TIM_Base_Start_IT(&htim1);
 }
}


