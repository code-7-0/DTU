#include "timer.h" 
uint16_t iwdg_cnt=1000;
uint8_t iwdg_flag=0;
uint16_t delaycount=499;
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
 
		//�����߷��͵ȴ���ʱ
		if(siglewire_wait_cnt)  siglewire_wait_cnt--;
		
		//EC600��ʱ
		fat_tim_proc();
		
		//2s��λ��1sι��
		if(iwdg_cnt>0){
			iwdg_cnt--;
			if(iwdg_cnt==0){
					iwdg_cnt=1000;
					iwdg_flag=1;
//					HAL_IWDG_Refresh(&hiwdg);//ι��
			}
		}	

		
//	__HAL_TIM_DISABLE(htim);//�رն�ʱ��
//			  HAL_TIM_Base_Stop_IT(&htim1);
//          HAL_TIM_Base_Start_IT(&htim1);
 }
}


