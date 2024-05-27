#include "timer.h" 
uint16_t iwdg_cnt=1000;
uint8_t iwdg_flag=0;
uint16_t delaycount=499;
uint16_t temper_command_05_cnt=600;//10;////10分钟问一次温控器0x05
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
		

		//网关发送命令后等待回应
		if(waiting_for_response==1){
			if(response_timeout_cnt>0) {
					response_timeout_cnt--;
					if(response_timeout_cnt==0){
						waiting_for_response=0;
	//					expected_command=0;
					}
				
			}
		}
		//单总线发送等待计时
		if(siglewire_wait_cnt)  siglewire_wait_cnt--;
		
		//EC600定时
		fat_tim_proc();
		
		//2s复位，1s喂狗
		if(iwdg_cnt>0){
			iwdg_cnt--;

			if(iwdg_cnt==0){
					iwdg_cnt=999;
					if(add_device_time_cnt>0){//60s
							add_device_time_cnt--;
							if(add_device_time_cnt==0){  
								//温控器0x02命令FIFO入栈
								construct_send_command_to_fifo(CMD_STOP_JOIN_NETWORK,NULL,NULL);
							}
					}
					if(temper_command_05_cnt>0){//10分钟
						temper_command_05_cnt--;
						if(temper_command_05_cnt==0){
							temper_command_05_cnt=600;//10;//
							construct_send_command_to_fifo(CMD_QUERY_STATUS,NULL,NULL);
						}
					}
					if(temper_rapif_reply_cnt>0){
						temper_rapif_reply_cnt--;
						if(temper_rapif_reply_cnt==0)
								 temper_rapif_reply_flag=0;
					}
					iwdg_flag=1;//喂狗标志
					RtcUpdateFlag=1;//时钟秒数更新标志 
//					HAL_IWDG_Refresh(&hiwdg);//喂狗
			}
		}	

		
//	__HAL_TIM_DISABLE(htim);//关闭定时器
//			  HAL_TIM_Base_Stop_IT(&htim1);
//          HAL_TIM_Base_Start_IT(&htim1);
 }
}


