/*
**************************************************************************************************************
*	@file	iap_protocol.c
*	@author 
*	@version 
*	@date    
*	@brief	在线升级In-Application-Programming协议
***************************************************************************************************************
*/
#include "iap_protocol.h"
#include "user_sim.h"

// IAP结构体
static HexObjFile_t sg_hex_obj_file = {0};

#define IAP_BUFF_THRESHOLD	10240	// 接收阈值，当接收长度大于该阈值且收到换行标识后，开始处理
#define IAP_BUFF_LENGTH		(IAP_BUFF_THRESHOLD+64) // 接收缓冲区长度
static char sg_iap_buff[IAP_BUFF_LENGTH] = {0};	 // IAP缓冲区
static uint32_t sg_iap_buff_pt = 0;		// 缓冲区内指针，即当前接收到的数据长度
static uint32_t sg_iap_total_len = 0;	// 已经准备好的缓冲区的总长度
volatile uint32_t sg_iap_buff_ready = 0;	// 当前是否有已经准备好的缓冲区


/*
 *	InitGprsIAP:	初始化GPRS在线编程
 *	参数：			无
 *	返回值：		无	
 */

//#ifdef VECT_TAB_SRAM
//  SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM. */
//#else
//  SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH. */
//#endif
 
extern void InitGprsIAP(void)
{
	InitHexObjFile(&sg_hex_obj_file);

	sg_iap_buff_pt = 0;
	sg_iap_buff_ready = 0;
	sg_iap_buff_ready = 0;
}

/*
 *	UpdateProgramFromGprs:	通过GPRS在线升级程序
 *	参数：					无
 *	返回值：				IAP_SUCCESS		--成功
 *							IAP_FAILURE		--失败
 *							IAP_TIMEOUT		--超时
 */
extern int32_t UpdateProgramFromGprs(void)
{
	uint16_t Count=0;
	uint32_t start_time = HAL_GetTick();
	// 等待IAP结束，或者2min超时
	HAL_FLASH_Unlock();	//解锁
	while (sg_hex_obj_file.status == IAP_CONTINUE && HAL_GetTick()<start_time+3600000)
	{
		uint32_t total_len, data_pt;
		char *buff;
        sg_iap_buff_pt=0;
        memset(sg_iap_buff,0,sizeof(sg_iap_buff));
        SimHttpSetBreak(Count*IAP_BUFF_THRESHOLD);
        SimHttpSetBreakEnd((Count+1)*IAP_BUFF_THRESHOLD+HEX_DATA_LEN-3);//hex文件一行最多45个字符(含\r\n)，加42是防止数据断层，同时也防止多出一层
        SimHttpGet();
        if(SimHttpReadData()==0)
        {   
            printf("数据获取失败,请检测url是否正确\r\n");
            return IAP_FAILURE;
        }
		while (sg_iap_buff_ready == 0);//等待数据获取完成
        HAL_Delay(100);//等待多余的数据输出出来，防止烧录的时候出现意外
		printf("%s",sg_iap_buff);
		// 缓冲区准备完毕
		sg_iap_buff_ready = 0;
		data_pt   = 0;
		total_len = sg_iap_total_len;
		buff = (char *)sg_iap_buff;
		
//		printf("%d,%d\r\n",Count,total_len);//分割一次缓冲区数据
		Count++;
		// 处理本次缓冲区的数据
		while (data_pt < total_len)
		{
			// 处理一行数据
			ParseHexObjFile(&sg_hex_obj_file, buff + data_pt);
			if (sg_hex_obj_file.status == IAP_FAILURE)
			{
			    printf("数据下载错误,HEX文件出问题或者下载丢失数据\r\n");
				return IAP_FAILURE;
			}
			// 寻找下一行
			while(buff[data_pt++]!='\n' && data_pt<total_len);
		}			
	}
	HAL_FLASH_Lock();//上锁
	HAL_Delay(5);
	if(HAL_GetTick()>start_time+300*1000)
	{
		printf("连接超时,更新失败,查看下载网址是否正确\r\n");
		return IAP_TIMEOUT;
	}else{
		printf("更新结束\r\n");
	}
	return sg_hex_obj_file.status;
}

//此处将代码放到RAM中运行，但由于没有修改.sct文件，实际上并没有在RAM中运行
#pragma arm section code = "RAMCODE"
/*
 *	HandleIAPData:	处理IAP数据
 *	p：				命令缓冲区指针
 *	len:			数据长度
 *	返回值：		无
 */
extern void HandleIAPData(char *p, uint32_t len)
{
	char *iap_buff;
	uint32_t new_length;
	// hex文件一行最长就50字节左右，所以不可能一下超过缓冲区长度
	new_length = len + sg_iap_buff_pt;
    
    if(*p!=':')//不接收断层数据
        return;
    if (new_length > IAP_BUFF_LENGTH)
	{
		return;
	}
    if(sg_iap_buff_ready==1)//多余的数据不接收
    {
        return ;
    }

	// 保存数据
	iap_buff = (char *)sg_iap_buff;
	memcpy(iap_buff + sg_iap_buff_pt, p, len);
	
	sg_iap_buff_pt = new_length;

	
	// 接收到的数据超过阈值，或者接收到结束标志，则置位缓冲区标记
	if (new_length > IAP_BUFF_THRESHOLD || strncmp(iap_buff+sg_iap_buff_pt-13, ":00000001FF", 11)==0)
	{
//		if(new_length > IAP_BUFF_THRESHOLD)
//		{
//			printf("recieve overflow\r\n");
//		if(strncmp(iap_buff+sg_iap_buff_pt-13, ":00000001FF", 11)==0)
//			printf("recieve all \r\n");
//		}
        //printf("接收完一波数据\r\n");
		sg_iap_total_len=sg_iap_buff_pt;
		sg_iap_buff_pt = 0;
		sg_iap_buff_ready = 1;
	}
}
#pragma arm section


/*
 *	GetIAPStatus:	获取IAP状态
 *	返回值：		IAP状态
 *					IAP_FAILURE    --IAP失败
 *					IAP_CONTINUE   --IAP进行中
 *					IAP_SUCCESS	   --IAP成功结束
 * 					IAP_UNKNOWN_CMD--非IAP命令
 */
extern int32_t GetIAPStatus(void)
{
	return sg_hex_obj_file.status;
}
///*
// *	GetURL:				将Flash中的URL获取出来，同时将相关参数配置好
// *	data:				下载http文件的命令
// *	server:				URL中的服务器网址
// *	返回值：			1--成功 -1--失败				
// */
//int8_t GetURL(char *data,char *server)
//{	
//	char url[101]={0},*addr;			   
//	FlashReadChar(URL,(char *)url,100); 
//	if(url[0]!=0)
//	{
//		url[100] = 0;
//		addr = strstr(url+7,"/");//过滤掉前面的8个字符: htttp://,找到之后的第一个'/'
//		if(addr>0){
//			strncpy(server,url+7,addr-url-7);
//			sprintf(data,"GET %s HTTP/1.1\r\nHost: %s\r\n\r\n",addr,server);	
//			printf("%s",data);
//			return 1;
//		}else{
//			FlashIncrease32(URL_ERROR);
//			printf("url error (can not found'/'):%s\r\n",url);	//url找不到'/'
//		}	
//	}else{
//		FlashIncrease32(URL_EMPTY);
//		printf(",url error(there isn't url):%s\r\n",url);		//没有url,url为空
//	}
//	FlashIncrease32(URL_FAIL);
//	return -1; 
//}
