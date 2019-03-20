/*
 * user_flash_L072.c	L072芯片Flahs驱动代码
*/

#include "user_flash_L072.h"
#include "bootloader_config.h"
#include <string.h>
/*
 *	FlashReadPage:		读取1页数据
 *	参数PageAddr：		页地址
 *	参数pBuffer：		保存读取到的数据的指针
 *	返回值：			1成功 0失败	
 */
uint8_t FlashReadPage(uint32_t PageAddr, uint32_t *pBuffer)
{
	uint32_t Address = 0;
	if(PageAddr<FLASH_BASE||PageAddr+FLASH_PAGE_SIZE>FLASH_BASE+FLASH_SIZE)
		return 0;
	if(PageAddr%FLASH_PAGE_SIZE!=0)
		return 0;
	Address = PageAddr;
	while (Address < PageAddr+FLASH_PAGE_SIZE)
	{
		*pBuffer = *(__IO uint32_t *)Address;
		Address = Address + 4;
		pBuffer++;
	}
	return 1;
}

/*
 *	FlashWritePage:		写1页数据
 *	参数PageAddr：		页地址
 *	参数pBuffer：		用于写入Flash中的数据指针
 *	返回值：			1成功 0失败	
 */
uint8_t FlashWritePage( uint32_t PageAddr, uint32_t *pPageBuffer)
{
	uint32_t Address = 0, PAGEError =0;
	if(PageAddr<FLASH_BASE||PageAddr+FLASH_PAGE_SIZE>FLASH_BASE+FLASH_SIZE)
		return 0;
	
	if(PageAddr%FLASH_PAGE_SIZE!=0)
		return 0;
	//解锁
    //__disable_irq();//关总中断
	HAL_FLASH_Unlock();
	FLASH_EraseInitTypeDef EraseInitStruct;
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = PageAddr;
	EraseInitStruct.NbPages     = 1;
//	printf("PageAddr:%x\r\n",PageAddr);
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	{
		printf("ERASE FLASH ERROR: %x\r\n",HAL_FLASH_GetError());
		HAL_FLASH_Lock();
		return 0;
	}
	
	Address=PageAddr;
	while (Address < PageAddr+FLASH_PAGE_SIZE)
	{
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, *pPageBuffer) == HAL_OK)
		{
			Address = Address + 4;
			pPageBuffer++;
		}
		else
		{
			printf("Write Flash Error\r\n");
			HAL_FLASH_Lock();
			return 0;
		}
	}
	HAL_FLASH_Lock();
    //__enable_irq();//开总中断
	return 1;
}
/*
 *	FlashWrite32:		写4字节(32位)数据,写的数据不能跨页(目前的需求用不到跨页写数据)
 *	参数WriteAddr：		该数据在Flash中的地址
 *	pBuffer:			用于写入Flash中的数据指针
 *	NumToWrite			数据长度(小于页大小/4)
 *	返回值：			1成功 0失败			
 */
uint8_t FlashWrite32( uint32_t WriteAddr, uint32_t * pBuffer, uint16_t NumToWrite )
{
	uint32_t FLASH_BUF [ FLASH_PAGE_SIZE/4 ]={0};
	uint32_t PageAdd;	//WriteAddr对应的页地址
	uint32_t OffSet;	//页地址的偏移  PageAdd+OffSet=WriteAddr
	uint16_t i=0;
	//获取WriteAddr对应的页地址
	if(WriteAddr<FLASH_BASE||WriteAddr+NumToWrite*4>FLASH_BASE+FLASH_SIZE)
		return 0;
		
	if(WriteAddr%4!=0)
		return 0;
		
	if(NumToWrite>FLASH_PAGE_SIZE/4)
		return 0;
	PageAdd=WriteAddr & 0xffffff00;		//L072的页大小为128(0x80),后两位清零即可
	OffSet=WriteAddr & 0x000000ff;
	if(OffSet/FLASH_PAGE_SIZE>0)//
	{
		PageAdd+=FLASH_PAGE_SIZE;
		OffSet-=FLASH_PAGE_SIZE;
	}
	if(PageAdd+FLASH_PAGE_SIZE < WriteAddr+NumToWrite*4)//数据跨页
		return 0;
	if(FlashReadPage(PageAdd,FLASH_BUF)!=1)	//先读一页，避免误擦除
		return 0;
	//修改旧数据
	
	for(i=0;i<NumToWrite;i++)
	{
		FLASH_BUF[OffSet/4+i]=*pBuffer;
		pBuffer++;
	}
	return FlashWritePage(PageAdd,FLASH_BUF);	//擦除旧数据、写入新数据
}
/*
 *	FlashWrite16:		写2字节(16位)数据,写的数据不能跨页(目前的需求用不到跨页写数据)
 *	参数WriteAddr：		该数据在Flash中的地址
 *	pBuffer:			用于写入Flash中的数据指针
 *	NumToWrite			数据长度(小于页大小)
 *	返回值：			1成功 0失败			
 */
uint8_t FlashWrite16( uint32_t WriteAddr, uint16_t * pBuffer, uint16_t NumToWrite )
{
	uint32_t FLASH_BUF [ FLASH_PAGE_SIZE/4 ];
	uint32_t PageAdd;	//WriteAddr对应的页地址
	uint32_t OffSet;	//页地址的偏移  PageAdd+OffSet=WriteAddr
//	uint16_t i;
	
	//获取WriteAddr对应的页地址
	if(WriteAddr<FLASH_BASE||WriteAddr+NumToWrite*2>FLASH_SIZE+FLASH_BASE)
		return 0;
		
	if(WriteAddr%2!=0)
		return 0;
		
	if(NumToWrite>FLASH_PAGE_SIZE/4*2)
		return 0;
	
	PageAdd=WriteAddr & 0xffffff00;
	OffSet=WriteAddr & 0x000000ff;
	if(OffSet/FLASH_PAGE_SIZE>0)//
	{
		PageAdd+=FLASH_PAGE_SIZE;
		OffSet-=FLASH_PAGE_SIZE;
	}
	if(PageAdd+FLASH_PAGE_SIZE < WriteAddr+NumToWrite*2)//数据跨页
		return 0;
	if(FlashReadPage(PageAdd,FLASH_BUF)!=1)	//先读一页，避免误擦除
		return 0;
	//修改旧数据
	memcpy(FLASH_BUF+OffSet/4,pBuffer,NumToWrite*2);
//	for(i=0;i<NumToWrite/2;i++)
//	{
//		uint32_t temp1,temp2;
//		//将16位数据拼接为32位
//		temp1=*pBuffer;
//		temp2=*(pBuffer+1);
//		FLASH_BUF[OffSet/4+i]=(temp2<<16|temp1);
//		pBuffer+=2;
//	}
//	for(uint8_t j=0;j<NumToWrite%2;j++)
//	{
//		FLASH_BUF[OffSet/4+i]=FLASH_BUF[OffSet/4+i] & 0xffff0000;	//清空后2字节
//		FLASH_BUF[OffSet/4+i]=FLASH_BUF[OffSet/4+i] | *pBuffer;
//	}
	if(FlashWritePage(PageAdd,FLASH_BUF)!=1)	//擦除旧数据、写入新数据
		return 0;
	return 1;
}
/*
 *	FlashRead32:		读取4字节(32位)数据
 *	参数ReadAddr：		该数据在Flash中的地址
 *	返回值：			返回读取到的数据		
 */
uint32_t FlashRead32(uint32_t ReadAddr )
{
	if(ReadAddr<FLASH_BASE||ReadAddr>FLASH_BASE+FLASH_SIZE)
		return 0;
	return (uint32_t)(*(__IO uint32_t *)ReadAddr);
}
/*
 *	FlashRead16:		读取2字节(16位)数据
 *	参数ReadAddr：		该数据在Flash中的地址
 *	返回值：			返回读取到的数据		
 */
uint16_t FlashRead16(uint32_t ReadAddr )
{
	if(ReadAddr<FLASH_BASE||ReadAddr>FLASH_BASE+FLASH_SIZE)
		return 0;
	return (uint16_t)(*(__IO uint16_t *)ReadAddr);
}
/*
 *	FlashRead8:		读取1字节(8位)数据
 *	参数ReadAddr：	该数据在Flash中的地址
 *	返回值：		返回读取到的数据		
 */
uint8_t FlashRead8(uint32_t ReadAddr )
{
	if(ReadAddr<FLASH_BASE||ReadAddr>FLASH_BASE+FLASH_SIZE)
		return 0;
	return (uint8_t)(*(__IO uint8_t *)ReadAddr);
}
/*
 *	FlashReadChar:	读取字节(8位)数据,通常用于读取保存在Flash的字符串
 *	参数ReadAddr：	该数据在Flash中的地址
 *	参数pBuffer：	保存读取到的数据的指针
 *	参数NumToRead：	读取的长度
 *	返回值：		实际读取到的字符长度,读取错误返回0
 */
uint16_t FlashReadChar(uint32_t ReadAddr,char* pBuffer,uint16_t NumToRead)
{
	uint16_t i;
	for(i=0;i<NumToRead;i++)
	{
		*pBuffer=FlashRead8(ReadAddr);
		if(*pBuffer=='\0')
			return i;
		pBuffer++;
		ReadAddr++;
	}
	return 0;
}
/*
 *	FlashRead16More:	读取多个2字节(16位)数据
 *	参数ReadAddr：	该数据在Flash中的地址
 *	参数pBuffer：	保存读取到的数据的指针
 *	参数NumToRead：	读取的长度
 *	返回值：		1成功 0失败
 */
uint16_t FlashRead16More(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead) 
{
	if(ReadAddr<FLASH_BASE||ReadAddr+NumToRead*2>FLASH_BASE+FLASH_SIZE)
		return 0;
	uint16_t i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=FlashRead16(ReadAddr);//读取2个字节.
		ReadAddr+=2;//偏移2个字节.	
	}
	return 1;
}
/*
 *	FlashIncrease32:	将Flash中以32位储存的计数器自增1
 *	参数：				该计数器在Flash中的地址
 *	返回值：			1--成功 0--失败				
 */
uint8_t FlashIncrease32(uint32_t add)
{
	uint32_t value;
	value=FlashRead32(add);
	value++;
	return FlashWrite32(add,&value,1);
}
/*
 *	FlashIncrease16:	将Flash中以16位储存的计数器自增1
 *	参数：				该计数器在Flash中的地址
 *	返回值：			1--成功 0--失败				
 */
uint8_t FlashIncrease16(uint32_t add)
{
	uint16_t value;
	value=FlashRead16(add);
	value++;
	return FlashWrite16(add,&value,1);
}
/*
 *	SetFlag:			修改FLAG_ADD地址的值，设置相关标志位
 *	参数flag：			只能为FlagType类型
 *	返回值：			无	
 */
void SetFlag(FlagType flag)
{
	uint32_t temp = (0x000000001U<<(uint8_t)flag);
	temp=FLAG|temp;
	//printf("Set Flag before:%x\r\n",FLAG);
	FlashWrite32(FLAG_ADD,&temp,1);
	//printf("Flag after Set:%x\r\n",FLAG);
}
/*
 *	CleanFlag:			修改FLAG_ADD地址的值，清除相关标志位
 *	参数flag：			只能为FlagType类型
 *	返回值：			无	
 */
void CleanFlag(FlagType flag)
{
	uint32_t temp = (0x000000001U<<(uint8_t)flag);
	temp=FLAG&(~temp);
	//printf("Clean Flag before:%x\r\n",FLAG);
	FlashWrite32(FLAG_ADD,&temp,1);
	//printf("Flag after Clean:%x\r\n",FLAG);
}
/*
 *	CheckFlag:			查询相关标志位是否被标志
 *	参数flag：			只能为FlagType类型
 *	返回值：			被标志返回>0的值，未标志返回0
 */
uint8_t CheckFlag(FlagType flag)
{
	uint32_t temp = (0x000000001U<<(uint8_t)flag);
	return FLAG&temp;
}

