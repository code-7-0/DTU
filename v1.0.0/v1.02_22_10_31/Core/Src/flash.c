#include "global.h"

//1.������
//2.ȷ��Ҫд���ַ��Flash�Ѿ�������ȫ��
//3.��� FLASH_SR �е� BSY λ��ȷ����ǰδִ���κ� FLASH ������
//4.�� FLASH_CR �Ĵ����е� PG λ�� 1������ FLASH ���.
//5.�������洢����ַ�����洢����� OTP �����ڣ�ִ������д�����.
//6.�ȴ� BSY λ���㣬���һ�α��.


//ֻ֧��˫��д�룬2K�ֽ�ÿҳ

unsigned char  Flag_wr_flash_u8; //�ַ���д���־
unsigned char  Flag_wr_flash_u16; //wordд���־
struct   Device_Flash Device_Flash; //FLASH�洢������Ϣ

void bk_flash_WriteOnePage_uint64(uint32_t flash_addr,uint64_t *write_data,uint16_t data_size);
void bk_flash_WriteOnePage_uint16(uint32_t flash_addr,uint16_t *write_data,uint16_t data_size);
void bk_flash_WriteOnePage_uint8(uint32_t flash_addr,uint8_t *write_data,uint16_t data_size);

void bk_flash_Read_uint16(uint32_t flash_addr,uint16_t *read_data,uint16_t data_size);

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
  
/*-----------------------------------------------------------------------------------------
	�������ƣ�bk_flash_WriteOnePage_uint8
	�ӿڣ�	flash_addr		д��flash����ʼ��ַ
			*write_data		д�����ݵ�ָ��
			data_size		д�����ݵ�����,���ܴ���2048
	˵����
		��8λ����д��flash
-----------------------------------------------------------------------------------------*/
void bk_flash_WriteOnePage_uint8(uint32_t flash_addr,uint8_t *write_data,uint16_t data_size)
{
	uint16_t i,m,n;
	uint64_t data;
	uint32_t index;
// 1 ����flash
	HAL_FLASH_Unlock();
 
// 2 ����FLASH
// ��ʼ��FLASH_EraseInitTypeDef
	FLASH_EraseInitTypeDef f;
	f.TypeErase = FLASH_TYPEERASE_PAGES;					  // ������ʽҳ����
	f.Page = bk_falsh_GetPage(flash_addr);					// ��������ʼҳ
	f.NbPages = 1;											            // ����1ҳ
// ����PageError
	uint32_t PageError = 0;
// ���ò�������
	HAL_FLASHEx_Erase(&f, &PageError);
 
// 3 ȡ��дFLASH
	n = data_size % 8;
	if(n == 0)												// ���ݳ����ܱ�8����
		m = data_size / 8 ;									// �õ�д��flash��˫�ֵĸ���
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
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, index, data);		// �������ȡ��д��Flash ��ȡ������ɾ������
	}
// 4 ����FLASH
  	HAL_FLASH_Lock();		
}


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
	  Flag_wr_flash_u8 = 1;         //д��
	  printf("�����������ã�\r\n");
}

//FLASH ������
void handle_flash(void)
{
    if(Flag_wr_flash_u8) //�洢����
		{
				printf("�����洢��\r\n");
			  bk_flash_WriteOnePage_uint8(STM32_FLASH_BASE,Device_Flash.hello,2048);
			  Flag_wr_flash_u8 = 0; //�����־
			  printf("�洢��ϣ�\r\n");
		}
		
		if(Flag_wr_flash_u16) //����
		{
				Flag_wr_flash_u16 = 0; //�����־
		}
}

//flash ��ʼ��
void init_flash(void)
{
			uint32_t flash_addr;
			//��flash��ȡ����������Ϣ��
				bk_flash_Read_uint8(STM32_FLASH_BASE,Device_Flash.hello,13);
	      if(strcmp(Device_Flash.hello,"Hello World!")!=0)
				{
						flash_Factory();
					  printf("�ظ��������ã�\r\n");
				}else  //��ȡ
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
					printf("��ȡ��ϣ�");
				}
}

