#include "flash.h"

//1.解锁。
//2.确保要写入地址的Flash已经擦除完全。
//3.检查 FLASH_SR 中的 BSY 位，确保当前未执行任何 FLASH 操作。
//4.将 FLASH_CR 寄存器中的 PG 位置 1，激活 FLASH 编程.
//5.针对所需存储器地址（主存储器块或 OTP 区域内）执行数据写入操作.
//6.等待 BSY 位清零，完成一次编程.


//只支持双字写入，2K字节每页 
uint8_t  Flag_fresh_flag; //字符串写入标志 
struct   Device_Flash Device_Flash; //FLASH存储基本信息



/*-----------------------------------------------------------------------------------------
	函数名称：bk_falsh_GetPage
	接口：	addr		flash的地址
	说明：
		通过flash的地址获取地址所在的页
		部分单片机没有Bank 2，需要根据手册自行修改函数
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

//一次性将要擦除的部分页面全部擦除
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
    f.NbPages     = 1;  //一次擦出一个扇区, 以执行一次喂狗，防止超时  
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
//    __ALIGN_BEGIN uint64_t read_data  __ALIGN_END;  //确保8字节数据类型的地址是8的倍数
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
//               HAL_IWDG_Refresh(&hiwdg);	//FLash操作可能非常耗时，如果有看门狗需要喂狗，以下代码由用户实现
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
    __ALIGN_BEGIN uint64_t write_data __ALIGN_END; // 确保8字节数据类型的地址是8的倍数
    HAL_FLASH_Unlock();
    for (i = 0; i < size; i += 8)
    {
        // 清零write_data以确保正确填充
        write_data = 0;

        // 计算剩余字节数，确保不会超出buf的大小
        uint16_t remaining = size - i;
        uint16_t bytesToWrite = remaining < 8 ? remaining : 8;

        // 根据剩余字节填充write_data
        for (uint16_t j = 0; j < bytesToWrite; ++j)
        {
            *((uint8_t *)&write_data + j) = buf[i + j];
        }

        // 写入Flash
        flash_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr + i, write_data);
        if (flash_status != HAL_OK)
        {
            HAL_FLASH_Lock();
            return -1;
        }

        // 读取并验证写入的数据
        __ALIGN_BEGIN uint64_t read_data __ALIGN_END = *(uint64_t *)(addr + i);
				if (read_data != write_data)
//        if ((read_data & ((1ULL << (bytesToWrite * 8)) - 1)) != (write_data & ((1ULL << (bytesToWrite * 8)) - 1)))
        {
            HAL_FLASH_Lock();
            return -1;
        }
        else
        {
            HAL_IWDG_Refresh(&hiwdg); // Flash操作可能非常耗时，如果有看门狗需要喂狗
        }
    }
    HAL_FLASH_Lock();
    return size;
}

 

 /*-----------------------------------------------------------------------------------------
	函数名称：bk_flash_WriteOnePage_uint8
	接口：	flash_addr		写入flash的起始地址
			*write_data		写入数据的指针
			data_size		写入数据的数量,不能大于2048
	说明：
		将8位数组写入flash，包含错误处理
-----------------------------------------------------------------------------------------*/
int bk_flash_WriteOnePage_uint8(uint32_t flash_addr, uint8_t *write_data, uint16_t data_size)
{
    uint16_t i, m, n;
    uint64_t data;
    uint32_t index;
    HAL_StatusTypeDef status;
	
    /* 忙状态 */
		if(__HAL_FLASH_GET_FLAG(FLASH_FLAG_CFGBSY) != 0x00U)
		{
				*(uint32_t *)(flash_addr+100) = 12323;//flash随意写入一个值，使flash触发错误
				FLASH->SR = FLASH_SR_CLEAR;	
		}
		
    // 1 解锁flash
    if(HAL_FLASH_Unlock() != HAL_OK)
    {
        // 解锁失败，可以在这里添加错误处理代码
        return HAL_ERROR;
    }
//		FLASH_WaitForLastOperation(1000); 
    // 2 擦除FLASH
    // 初始化FLASH_EraseInitTypeDef
    FLASH_EraseInitTypeDef f;
    f.TypeErase = FLASH_TYPEERASE_PAGES;
    f.Page = bk_falsh_GetPage(flash_addr);
    f.NbPages = 1;
    // 设置PageError
    uint32_t PageError = 0;
		FLASH->SR = FLASH_SR_CLEAR;		 /* 清除所有的错误标志位 */		
    // 调用擦除函数 
    status = HAL_FLASHEx_Erase(&f, &PageError);
    if(status != HAL_OK)
    { 
        // 擦除失败，可以在这里添加错误处理代码
        HAL_FLASH_Lock();
//				if(__HAL_FLASH_GET_FLAG(FLASH_FLAG_CFGBSY) != 0x00U)
//				{
//						*(uint32_t *)(FLASH_BASE+240) = 12323;//flash随意写入一个值，使flash触发错误
//						FLASH->SR = FLASH_SR_CLEAR;	
//				}
        return status;
    }

    // 3 写FLASH 
    n = data_size % 8;
    m = (data_size / 8) + (n ? 1 : 0);
		for(i=0;i<m;i++)
		{
			data = 0;
			if(i != (m-1) || (i == (m-1) && n == 0))
			{
//				uint64_t *p_data = (uint64_t *)write_data; // 将write_data转换为uint64_t指针
//			// 注意：这里假设write_data已经按照uint64_t对齐
//			// 在某些平台上，如果write_data没有对齐，这可能会引发硬件异常
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
            // 写入失败，可以在这里添加错误处理代码
            HAL_FLASH_Lock();
            return status;
        }
				
		// 4 读取并验证写入的数据
        __ALIGN_BEGIN uint64_t read_data __ALIGN_END = *(uint64_t *)index;
				if (read_data != data)
       {
            HAL_FLASH_Lock();
            return -1;
        }				
				
    }

    // 5 锁定FLASH
    if(HAL_FLASH_Lock() != HAL_OK)
    {
        // 锁定失败，可以在这里添加错误处理代码
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

//    // 1 解锁flash
//    if(HAL_FLASH_Unlock() != HAL_OK)
//    {
//        // 解锁失败，可以在这里添加错误处理代码
//        return HAL_ERROR;
//    }

//    // 2 擦除FLASH
//    // 初始化FLASH_EraseInitTypeDef
//    FLASH_EraseInitTypeDef f;
//    f.TypeErase = FLASH_TYPEERASE_PAGES;
//    f.Page = bk_falsh_GetPage(flash_addr);
//    f.NbPages = 1;
//    // 设置PageError
//    uint32_t PageError = 0;
//    // 调用擦除函数
//    status = HAL_FLASHEx_Erase(&f, &PageError);
//    if(status != HAL_OK)
//    {
//        // 擦除失败，可以在这里添加错误处理代码
//        HAL_FLASH_Lock();
//        return status;
//    }

//    // 3 写FLASH
//    n = data_size % 8;
//    m = (data_size / 8) + (n ? 1 : 0);
//		uint64_t temp[m];// 创建临时数组用于存储对齐后的数据
//		memset(temp, 0, sizeof(temp)); // 初始化为零
//		memcpy((uint8_t*)temp, write_data, data_size); // 复制数据到临时数组
//		// 开始写入 Flash
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


//    // 4 锁定FLASH
//    if(HAL_FLASH_Lock() != HAL_OK)
//    {
//        // 锁定失败，可以在这里添加错误处理代码
//        return HAL_ERROR;
//    }

//    return HAL_OK;
//}

/*-----------------------------------------------------------------------------------------
	函数名称：	bk_flash_Read_uint8
	接口：			flash_addr		读取flash的起始地址
						*write_data		读取数据的指针
						data_size			读取数据的数量
	说明：
		将falsh中的数据按照8位的形式读出指定的长度，存入对应的数组中
-----------------------------------------------------------------------------------------*/


void bk_flash_Read_uint8(uint32_t flash_addr,uint8_t *read_data,uint16_t data_size)
{
	uint16_t i;
	uint32_t index;
	
	for(i=0;i<data_size;i++)
	{
		index = flash_addr + i;
		if(index < (FLASH_BASE + FLASH_BANK_SIZE))
			//read_data[i] = ~*((__IO uint8_t *)index);//反写
		  read_data[i] = *((__IO uint8_t *)index);
		else
			read_data[i] = 0;
	}
}

//出厂参数设定
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
	
//	  strcpy( Device_Flash.QMTSUB4,"/sys/a1t3mgfCASN/yusheng/thing/event/property/post_reply");//固件版本、QCCID
//		strcpy( Device_Flash.QMTSUB5,"/sys/a1t3mgfCASN/yusheng/thing/event/fault/post");	//ota升级
	  Flag_fresh_flag = 1;         //写入
	  debug_printf("出厂参数设置！\r\n");
}



#define  MAX_WRITE_ATTEMPTS 3
int flush_buffer_to_flash(uint32_t flash_addr, uint8_t *write_data, uint16_t data_size) {
    if (data_size > 0) { // 检查是否有数据需要写入
        uint8_t attempt_count = 0;
        int flush_status;
        do {
//					HAL_IWDG_Refresh(&hiwdg);	
//					__disable_irq();
//					HAL_NVIC_DisableIRQ(TIM1_BRK_UP_TRG_COM_IRQn); 
            flush_status = bk_flash_WriteOnePage_uint8(flash_addr, write_data, data_size);
            if (flush_status == HAL_OK) {
                break; // 成功写入，退出循环
            }
//						HAL_NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);

//						__enable_irq();
            attempt_count++;
        } while (attempt_count < MAX_WRITE_ATTEMPTS);

        return flush_status; // 返回最后一次尝试的状态
    }
    return HAL_OK; // 如果缓存为空，则无需写入，直接返回成功
}

//FLASH 处理函数
void Handle_Flash(void)
{ 
    if(Flag_fresh_flag) //存储数据
		{
				debug_printf("启动FLASH存储！\r\n");
			  if(HAL_OK == flush_buffer_to_flash(STM32_FLASH_BASE,(uint8_t*)Device_Flash.hello,2048))
				{
						Flag_fresh_flag = 0; //清零标志 
						debug_printf("FLASH存储完毕！\r\n");					
				}
				else{ 
					debug_printf("FLASH存储失败！\r\n"); 
				}
		}
}

//////FLASH 处理函数
////void Handle_Flash(void)
////{
////	static uint8_t flash_error_cnt=0;
////    if(Flag_fresh_flag) //存储数据
////		{
////				debug_printf("启动FLASH存储！\r\n");
////			  if(HAL_OK == bk_flash_WriteOnePage_uint8(STM32_FLASH_BASE,(uint8_t*)Device_Flash.hello,2048))
////				{
////						Flag_fresh_flag = 0; //清零标志
////						flash_error_cnt=0;
////						debug_printf("FLASH存储完毕！\r\n");					
////				}
////				else{
////					flash_error_cnt++;
////					debug_printf("FLASH存储失败%d次！\r\n",flash_error_cnt);
////					if(flash_error_cnt>=5){
////							debug_printf("存储失败,退出FLASH存储！\r\n");
////							flash_error_cnt=0;
////							Flag_fresh_flag=0;
////					}
////				}

////		}
////		
////}

//flash 初始化
void Init_Flash(void)
{
			uint32_t flash_addr;
			//从flash读取所有设置信息；
				bk_flash_Read_uint8(STM32_FLASH_BASE,(uint8_t*)Device_Flash.hello,13);
	      if(strcmp(Device_Flash.hello,"Hello World!")!=0)
				{
						Flash_Factory();
//					  debug_printf("恢复出厂设置！\r\n");
				}
				else  //读取
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
					 
					 debug_printf("读取FLASH完毕！\r\n");
					 debug_printf("%s\r\n", Device_Flash.hello);
					 debug_printf("*****硬件版本V1.0.0*****\r\n");  // \n表示换行
					 debug_printf("*****软件版本V1.0.0*****\r\n");  // \n表示换行
					 debug_printf("iotCode：%s\r\n", Device_Flash.iotCode);
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

