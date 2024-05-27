#include "flash.h"

//1.������
//2.ȷ��Ҫд���ַ��Flash�Ѿ�������ȫ��
//3.��� FLASH_SR �е� BSY λ��ȷ����ǰδִ���κ� FLASH ������
//4.�� FLASH_CR �Ĵ����е� PG λ�� 1������ FLASH ���.
//5.�������洢����ַ�����洢����� OTP �����ڣ�ִ������д�����.
//6.�ȴ� BSY λ���㣬���һ�α��.


//ֻ֧��˫��д�룬2K�ֽ�ÿҳ 
uint8_t  Flag_fresh_flag; //�ַ���д���־ 
struct   Device_Flash Device_Flash; //FLASH�洢������Ϣ



/*-----------------------------------------------------------------------------------------
	�������ƣ�bk_falsh_GetPage
	�ӿڣ�	addr		flash�ĵ�ַ
	˵����
		ͨ��flash�ĵ�ַ��ȡ��ַ���ڵ�ҳ
		���ֵ�Ƭ��û��Bank 2����Ҫ�����ֲ������޸ĺ���
-----------------------------------------------------------------------------------------*/
static uint32_t bk_falsh_GetPage(uint32_t addr)
{
  uint32_t page = 0;
 
  if (addr < (FLASH_BASE + FLASH_BANK_SIZE))
  {
    /* Bank 1 */
    page = (addr - FLASH_BASE) / FLASH_PAGE_SIZE;
  }
  else
  {
    /* Bank 2 */
    page = (addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
  }
 
  return page;
}

//һ���Խ�Ҫ�����Ĳ���ҳ��ȫ������
int flash_mass_erase(uint32_t flash_addr, uint16_t size)
{		 
    HAL_StatusTypeDef flash_status;
    FLASH_EraseInitTypeDef f;   
    uint32_t HeadErasePage ,PAGEError = 0;
		 HeadErasePage = bk_falsh_GetPage(flash_addr);
    uint16_t erase_pages = size / FLASH_PAGE_SIZE;
    if(size % FLASH_PAGE_SIZE != 0) erase_pages++;           
//    f.Banks = FLASH_BANK_1;
    f.TypeErase   = FLASH_TYPEERASE_PAGES;
    f.NbPages     = 1;  //һ�β���һ������, ��ִ��һ��ι������ֹ��ʱ  
    HAL_FLASH_Unlock();
//	  FLASH_WaitForLastOperation(1000); 
//	 __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR); 
    for (uint16_t i = 0; i < erase_pages; i++) 
    {
        f.Page = HeadErasePage + i;       
        flash_status         = HAL_FLASHEx_Erase(&f, &PAGEError);         
        if (flash_status != HAL_OK) 
        {
            HAL_FLASH_Lock(); 
            return -1;
        }
        else
        {
            HAL_IWDG_Refresh(&hiwdg);	
        }
    }
    HAL_FLASH_Lock(); 
    return size;
}

//int flash_write(uint32_t addr, const uint8_t *buf, uint16_t size)
//{
//		HAL_StatusTypeDef flash_status;
//    uint16_t   i;
////    uint32_t addr = stm32_onchip_flash.addr + offset;
//    __ALIGN_BEGIN uint64_t write_data __ALIGN_END;
//    __ALIGN_BEGIN uint64_t read_data  __ALIGN_END;  //ȷ��8�ֽ��������͵ĵ�ַ��8�ı���
//    HAL_FLASH_Unlock();
//    for (i = 0; i < size; i+=8) 
//    {
////        read_data = *(uint64_t *)(addr+i);
////        if (read_data != (uint64_t)(*((uint64_t *)((uint32_t)buf + i))))
////        {  
//							*((uint8_t *)&write_data + 0) = buf[0 + i];
//							*((uint8_t *)&write_data + 1) = buf[1 + i];
//							*((uint8_t *)&write_data + 2) = buf[2 + i];
//							*((uint8_t *)&write_data + 3) = buf[3 + i];
//							*((uint8_t *)&write_data + 4) = buf[4 + i];
//							*((uint8_t *)&write_data + 5) = buf[5 + i];
//							*((uint8_t *)&write_data + 6) = buf[6 + i];
//							*((uint8_t *)&write_data + 7) = buf[7 + i];			
//						flash_status=HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,(addr+i),write_data);      
////            HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,(uint32_t)(uint64_t*)(addr+i),*((uint64_t *)(buf + i)));                
//						if(flash_status != HAL_OK)
//							return -1;
//						read_data = *(uint64_t *)(addr+i);
//            if (read_data != write_data)
//            {
//                HAL_FLASH_Lock(); 
//                return -1;
//            }
//            else
//            {        
//               HAL_IWDG_Refresh(&hiwdg);	//FLash�������ܷǳ���ʱ������п��Ź���Ҫι�������´������û�ʵ��
//            }  
////        }     
//    } 
//    HAL_FLASH_Lock();
//    return size;
//}
int flash_write(uint32_t addr, const uint8_t *buf, uint16_t size)
{
    HAL_StatusTypeDef flash_status;
    uint16_t i;
    __ALIGN_BEGIN uint64_t write_data __ALIGN_END; // ȷ��8�ֽ��������͵ĵ�ַ��8�ı���
    HAL_FLASH_Unlock();
    for (i = 0; i < size; i += 8)
    {
        // ����write_data��ȷ����ȷ���
        write_data = 0;

        // ����ʣ���ֽ�����ȷ�����ᳬ��buf�Ĵ�С
        uint16_t remaining = size - i;
        uint16_t bytesToWrite = remaining < 8 ? remaining : 8;

        // ����ʣ���ֽ����write_data
        for (uint16_t j = 0; j < bytesToWrite; ++j)
        {
            *((uint8_t *)&write_data + j) = buf[i + j];
        }

        // д��Flash
        flash_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr + i, write_data);
        if (flash_status != HAL_OK)
        {
            HAL_FLASH_Lock();
            return -1;
        }

        // ��ȡ����֤д�������
        __ALIGN_BEGIN uint64_t read_data __ALIGN_END = *(uint64_t *)(addr + i);
				if (read_data != write_data)
//        if ((read_data & ((1ULL << (bytesToWrite * 8)) - 1)) != (write_data & ((1ULL << (bytesToWrite * 8)) - 1)))
        {
            HAL_FLASH_Lock();
            return -1;
        }
        else
        {
            HAL_IWDG_Refresh(&hiwdg); // Flash�������ܷǳ���ʱ������п��Ź���Ҫι��
        }
    }
    HAL_FLASH_Lock();
    return size;
}

 

 /*-----------------------------------------------------------------------------------------
	�������ƣ�bk_flash_WriteOnePage_uint8
	�ӿڣ�	flash_addr		д��flash����ʼ��ַ
			*write_data		д�����ݵ�ָ��
			data_size		д�����ݵ�����,���ܴ���2048
	˵����
		��8λ����д��flash������������
-----------------------------------------------------------------------------------------*/
int bk_flash_WriteOnePage_uint8(uint32_t flash_addr, uint8_t *write_data, uint16_t data_size)
{
    uint16_t i, m, n;
    uint64_t data;
    uint32_t index;
    HAL_StatusTypeDef status;
	
    /* æ״̬ */
		if(__HAL_FLASH_GET_FLAG(FLASH_FLAG_CFGBSY) != 0x00U)
		{
				*(uint32_t *)(flash_addr+100) = 12323;//flash����д��һ��ֵ��ʹflash��������
				FLASH->SR = FLASH_SR_CLEAR;	
		}
		
    // 1 ����flash
    if(HAL_FLASH_Unlock() != HAL_OK)
    {
        // ����ʧ�ܣ�������������Ӵ��������
        return HAL_ERROR;
    }
//		FLASH_WaitForLastOperation(1000); 
    // 2 ����FLASH
    // ��ʼ��FLASH_EraseInitTypeDef
    FLASH_EraseInitTypeDef f;
    f.TypeErase = FLASH_TYPEERASE_PAGES;
    f.Page = bk_falsh_GetPage(flash_addr);
    f.NbPages = 1;
    // ����PageError
    uint32_t PageError = 0;
		FLASH->SR = FLASH_SR_CLEAR;		 /* ������еĴ����־λ */		
    // ���ò������� 
    status = HAL_FLASHEx_Erase(&f, &PageError);
    if(status != HAL_OK)
    { 
        // ����ʧ�ܣ�������������Ӵ��������
        HAL_FLASH_Lock();
//				if(__HAL_FLASH_GET_FLAG(FLASH_FLAG_CFGBSY) != 0x00U)
//				{
//						*(uint32_t *)(FLASH_BASE+240) = 12323;//flash����д��һ��ֵ��ʹflash��������
//						FLASH->SR = FLASH_SR_CLEAR;	
//				}
        return status;
    }

    // 3 дFLASH 
    n = data_size % 8;
    m = (data_size / 8) + (n ? 1 : 0);
		for(i=0;i<m;i++)
		{
			data = 0;
			if(i != (m-1) || (i == (m-1) && n == 0))
			{
//				uint64_t *p_data = (uint64_t *)write_data; // ��write_dataת��Ϊuint64_tָ��
//			// ע�⣺�������write_data�Ѿ�����uint64_t����
//			// ��ĳЩƽ̨�ϣ����write_dataû�ж��룬����ܻ�����Ӳ���쳣
//			data = p_data[i];
				
				*((uint8_t *)&data + 0) = write_data[0 + i*8];
				*((uint8_t *)&data + 1) = write_data[1 + i*8];
				*((uint8_t *)&data + 2) = write_data[2 + i*8];
				*((uint8_t *)&data + 3) = write_data[3 + i*8];
				*((uint8_t *)&data + 4) = write_data[4 + i*8];
				*((uint8_t *)&data + 5) = write_data[5 + i*8];
				*((uint8_t *)&data + 6) = write_data[6 + i*8];
				*((uint8_t *)&data + 7) = write_data[7 + i*8];			
			}
			else
			{
					*((uint8_t *)&data + 0) = write_data[0 + i*8];
				if(n >= 2)
					*((uint8_t *)&data + 1) = write_data[1 + i*8];
				if(n >= 3)
					*((uint8_t *)&data + 2) = write_data[2 + i*8];		
				if(n >= 4)
					*((uint8_t *)&data + 3) = write_data[3 + i*8];
				if(n >= 5)
					*((uint8_t *)&data + 4) = write_data[4 + i*8];		
				if(n >= 6)
					*((uint8_t *)&data + 5) = write_data[5 + i*8];
				if(n >= 7)
					*((uint8_t *)&data + 6) = write_data[6 + i*8];					
			}
				index = flash_addr + i*8;
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, index, data);
        if(status != HAL_OK)
        {
            // д��ʧ�ܣ�������������Ӵ��������
            HAL_FLASH_Lock();
            return status;
        }
				
		// 4 ��ȡ����֤д�������
        __ALIGN_BEGIN uint64_t read_data __ALIGN_END = *(uint64_t *)index;
				if (read_data != data)
       {
            HAL_FLASH_Lock();
            return -1;
        }				
				
    }

    // 5 ����FLASH
    if(HAL_FLASH_Lock() != HAL_OK)
    {
        // ����ʧ�ܣ�������������Ӵ��������
        return status;
    }

    return HAL_OK;
} 



//HAL_StatusTypeDef bk2_flash_WriteOnePage_uint8(uint32_t flash_addr, uint8_t *write_data, uint16_t data_size)
//{
//    uint16_t i, m, n;
//    uint64_t data;
//    uint32_t index;
//    HAL_StatusTypeDef status;

//    // 1 ����flash
//    if(HAL_FLASH_Unlock() != HAL_OK)
//    {
//        // ����ʧ�ܣ�������������Ӵ��������
//        return HAL_ERROR;
//    }

//    // 2 ����FLASH
//    // ��ʼ��FLASH_EraseInitTypeDef
//    FLASH_EraseInitTypeDef f;
//    f.TypeErase = FLASH_TYPEERASE_PAGES;
//    f.Page = bk_falsh_GetPage(flash_addr);
//    f.NbPages = 1;
//    // ����PageError
//    uint32_t PageError = 0;
//    // ���ò�������
//    status = HAL_FLASHEx_Erase(&f, &PageError);
//    if(status != HAL_OK)
//    {
//        // ����ʧ�ܣ�������������Ӵ��������
//        HAL_FLASH_Lock();
//        return status;
//    }

//    // 3 дFLASH
//    n = data_size % 8;
//    m = (data_size / 8) + (n ? 1 : 0);
//		uint64_t temp[m];// ������ʱ�������ڴ洢����������
//		memset(temp, 0, sizeof(temp)); // ��ʼ��Ϊ��
//		memcpy((uint8_t*)temp, write_data, data_size); // �������ݵ���ʱ����
//		// ��ʼд�� Flash
//		for (i = 0; i < m; i++)
//		{
//				index = flash_addr + i * 8;
//				status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, index, temp[i]);

//				if (status != HAL_OK)
//				{
//           HAL_FLASH_Lock();
//            return status;
//				}
//		}


//    // 4 ����FLASH
//    if(HAL_FLASH_Lock() != HAL_OK)
//    {
//        // ����ʧ�ܣ�������������Ӵ��������
//        return HAL_ERROR;
//    }

//    return HAL_OK;
//}

/*-----------------------------------------------------------------------------------------
	�������ƣ�	bk_flash_Read_uint8
	�ӿڣ�			flash_addr		��ȡflash����ʼ��ַ
						*write_data		��ȡ���ݵ�ָ��
						data_size			��ȡ���ݵ�����
	˵����
		��falsh�е����ݰ���8λ����ʽ����ָ���ĳ��ȣ������Ӧ��������
-----------------------------------------------------------------------------------------*/


void bk_flash_Read_uint8(uint32_t flash_addr,uint8_t *read_data,uint16_t data_size)
{
	uint16_t i;
	uint32_t index;
	
	for(i=0;i<data_size;i++)
	{
		index = flash_addr + i;
		if(index < (FLASH_BASE + FLASH_BANK_SIZE))
			//read_data[i] = ~*((__IO uint8_t *)index);//��д
		  read_data[i] = *((__IO uint8_t *)index);
		else
			read_data[i] = 0;
	}
}

//���������趨
void Flash_Factory(void)
{
	  strcpy( Device_Flash.hello,"Hello World!");
	  strcpy( Device_Flash.iotCode,"138");	
	  strcpy( Device_Flash.clientId,"a1t3mgfCASN.yusheng|securemode=2,signmethod=hmacsha256,timestamp=1653163471217|");
	  strcpy( Device_Flash.mqttHostUrl,"a15dLSmBATk.iot-as-mqtt.cn-shanghai.aliyuncs.com");
	  strcpy( Device_Flash.port,"1883");
	  strcpy( Device_Flash.passwd,"e65e7d575af19e7f91f1307576d91dc8a4f6e7f724b578a50547d7cf78115ae6");
	  strcpy( Device_Flash.username,"yusheng&a1t3mgfCASN");
	  strcpy( Device_Flash.QMTSUB1,"/sys/a1t3mgfCASN/yusheng/thing/event/property/post");
	  strcpy( Device_Flash.QMTSUB2,"/sys/a1t3mgfCASN/yusheng/thing/event/property/post_reply");
		strcpy( Device_Flash.QMTSUB3,"/sys/a1t3mgfCASN/yusheng/thing/event/fault/post");
	  strcpy( Device_Flash.QMTSUB4,"/sys/i60d1iPGj1Pl/lhGP0rlsr5COaGog/thing/event/firmware/post");
		strcpy( Device_Flash.QMTSUB5,"/ota/device/upgrade/i60d1iPGj1Pl/lhGP0rlsr5COaGog");	 
		//  /sys/i60d1iPGj1Pl/NN7qzX8VXtQdC22l/thing/event/fault/post
	
//	  strcpy( Device_Flash.QMTSUB4,"/sys/a1t3mgfCASN/yusheng/thing/event/property/post_reply");//�̼��汾��QCCID
//		strcpy( Device_Flash.QMTSUB5,"/sys/a1t3mgfCASN/yusheng/thing/event/fault/post");	//ota����
	  Flag_fresh_flag = 1;         //д��
	  debug_printf("�����������ã�\r\n");
}



#define  MAX_WRITE_ATTEMPTS 3
int flush_buffer_to_flash(uint32_t flash_addr, uint8_t *write_data, uint16_t data_size) {
    if (data_size > 0) { // ����Ƿ���������Ҫд��
        uint8_t attempt_count = 0;
        int flush_status;
        do {
//					HAL_IWDG_Refresh(&hiwdg);	
//					__disable_irq();
//					HAL_NVIC_DisableIRQ(TIM1_BRK_UP_TRG_COM_IRQn); 
            flush_status = bk_flash_WriteOnePage_uint8(flash_addr, write_data, data_size);
            if (flush_status == HAL_OK) {
                break; // �ɹ�д�룬�˳�ѭ��
            }
//						HAL_NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);

//						__enable_irq();
            attempt_count++;
        } while (attempt_count < MAX_WRITE_ATTEMPTS);

        return flush_status; // �������һ�γ��Ե�״̬
    }
    return HAL_OK; // �������Ϊ�գ�������д�룬ֱ�ӷ��سɹ�
}

//FLASH ������
void Handle_Flash(void)
{ 
    if(Flag_fresh_flag) //�洢����
		{
				debug_printf("����FLASH�洢��\r\n");
			  if(HAL_OK == flush_buffer_to_flash(STM32_FLASH_BASE,(uint8_t*)Device_Flash.hello,2048))
				{
						Flag_fresh_flag = 0; //�����־ 
						debug_printf("FLASH�洢��ϣ�\r\n");					
				}
				else{ 
					debug_printf("FLASH�洢ʧ�ܣ�\r\n"); 
				}
		}
}

//////FLASH ������
////void Handle_Flash(void)
////{
////	static uint8_t flash_error_cnt=0;
////    if(Flag_fresh_flag) //�洢����
////		{
////				debug_printf("����FLASH�洢��\r\n");
////			  if(HAL_OK == bk_flash_WriteOnePage_uint8(STM32_FLASH_BASE,(uint8_t*)Device_Flash.hello,2048))
////				{
////						Flag_fresh_flag = 0; //�����־
////						flash_error_cnt=0;
////						debug_printf("FLASH�洢��ϣ�\r\n");					
////				}
////				else{
////					flash_error_cnt++;
////					debug_printf("FLASH�洢ʧ��%d�Σ�\r\n",flash_error_cnt);
////					if(flash_error_cnt>=5){
////							debug_printf("�洢ʧ��,�˳�FLASH�洢��\r\n");
////							flash_error_cnt=0;
////							Flag_fresh_flag=0;
////					}
////				}

////		}
////		
////}

//flash ��ʼ��
void Init_Flash(void)
{
			uint32_t flash_addr;
			//��flash��ȡ����������Ϣ��
				bk_flash_Read_uint8(STM32_FLASH_BASE,(uint8_t*)Device_Flash.hello,13);
	      if(strcmp(Device_Flash.hello,"Hello World!")!=0)
				{
						Flash_Factory();
//					  debug_printf("�ָ��������ã�\r\n");
				}
				else  //��ȡ
				{
					flash_addr = STM32_FLASH_BASE;
					bk_flash_Read_uint8(flash_addr,(uint8_t*)Device_Flash.hello,64);flash_addr+=64;
					bk_flash_Read_uint8(flash_addr,(uint8_t*)Device_Flash.iotCode,64);flash_addr+=64;
				  bk_flash_Read_uint8(flash_addr,(uint8_t*)Device_Flash.clientId,128);flash_addr+=128;
					bk_flash_Read_uint8(flash_addr,(uint8_t*)Device_Flash.username,128);flash_addr+=128;
					bk_flash_Read_uint8(flash_addr,(uint8_t*)Device_Flash.mqttHostUrl,128);flash_addr+=128;
					bk_flash_Read_uint8(flash_addr,(uint8_t*)Device_Flash.passwd,128);flash_addr+=128;
					bk_flash_Read_uint8(flash_addr,(uint8_t*)Device_Flash.port,128);flash_addr+=128;
					bk_flash_Read_uint8(flash_addr,(uint8_t*)Device_Flash.QMTSUB1,128);flash_addr+=128;
					bk_flash_Read_uint8(flash_addr,(uint8_t*)Device_Flash.QMTSUB2,128);flash_addr+=128;
					bk_flash_Read_uint8(flash_addr,(uint8_t*)Device_Flash.QMTSUB3,128);flash_addr+=128; 
					bk_flash_Read_uint8(flash_addr,(uint8_t*)Device_Flash.QMTSUB4,128);flash_addr+=128;
					bk_flash_Read_uint8(flash_addr,(uint8_t*)Device_Flash.QMTSUB5,128);flash_addr+=128;
//	  strcpy( Device_Flash.QMTSUB4,"/sys/i60d1iPGj1Pl/lhGP0rlsr5COaGog/thing/event/firmware/post");
//		strcpy( Device_Flash.QMTSUB5,"/ota/device/upgrade/i60d1iPGj1Pl/lhGP0rlsr5COaGog/");						
					 
					 debug_printf("��ȡFLASH��ϣ�\r\n");
					 debug_printf("%s\r\n", Device_Flash.hello);
					 debug_printf("*****Ӳ���汾V1.0.0*****\r\n");  // \n��ʾ����
					 debug_printf("*****����汾V1.0.0*****\r\n");  // \n��ʾ����
					 debug_printf("iotCode��%s\r\n", Device_Flash.iotCode);
					 debug_printf("ClientId:    %s\r\n", Device_Flash.clientId);
					 debug_printf("MqttHostUrl: %s\r\n", Device_Flash.mqttHostUrl);
					 debug_printf("Port:        %s\r\n", Device_Flash.port);
					 debug_printf("Passwd:      %s\r\n", Device_Flash.passwd);
					 debug_printf("Username:    %s\r\n", Device_Flash.username);
					 debug_printf("QMTSUB1:     %s\r\n", Device_Flash.QMTSUB1);
					 debug_printf("QMTSUB2:     %s\r\n", Device_Flash.QMTSUB2);
					 debug_printf("QMTSUB3:     %s\r\n", Device_Flash.QMTSUB3);		
					 debug_printf("QMTSUB4:     %s\r\n", Device_Flash.QMTSUB4);
					 debug_printf("QMTSUB5:     %s\r\n", Device_Flash.QMTSUB5);						 
				}
}

