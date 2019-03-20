#include "flash.h"
#include "string.h"
#include "stdio.h"
#include "NBI_common.h"
#include "debug.h"
/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
#if STM32_FLASH_SIZE < 256
  #define STM_SECTOR_SIZE  128 //字节
#else 
  #define STM_SECTOR_SIZE	 2048
#endif


/* 私有变量 ------------------------------------------------------------------*/
#if STM32_FLASH_WREN	//如果使能了写 
static FLASH_TYPE STMFLASH_BUF [ STM_SECTOR_SIZE / FLASH_ALINE ];//最多是2K字节
#endif

/* 扩展变量 ------------------------------------------------------------------*/
/* 私有函数原形 --------------------------------------------------------------*/
/* 函数体 --------------------------------------------------------------------*/

/**
  * 函数功能: 读取指定地址的半字(16位数据)
  * 输入参数: faddr:读地址(此地址必须为2的倍数!!)
  * 返 回 值: 返回值:对应数据.
  * 说    明：无
  */
FLASH_TYPE memCTOI(char *data) {FLASH_TYPE ppp; memcpy(&ppp,data,FLASH_ALINE); return ppp;}	
HAL_StatusTypeDef STMFLASH_WriteStruct(uint32_t WriteAddr, void * pBuffer, uint16_t NumToWrite)
{
	int i = 0;
	uint32_t numData = 0;
	FLASH_TYPE Buffer[STM_SECTOR_SIZE / FLASH_ALINE];	
	for(i = 0; i < NumToWrite/FLASH_ALINE ;i ++)
	{
		Buffer[i] = memCTOI((char *)pBuffer);//*data;	
		pBuffer = ((char *)pBuffer+FLASH_ALINE);
		numData ++;		
	}	
		
	return STMFLASH_Write(WriteAddr,Buffer,numData);	
}	

void STMFLASH_ReadStruct(uint32_t ReadAddr, void * pBuffer, uint16_t NumToRead)
{
	FLASH_TYPE Buffer[STM_SECTOR_SIZE/FLASH_ALINE] = {0};
	#if FLASH_ALINE == 2
		STMFLASH_Read(ReadAddr,Buffer,NumToRead/FLASH_ALINE); //一次两个字节
	#else
		STMFLASH_Read_AllWord(ReadAddr,Buffer,NumToRead/FLASH_ALINE); //一次两个字节
	#endif
	memcpy(pBuffer,Buffer,NumToRead);
}	

HAL_StatusTypeDef STMFLASH_writeString(uint32_t WriteAddr, void * pBuffer, uint16_t NumToWrite)
{
	int i = 0;
	uint32_t numData = 0;
	FLASH_TYPE Buffer[STM_SECTOR_SIZE / FLASH_ALINE];	
//	DEBUG_ERROR("define\r\n");
//	HAL_Delay(1000);
	for(i = 0; i < NumToWrite/FLASH_ALINE ;i ++)
	{
		Buffer[i] = memCTOI((char *)pBuffer);//*data; 
		pBuffer = ((char *)pBuffer+FLASH_ALINE);
		numData ++; 
//		DEBUG_ERROR("is OK\r\n");
//		HAL_Delay(1000);		
	} 
//	DEBUG_ERROR("had finish\r\n");
//	HAL_Delay(2000);
	return STMFLASH_Write(WriteAddr,Buffer,numData);	
} 

void STMFLASH_ReadString(uint32_t ReadAddr, char* pBuffer, uint16_t NumToRead)
{
	FLASH_TYPE Buffer[STM_SECTOR_SIZE/FLASH_ALINE] = {0};
	#if FLASH_ALINE == 2
		STMFLASH_Read(ReadAddr,Buffer,NumToRead/FLASH_ALINE); //一次两个字节
	#else
		STMFLASH_Read_AllWord(ReadAddr,Buffer,NumToRead/FLASH_ALINE); //一次四个字节
	#endif
	memcpy(pBuffer,Buffer,NumToRead);

}	
/**
  * 函数功能: 读取指定地址的半字(32位数据)
  * 输入参数: faddr:读地址(此地址必须为4的倍数!!)
  * 返 回 值: 返回值:对应数据.
  * 说    明：无
  */
uint16_t STMFLASH_ReadHalfWord ( uint16_t faddr )
{
	return *(__IO uint16_t*)faddr;
}

uint32_t STMFLASH_ReadAllWord ( uint32_t faddr )
{
	return *(__IO uint32_t*)faddr;
}
#if STM32_FLASH_WREN	//如果使能了写   
/**
  * 函数功能: 不检查的写入
  * 输入参数: WriteAddr:起始地址
  *           pBuffer:数据指针
  *           NumToWrite:半字(16位)数
  * 返 回 值: 无
  * 说    明：无
  */
HAL_StatusTypeDef STMFLASH_Write_NoCheck ( uint32_t WriteAddr, FLASH_TYPE * pBuffer, uint32_t NumToWrite )   
{ 			 		 
	uint32_t i;	
	
	for(i=0;i<NumToWrite;i++)
	{		
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAMDATA_WORD ,WriteAddr,pBuffer[i]) == HAL_ERROR)
		{
			return HAL_ERROR;
		}
		WriteAddr+=FLASH_ALINE;                                    //地址增加2.
	}  
	return HAL_OK;
} 

/**
  * 函数功能: 从指定地址开始写入指定长度的数据
  * 输入参数: WriteAddr:起始地址(此地址必须为2的倍数!!)
  *           pBuffer:数据指针
  *           NumToWrite:半字(16位)数(就是要写入的16位数据的个数.)
  * 返 回 值: 无
  * 说    明：无
  */
HAL_StatusTypeDef STMFLASH_Write ( uint32_t WriteAddr, FLASH_TYPE * pBuffer, uint32_t NumToWrite )	
{	
//	uint32_t SECTORError = 0;
	uint32_t secoff;	   //扇区内偏移地址(16位字计算)
	uint32_t secremain; //扇区内剩余地址(16位字计算)	   
	uint32_t i;    
	uint32_t secpos;	   //扇区地址
	uint32_t offaddr;   //去掉0X08000000后的地址
	
	if(WriteAddr<FLASH_BASE||(WriteAddr>=(FLASH_BASE+1024*STM32_FLASH_SIZE)))
		return HAL_ERROR;
	
	HAL_FLASH_Unlock();						//解锁	
	//FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
	offaddr=WriteAddr-FLASH_BASE;		//实际偏移地址.
	secpos=offaddr/STM_SECTOR_SIZE;			//扇区地址  0~127 for STM32F103RBT6
	secoff=(offaddr%STM_SECTOR_SIZE)/FLASH_ALINE;		//在扇区内的偏移(2个字节为基本单位.)
	secremain=STM_SECTOR_SIZE/FLASH_ALINE-secoff;		//扇区剩余空间大小   
	if(NumToWrite<=secremain)
		secremain=NumToWrite;//不大于该扇区范围	
	while(1)
	{
		#if FLASH_ALINE == 2		
			STMFLASH_Read(secpos*STM_SECTOR_SIZE+FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//读出整个扇区的内容
//			printf("read data for half word\r\n");
			HAL_Delay(2000);
		#else
			STMFLASH_Read_AllWord(secpos*STM_SECTOR_SIZE+FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/4);//读出整个扇区的内容
		#endif
		for(i=0;i<secremain;i++)//校验数据
		{
			#if FLASH_ALINE == 2
				if(STMFLASH_BUF[secoff+i]!=0XFFFF)
			#else
				if(STMFLASH_BUF[secoff+i]!=0XFFFFFFFF)
			#endif
				break;
		}
		if(i<secremain)//需要擦除
		{
			/* Fill EraseInit structure*/
			uint32_t Sector   = secpos*STM_SECTOR_SIZE+FLASH_BASE;	
			FLASH_EraseInitTypeDef f;
			f.NbPages = (int)(NumToWrite / STM_SECTOR_SIZE) + 1;
			f.PageAddress = Sector;
			f.TypeErase = FLASH_TYPEERASE_PAGES;
			
			uint32_t PageError = 0;
//			DEBUG_ERROR("f.NbPages = %d\r\n",f.NbPages);
//			DEBUG_ERROR("f.PageAddress = %d\r\n",Sector);
			
			if(HAL_FLASHEx_Erase(&f, &PageError) == HAL_ERROR)
			{
				HAL_Delay(500);
				if(HAL_FLASHEx_Erase(&f, &PageError) == HAL_ERROR)
				{
					DEBUG_ERROR("erase error  = %X \r\n",PageError);
					return HAL_ERROR;
				}

			}		
			for(i=0;i<secremain;i++)
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}			
			if(STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/FLASH_ALINE) == HAL_ERROR)
			{
				DEBUG_ERROR("Write_NoCheck error\r\n");
				return HAL_ERROR;
			}
		}
		else
		{
			if(STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain) == HAL_ERROR)
			{
				DEBUG_ERROR("Write_NoCheck error\r\n");
				return HAL_ERROR;
			}
		}
		if(NumToWrite==secremain)
			break;//写入结束了
		else//写入未结束
		{
			secpos++;				//扇区地址增1
			secoff=0;				//偏移位置为0 	 
			pBuffer+=secremain;  	//指针偏移
			WriteAddr+=secremain;	//写地址偏移	   
			NumToWrite-=secremain;	//字节(16位)数递减
			if(NumToWrite>(STM_SECTOR_SIZE/FLASH_ALINE))
				secremain=STM_SECTOR_SIZE/FLASH_ALINE;//下一个扇区还是写不完
			else 
				secremain=NumToWrite;//下一个扇区可以写完了
		}	 
	}
	
	if(HAL_FLASH_Lock() == HAL_ERROR)
	{
		DEBUG_ERROR("HAL_FLASH_Lock error\r\n");
		return HAL_ERROR;
	}
	return HAL_OK;
}
#endif

/**
  * 函数功能: 从指定地址开始读出指定长度的数据
  * 输入参数: ReadAddr:起始地址
  *           pBuffer:数据指针
  *           NumToRead:半字(16位)数
  * 返 回 值: 无
  * 说    明：无
  */
void STMFLASH_Read ( uint32_t ReadAddr, uint16_t *pBuffer, uint32_t NumToRead )
{
	uint16_t i;
	
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i] = STMFLASH_ReadHalfWord(ReadAddr);//读取4个字节.
		ReadAddr += 2;//偏移2个字节.	
	}
}


void STMFLASH_Read_AllWord( uint32_t ReadAddr, uint32_t *pBuffer, uint32_t NumToRead )   	
{
	uint16_t i;
	
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i] = STMFLASH_ReadAllWord(ReadAddr);//读取4个字节.
		ReadAddr += 4;//偏移4个字节.	
	}
}

/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/

