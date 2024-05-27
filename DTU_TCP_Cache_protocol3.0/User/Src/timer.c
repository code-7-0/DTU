#include "timer.h" 
uint16_t iwdg_cnt=1000;
uint8_t iwdg_flag=0;
uint16_t delaycount=499;
uint16_t temper_command_05_cnt=600;//10;////10������һ���¿���0x05
//1ms��ʱ�ж� �ص�����
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
 if(htim==(&htim1))
 {
//	if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) 
//TIM_ClearITPendingBit(TIM1,TIM_IT_Update);  //��׼����Ҫ���TIMx�����жϱ�־,HAL�ⲻ��Ҫ,ֱ��дҵ���߼�
	
	 timer_cnt++;//����һ��ѭ�����ڼ�ʱ
		if(delaycount > 0)	 delaycount--;
		else  //�뼶��ʱ
		{
			delaycount = 499; 
			HAL_GPIO_TogglePin(GPIOB, RUN_LED_Pin);//���Ի���ʱ��
			
			if(IntegrationDelay>0)  IntegrationDelay--;
			else IntegrationFlag = 1;
			
			topic_update_flag=1;//500ms
			
			
		}
		

		//���ط��������ȴ���Ӧ
		if(waiting_for_response==1){
			if(response_timeout_cnt>0) {
					response_timeout_cnt--;
					if(response_timeout_cnt==0){
						waiting_for_response=0;
	//					expected_command=0;
					}
				
			}
		}
		//�����߷��͵ȴ���ʱ
		if(siglewire_wait_cnt)  siglewire_wait_cnt--;
		
		//EC600��ʱ
		fat_tim_proc();
		
		//2s��λ��1sι��
		if(iwdg_cnt>0){
			iwdg_cnt--;

			if(iwdg_cnt==0){
					iwdg_cnt=999;
					if(add_device_time_cnt>0){//60s
							add_device_time_cnt--;
							if(add_device_time_cnt==0){  
								//�¿���0x02����FIFO��ջ
								construct_send_command_to_fifo(CMD_STOP_JOIN_NETWORK,NULL,NULL);
							}
					}
					if(temper_command_05_cnt>0){//10����
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
					iwdg_flag=1;//ι����־
					RtcUpdateFlag=1;//ʱ���������±�־ 
//					HAL_IWDG_Refresh(&hiwdg);//ι��
			}
		}	

		
//	__HAL_TIM_DISABLE(htim);//�رն�ʱ��
//			  HAL_TIM_Base_Stop_IT(&htim1);
//          HAL_TIM_Base_Start_IT(&htim1);
 }
}


