/*
**************************************************************************************************************
*	@file	parse_hex_file.c
*	@author 
*	@version 
*	@date    
*	@brief	解析hex程序文件，并烧写到FLASH
***************************************************************************************************************
*/
#include "parse_hex_file.h"

// FLASH PAGE的大小
#define PAGE_SIZE		FLASH_PAGE_SIZE
#define PAGE_SIZE_MASK	(PAGE_SIZE-1)

#define RECORD_MARK		':'	// 数据行的起始标识
#define RECTYP_DATA		0	// 记录类型--数据
#define RECTYP_EOF		1	// 记录类型--文件结束
#define RECTYP_ESAR		2	// 记录类型--Extended Segment Address Record
#define RECTYP_SSAR		3	// 记录类型--Start Segment Address Record
#define RECTYP_ELAR		4	// 记录类型--Extended Linear Address Record
#define RECTYP_SLAR		5	// 记录类型--Start Linear Address Record

#define DATATYP_8BIT 	1
#define DATATYP_16BIT	2
#define DATATYP_32BIT	4

// 获取距起始字符':'偏移量为offset的数字
#define GET_DIGITAL(p_mark, offset)	(*((p_mark)+(offset))-'0')

/*
 *	InitHexObjFile:	初始化Hex文件结构体
 *	p：				HexObjFile结构体
 *	返回值：		无	
 */
extern void InitHexObjFile(HexObjFile_t *p)
{
	p->empty_addr = 0;
	p->status = IAP_CONTINUE;

//	/* Unlock the Flash Bank1 Program Erase controller */
//	FLASH_UnlockBank1();
}

/*
 *	CvtHexAsciiToNumber:	初始化Hex文件结构体
 *	str：					字符串地址
 *	type:					转换的类型，DATATYP_8BIT为一个字节，对应两个字符，如"10"，其他的依次类推
 *	data:					转换后的数据
 *	返回值：				0--转换成功  -1--转换失败	
 */
static int8_t CvtHexAsciiToNumber(char *str, uint32_t type, uint32_t *data)
{
	uint8_t i;

	if (type!=DATATYP_8BIT && type!=DATATYP_16BIT && type!=DATATYP_32BIT )
	{
		return -1;
	}

	*data = 0;
	for (i=0; i<type; i++)
	{
		char ch1 = *(str++);
		char ch2 = *(str++);
		(*data) <<= 8;			 

		if (ch1>='0' && ch1<='9')
		{
			(*data) |= (ch1-'0')<<4;
		}
		else if (ch1>='a' && ch1<='f')
		{
			(*data) |= (ch1-'a'+10)<<4;
		}
		else if (ch1>='A' && ch1<='F')
		{
			(*data) |= (ch1-'A'+10)<<4;
		}
		else
		{
			return -1;
		}

		if (ch2>='0' && ch2<='9')
		{
			(*data) |= (ch2-'0');
		}
		else if (ch2>='a' && ch2<='f')
		{
			(*data) |= (ch2-'a'+10);
		}
		else if (ch2>='A' && ch2<='F')
		{
			(*data) |= (ch2-'A'+10);
		}
		else
		{
			return -1;
		}
	}
	return 0;
}

/*
 *	ParseHexObjFile:	解析Hex文件并下载，升级过程不要输出过多调试信息，很容易失败
 *	p_hex：				HexObjFile结构体
 *	str:				字符串行，以'\0'或者'\n'结尾
 *	返回值：			0--成功 -1--失败				
 */
extern int8_t ParseHexObjFile(HexObjFile_t *p_hex, char *str)
{
	char ch;
	volatile uint16_t APP_AREA;
	uint32_t rectyp, len, data, offset,start_addr;
//	uint32_t end_addr, page_start_addr;
	// 寻找起始字符':'
    //printf("%s",str);
	for (ch=*str; ch!='\n'&&ch!='\0'; ch=*(++str))
	{
		if (ch == RECORD_MARK)
		{
			if(p_hex->Start!=1)//IAP没有开始
			{
				if(strncmp(str,":02000004",9)==0)//找到固件开头
				{
					printf("Start IAP\r\n");
					p_hex->Start=1;
					break;
				}
			}
			break;
		}
	}
	
	if(p_hex->Start!=1)//IAP没有开始
	{
//		printf("HTTP HEADER\r\n");
		p_hex->status = IAP_UNKNOWN_CMD;
		return IAP_UNKNOWN_CMD;
	}

	// 找到起始字符，则开始解码

	if (CvtHexAsciiToNumber(str+7, DATATYP_8BIT, &rectyp)==0) 	// 记录类型合法
	{
		switch (rectyp)
		{
			case RECTYP_ELAR:	// 扩展线性地址记录
				
				APP_AREA=FlashRead16(APP_AREA_ADD);
				//printf("APP_AREA:%x,%x\r\n",APP_AREA,APP_AREA_ADD);

				if (CvtHexAsciiToNumber(str+9, DATATYP_16BIT, &p_hex->linear_base_addr) == 0)
				{
                    p_hex->linear_base_addr <<= 16;	 			//0x0800_0000
                    p_hex->linear_base_addr = p_hex->linear_base_addr + 0;
                    p_hex->vct_offset = 0;
                    p_hex->status = IAP_CONTINUE;
				}
                printf("p_hex->linear_base_addr:%x\r\n",p_hex->linear_base_addr);   //0xa000
                printf("p_hex->vct_offset:%x\r\n",p_hex->vct_offset);               //0xa000
				break;
	
	
			case RECTYP_DATA:	// 数据记录
                //printf("数据记录\r\n");
                if(strlen(str)>=HEX_DATA_LEN)
                {
                    if (CvtHexAsciiToNumber(str+3, DATATYP_16BIT, &offset)==0 && CvtHexAsciiToNumber(str+1, DATATYP_8BIT, &len)==0)
                    {
                        HAL_StatusTypeDef FLASHStatus =HAL_OK;
                        //offset是从5000(BOOTLOADER_SIZE)开始
                        start_addr = p_hex->linear_base_addr + offset;	// 烧写的起始地址,0xa000+0x5XXX
                        uint32_t byte_cnt;
                        uint32_t little_endian_data = 0;
                        for (byte_cnt=0; (byte_cnt<len)&&(FLASHStatus==HAL_OK); byte_cnt+=4)
                        {
                            if (CvtHexAsciiToNumber(str+9+(byte_cnt<<1), DATATYP_32BIT, &data) != 0)
                            {
                                break;
                            }
                            // 字节序转换
                            little_endian_data = data>>24 & 0xFF;
                            little_endian_data |= (data>>16 & 0xFF) << 8;
                            little_endian_data |= (data>>8  & 0xFF) << 16;
                            little_endian_data |= (data     & 0xFF) << 24;
                            if((little_endian_data&0xfff00000)==0x08000000)//中断向量表的地址必定为80开头,//中断向量表的地址必定小于分区的最大地址，同时大于分区的基地址
                            {
                                if( ((little_endian_data+ p_hex->vct_offset) < (p_hex->linear_base_addr+AREA_SISE+BOOTLOADER_SIZE)) && ((little_endian_data + p_hex->vct_offset) > p_hex->linear_base_addr+BOOTLOADER_SIZE) )
                                {
                                    //printf("l:%x\r\n",little_endian_data);
                                    little_endian_data = little_endian_data + p_hex->vct_offset;
                                    //printf("l:%x\r\n",little_endian_data);
                                }
                            }
                            //printf("%x:%x\r\n",start_addr+byte_cnt,little_endian_data);
                            FLASHStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, start_addr+byte_cnt, little_endian_data);
                            //HAL_Delay(1);
                            if(FLASHStatus!=HAL_OK)
                            {
                                printf("烧写错误:%x,%x\r\n",p_hex->linear_base_addr,offset);
                                printf("烧写错误:%x,%x,%x\r\n",start_addr+byte_cnt,FLASHStatus,HAL_FLASH_GetError());
                                p_hex->status = IAP_FAILURE;
                            }
                        }
                        if (byte_cnt == len)
                        {
                            p_hex->status = IAP_CONTINUE;
                        }
                    }
                    else
                    {
                        printf("数据转换失败\r\n");
                        p_hex->status = IAP_FAILURE;
                    }
                }
                else
                {
                    printf("忽略断层数据\r\n");
                    p_hex->status = IAP_CONTINUE;
                }
				break;

			case RECTYP_EOF:
				printf("End Of Update\r\n");
				APP_AREA=FlashRead16(APP_AREA_ADD);
//				printf("APP_AREA:%x,%x\r\n",APP_AREA,APP_AREA_ADD);
				if(APP_AREA==(uint16_t)APP_AREA_A)				//当前用户程序在A区运行,则烧写B区,B区烧写完成，把当前运行区域修改为B区
				{
					//printf("Program B:finish\r\n");
					FlashWrite16(APP_AREA_ADD,(uint16_t *)"1",1);	//跳转到B区
					FlashWrite16(AREA_B_RUN,(uint16_t *)"1",1);		//B区可运行
				}
				else if(APP_AREA==(uint16_t)APP_AREA_B)		//当前用户程序在B区运行，则烧写A区,A区烧写完成，把当前运行区域修改为A区
				{
					//printf("Program A:finish\r\n");
					FlashWrite16(APP_AREA_ADD,(uint16_t *)"0",1);	//跳转到A区
					FlashWrite16(AREA_A_RUN,(uint16_t *)"1",1);		//A区可运行
				}
				else	//A、B区都不在，可能是第一次上电，也可能是该数据被破坏
				{
					//printf("Program A:finish(first)\r\n");
					FlashWrite16(APP_AREA_ADD,(uint16_t *)"0",1);	//跳转到A区
					FlashWrite16(AREA_A_RUN,(uint16_t *)"1",1);		//A区可运行
				}

				p_hex->status = IAP_SUCCESS;
				break;
			

//			case RECTYP_ESAR:
//				p_hex->status = IAP_FAILURE;
//				break;
//			case RECTYP_SSAR:
//				p_hex->status = IAP_FAILURE;
//				break;
//			case RECTYP_SLAR:
//				p_hex->status = IAP_CONTINUE;
//				break;

			default:
                p_hex->status = IAP_CONTINUE;
				break;
		}
	}

	return p_hex->status;
}

/*
 *	ShowFlash:			将Flash中的计数器显示出来
 *	参数：				无
 *	返回值：			无				
 */
void ShowFlash()
{
	printf("Application current area(A:0x30 B:0x31):%x\r\n",FlashRead16(APP_AREA_ADD));//当前运行区域
	printf("BOOT_TOTAL:%d\r\n",FlashRead32(BOOT_TOTAL));
	printf("BOOT_CMD_TOTAL:%d\r\n",FlashRead32(BOOT_CMD_TOTAL));
	printf("REBOOT_TOTAL:%d\r\n",FlashRead32(REBOOT_TOTAL));
	printf("BOOT_SUCCEED:%d\r\n\r\n",FlashRead32(BOOT_SUCCEED));

	printf("SEND_SEQ_TOTAL:%d\r\n",FlashRead32(SEND_SEQ_TOTAL));
	printf("SEND_SEQ_SUCCEED:%d\r\n",FlashRead32(SEND_SEQ_SUCCEED));
	printf("RECIEVE_SEQ:%d\r\n",FlashRead32(RECIEVE_SEQ));
	printf("RECIEVE_SEQ_ACK:%d\r\n\r\n",FlashRead32(RECIEVE_SEQ_ACK));

	printf("INIT_SIMCOM_FAIL:%d\r\n",FlashRead32(INIT_SIMCOM_FAIL));
	printf("SIMCOM_CMD_DO_ERR:%d\r\n",FlashRead16(SIMCOM_CMD_DO_ERR));
	printf("SIMCOM_CMD_DO_TMEOUT:%d\r\n",FlashRead16(SIMCOM_CMD_DO_TMEOUT));
	printf("SIMCOM_CMD_BUSY:%d\r\n",FlashRead16(SIMCOM_CMD_BUSY));
	printf("SIMCOM_CMD_SEND_ERR:%d\r\n\r\n",FlashRead16(SIMCOM_CMD_SEND_ERR));

	printf("CONNECT_TCP_FAIL:%d\r\n",FlashRead32(CONNECT_TCP_FAIL));
	printf("CONNECT_TCP_NONE:%d\r\n",FlashRead32(CONNECT_TCP_NONE));
	printf("CONNECT_TCP_FAILURE:%d\r\n",FlashRead32(CONNECT_TCP_FAILURE));
	printf("CONNECT_TCP_EXISTED:%d\r\n\r\n",FlashRead32(CONNECT_TCP_EXISTED));

	printf("SEND_TCP_DATA_FAIL:%d\r\n",FlashRead32(SEND_TCP_DATA_FAIL));
	printf("SEND_TCP_NONE:%d\r\n",FlashRead32(SEND_TCP_NONE));
	printf("SEND_TCP_FAILURE:%d\r\n",FlashRead32(SEND_TCP_FAILURE));
	printf("SEND_TCP_TRANS_FAIL:%d\r\n\r\n",FlashRead32(SEND_TCP_TRANS_FAIL));

	printf("GPRSPROGRAM_FAIL:%d\r\n",FlashRead32(GPRSPROGRAM_FAIL));
	printf("DOWNLOADDATA_ERR:%d\r\n",FlashRead32(DOWNLOADDATA_ERR));
	printf("DOWNLOAD_TIMEOUT:%d\r\n\r\n",FlashRead32(DOWNLOAD_TIMEOUT));

	printf("URL_FAIL:%d\r\n",FlashRead32(URL_FAIL));
	printf("URL_ERROR:%d\r\n",FlashRead32(URL_ERROR));
	printf("URL_EMPTY:%d\r\n",FlashRead32(URL_EMPTY));
	printf("MEMORY_OVERFLOW:%d\r\n\r\n",FlashRead32(MEMORY_OVERFLOW));
    while(1);
}

