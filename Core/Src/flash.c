#include "global.h"

//1.解锁。
//2.确保要写入地址的Flash已经擦除完全。
//3.检查 FLASH_SR 中的 BSY 位，确保当前未执行任何 FLASH 操作。
//4.将 FLASH_CR 寄存器中的 PG 位置 1，激活 FLASH 编程.
//5.针对所需存储器地址（主存储器块或 OTP 区域内）执行数据写入操作.
//6.等待 BSY 位清零，完成一次编程.


//只支持双字写入，2K字节每页

unsigned char  Flag_wr_flash_u8; //字符串写入标志
unsigned char  Flag_wr_flash_u16; //word写入标志
struct   Device_Flash Device_Flash; //FLASH存储基本信息

void bk_flash_WriteOnePage_uint64(uint32_t flash_addr,uint64_t *write_data,uint16_t data_size);
void bk_flash_WriteOnePage_uint16(uint32_t flash_addr,uint16_t *write_data,uint16_t data_size);
void bk_flash_WriteOnePage_uint8(uint32_t flash_addr,uint8_t *write_data,uint16_t data_size);

void bk_flash_Read_uint16(uint32_t flash_addr,uint16_t *read_data,uint16_t data_size);

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
  
/*-----------------------------------------------------------------------------------------
	函数名称：bk_flash_WriteOnePage_uint8
	接口：	flash_addr		写入flash的起始地址
			*write_data		写入数据的指针
			data_size		写入数据的数量,不能大于2048
	说明：
		将8位数组写入flash
-----------------------------------------------------------------------------------------*/
void bk_flash_WriteOnePage_uint8(uint32_t flash_addr,uint8_t *write_data,uint16_t data_size)
{
	uint16_t i,m,n;
	uint64_t data;
	uint32_t index;
// 1 解锁flash
	HAL_FLASH_Unlock();
 
// 2 擦除FLASH
// 初始化FLASH_EraseInitTypeDef
	FLASH_EraseInitTypeDef f;
	f.TypeErase = FLASH_TYPEERASE_PAGES;					  // 擦除方式页擦除
	f.Page = bk_falsh_GetPage(flash_addr);					// 擦除的起始页
	f.NbPages = 1;											            // 擦除1页
// 设置PageError
	uint32_t PageError = 0;
// 调用擦除函数
	HAL_FLASHEx_Erase(&f, &PageError);
 
// 3 取反写FLASH
	n = data_size % 8;
	if(n == 0)												// 数据长度能被8整除
		m = data_size / 8 ;									// 得到写入flash的双字的个数
	else 
		m = data_size / 8 + 1;
	for(i=0;i<m;i++)
	{
		data = 0;
		if(i != (m-1) || (i == (m-1) && n == 0))
		{
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
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, index, data);		// 如果不想取反写入Flash 把取反符号删除即可
	}
// 4 锁定FLASH
  	HAL_FLASH_Lock();		
}


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
void flash_Factory(void)
{
	  strcpy( Device_Flash.hello,"Hello World!");
	  strcpy( Device_Flash.clientId,"a1t3mgfCASN.yusheng|securemode=2,signmethod=hmacsha256,timestamp=1653163471217|");
	  strcpy( Device_Flash.mqttHostUrl,"a15dLSmBATk.iot-as-mqtt.cn-shanghai.aliyuncs.com");
	  strcpy( Device_Flash.port,"1883");
	  strcpy( Device_Flash.passwd,"e65e7d575af19e7f91f1307576d91dc8a4f6e7f724b578a50547d7cf78115ae6");
	  strcpy( Device_Flash.username,"yusheng&a1t3mgfCASN");
	  strcpy( Device_Flash.QMTSUB1,"/sys/a1t3mgfCASN/yusheng/thing/event/property/post");
	  strcpy( Device_Flash.QMTSUB2,"/sys/a1t3mgfCASN/yusheng/thing/event/property/post_reply");
		strcpy( Device_Flash.QMTSUB3,"/sys/a1t3mgfCASN/yusheng/thing/event/fault/post");
	  Flag_wr_flash_u8 = 1;         //写入
	  printf("出厂参数设置！\r\n");
}

//FLASH 处理函数
void handle_flash(void)
{
    if(Flag_wr_flash_u8) //存储数据
		{
				printf("启动存储！\r\n");
			  bk_flash_WriteOnePage_uint8(STM32_FLASH_BASE,Device_Flash.hello,2048);
			  Flag_wr_flash_u8 = 0; //清零标志
			  printf("存储完毕！\r\n");
		}
		
		if(Flag_wr_flash_u16) //待用
		{
				Flag_wr_flash_u16 = 0; //清零标志
		}
}

//flash 初始化
void init_flash(void)
{
			uint32_t flash_addr;
			//从flash读取所有设置信息；
				bk_flash_Read_uint8(STM32_FLASH_BASE,Device_Flash.hello,13);
	      if(strcmp(Device_Flash.hello,"Hello World!")!=0)
				{
						flash_Factory();
					  printf("回复出厂设置！\r\n");
				}else  //读取
				{
					flash_addr = STM32_FLASH_BASE;
					bk_flash_Read_uint8(flash_addr,Device_Flash.hello,128);flash_addr+=128;
				  bk_flash_Read_uint8(flash_addr,Device_Flash.clientId,128);flash_addr+=128;
					bk_flash_Read_uint8(flash_addr,Device_Flash.username,128);flash_addr+=128;
					bk_flash_Read_uint8(flash_addr,Device_Flash.mqttHostUrl,128);flash_addr+=128;
					bk_flash_Read_uint8(flash_addr,Device_Flash.passwd,128);flash_addr+=128;
					bk_flash_Read_uint8(flash_addr,Device_Flash.port,128);flash_addr+=128;
					bk_flash_Read_uint8(flash_addr,Device_Flash.QMTSUB1,128);flash_addr+=128;
					bk_flash_Read_uint8(flash_addr,Device_Flash.QMTSUB2,128);flash_addr+=128;
					bk_flash_Read_uint8(flash_addr,Device_Flash.QMTSUB3,128);flash_addr+=128; 
					printf("读取完毕！");
				}
}

