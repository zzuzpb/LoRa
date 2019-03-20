#include "NBI_rs485.h"
#include "string.h"
#include "FIFO_Uart.h"
#include "NBI_common.h"
#include "usart.h"
#include "user_mian.h"
#include "gpio.h"
#include "flash.h"

    
stu_NBI_Rs485 g_stu_rs485[] =             
{
/// addr    regAddress  regDatalen   sendDataLen  revDataLen   timeOut  sendBuff   revBuff name
	{0x00 , 0x0000,     0x0000,         7,           9,          1000*1, "EXPEND",  "",    "EXPEND"},	
	//{0x02 , 0x0000, 0x0002, {2,2}, 6 ,9 , 1000*1,"qxz" ,"" ,"qxz"},
	{0x01 , 0x0014,     0x0004,         6,          5+4*2,     1000*1,   "" ,       "",    "threeone"},
	{0x10 , 0x0000, 0x0001,  6 ,7 , 1000*1,"" ,"" ,"PH"},
	{0x11 , 0x0000, 0x0001,  6 ,7 , 1000*1,"" ,"" ,"OXY"},
	{0x12 , 0x0000, 0x0001,  6 ,7 , 1000*1,"" ,"" ,"andan"},
	{0x13 , 0x0000, 0x0002,  6 ,9 , 1000*1,"" ,"" ,"diandao"},
	{0x0C , 0x0000, 0x0001,  6 ,9 , 1000*1,"" ,"" ,"ST-TW"},
	{0x0D , 0x0000, 0x0001,  6 ,7 , 1000*1,"" ,"" ,"ST-EC"},
	{0x02 , 0x0000, 0x0002,  6 ,9 , 1000*1,"" ,"" ,"SWR-100W"},
	{0x07 , 0x0000, 0x0001,  6 ,7 , 1000*1,"" ,"" ,"ST_YMW"},
	{0x08 , 0x0001, 0x0001,  6 ,7 , 1000*1,"" ,"" ,"ST_YMS"},
	{0x05 , 0x0000, 0x0001,  6 ,7 , 1000*1,"" ,"" ,"ST_PH"},
	{0x06 , 0x0000, 0x0001,  6 ,7 , 1000*1,"" ,"" ,"ST_GH"},	
	{0x09 , 0x0000, 0x0001,  6 ,7 , 1000*1,"" ,"" ,"ST_YL"},		
	{0x0A , 0x0000, 0x0001,  6 ,7 , 1000*1,"" ,"" ,"ST_FS"},			
	{0x0B , 0x0000, 0x0001,  6 ,7 , 1000*1,"" ,"" ,"ST_FX"},				
	{0x0F , 0x0000, 0x0001,  6 ,7 , 1000*1,"" ,"" ,"ST_AP"},					
	{0x0E , 0x0000, 0x0001,  6 ,7 , 1000*5,"" ,"" ,"ST_CO2"},						
	{0xFF , 0x0000, 0x0002,  6 ,9 , 1000*1,"" ,"" ,"null"},
};


stu_Rs485_data g_rs485mainportbuff[NBI_RS485_PIN_COUNT];
stu_Rs485_data g_rs485exboxbuff[NBI_RS485_EXBOX_COUNT];

int g_rs485exboxindex = 0;

uint8_t _12VPowerOn(void)
{
	HAL_GPIO_WritePin(Out_12V_ON_Pin_GPIO_Port,Out_12V_ON_Pin_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(POWER_485IC_Port,POWER_485IC_Pin,GPIO_PIN_SET);
	return 1;
}

uint8_t _12VPowerOff(void)
{
	HAL_GPIO_WritePin(Out_12V_ON_Pin_GPIO_Port,Out_12V_ON_Pin_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(POWER_485IC_Port,POWER_485IC_Pin,GPIO_PIN_RESET);
	return 1;
}

static RS485_STATUS RS485Status(RS485_STATUS status)
{
	static RS485_STATUS stc_status = RS485_NONE;
	if(status == RS485_NULL)
	{
		return stc_status;
	}
	stc_status = status;
	return stc_status;
}

static uint16_t CalcCRC16(uint8_t *data, uint8_t len)
{
    uint16_t result = 0xffff;
    uint8_t i, j;

    for (i=0; i<len; i++)
    {
        result ^= data[i];
        for (j=0; j<8; j++)
        {
            if ( result&0x01 )
            {
                result >>= 1;
                result ^= 0xa001;
            }
            else
            {
                result >>= 1;
            }
        }
    }
    GET_CRC(&(data[len]), result);
    return result;
}

unsigned short U2rever(unsigned short data)
{
	return (data >> 8 | ((data &0xff)<<8));
}

unsigned int U4rever(unsigned int data)
{
	unsigned char *buff = (unsigned char *)(&data);
	unsigned int temp = 0;
	for(int i = 0 ; i < 4 ; i ++)
	{
		temp <<= 8;
		temp |= buff[i];
	}
	return temp;
}
unsigned short U2(unsigned char* data)
{
	unsigned short temp;
	memcpy(&temp,data,2);
	temp = U2rever(temp);
	return temp;
}
unsigned int U4(unsigned char* data)
{
	unsigned int temp;
	memcpy(&temp,data,4);
	temp = U4rever(temp);
	return temp;
}


void printRs485(uint8_t *buff,int len)
{
	printf("\r\n");
	for(int i = 0 ; i < len ; i++)
	{
		printf("%02X ",buff[i]);
	}
	printf("\r\n");	
}

static uint8_t g_rs485Revbuff[20];
int Rs485_Cmd(uint8_t *sendData , int len)
{	
    RS485_TO_TX();		 
    CalcCRC16(sendData,len);
    HAL_Delay(200);
//    printf("---send : ");
//    for(int i = 0; i < len+2; i++)
//    printf("%02X ",sendData[i]);
//    printf("\r\n");
    HAL_UART_Transmit(&huart4,sendData,len + 2,0xff);				
    RS485_TO_RX();
    HAL_Delay(NBI_RS485_REV_TIME_OUT);
    memset(g_rs485Revbuff, 0, sizeof(g_rs485Revbuff));
    int length = get_rs485Data(g_rs485Revbuff);
    
    char crcH = g_rs485Revbuff[length-1];
    char crcL = g_rs485Revbuff[length-2];
    
    CalcCRC16(g_rs485Revbuff,length-2);
    if(crcH == g_rs485Revbuff[length-1] && crcL == g_rs485Revbuff[length-2])			
        return length;
    else
        return 0;		
}


/**
  * @brief  扩展盒相应接口状态标志，接入则状态标记HAL_OK,否则标记为HAL_NONE
  * @param  expend_sensor：扩展盒共有几个接口接入传感器，index：PLUS哪个接口接入扩展盒
  * @retval None
  */
void checkExpendBox(int expend_sensor,int index)
{
	//0x4f
	for(int i = 0 ; i < 5 ; i ++)
	{
		if(i == 4)
		{
			if(expend_sensor & (0x01 << 6))
			{
				g_rs485exboxbuff[i+index].expendboxlive = HAL_OK;	
                DEBUG_NORMAL("i+index : %02x",i+index);                
			}
			break;
		}
		if(expend_sensor & (0x01 << i))
		{
            DEBUG_NORMAL("i : %02x, index : %02x",i,index);           
			g_rs485exboxbuff[i+index].expendboxlive = HAL_OK;
            DEBUG_NORMAL("i+index : %02x",i+index);
		}
        
	}	
}

void RS485_PStuforSend(int index,char mode)
{   
    for(int i = 0; i<NBI_RS485_SEND_BUFF_LEN; i++)
    {
        if(g_rs485mainportbuff[index].data->addr == g_stu_rs485[i].addr)
        {
            index = i;
            break;
        }
    }
    DEBUG_NORMAL("index = %d,addr = %02X\r\n",index,g_stu_rs485[index].addr);

	memset(g_stu_rs485[index].sendBuff,0,sizeof(g_stu_rs485[index].sendBuff));
	g_stu_rs485[index].sendBuff[0] = g_stu_rs485[index].addr;
	g_stu_rs485[index].sendBuff[1] = mode;
	g_stu_rs485[index].sendBuff[2] = (g_stu_rs485[index].regAddress & 0xFF00)  >> 8;		
	g_stu_rs485[index].sendBuff[3] = (g_stu_rs485[index].regAddress & 0x00FF);
	g_stu_rs485[index].sendBuff[4] = (g_stu_rs485[index].regDatalen & 0xFF00)  >> 8;				
	g_stu_rs485[index].sendBuff[5] = (g_stu_rs485[index].regDatalen & 0x00FF);
}

void RS485_PackageStuforSend(int index,char mode)
{
	DEBUG_NORMAL("index = %d,addr = %02X\r\n",index,g_stu_rs485[index].addr);
	memset(g_stu_rs485[index].sendBuff,0,sizeof(g_stu_rs485[index].sendBuff));
	g_stu_rs485[index].sendBuff[0] = g_stu_rs485[index].addr;
	g_stu_rs485[index].sendBuff[1] = mode;
	g_stu_rs485[index].sendBuff[2] = (g_stu_rs485[index].regAddress & 0xFF00)  >> 8;		
	g_stu_rs485[index].sendBuff[3] = (g_stu_rs485[index].regAddress & 0x00FF);
	g_stu_rs485[index].sendBuff[4] = (g_stu_rs485[index].regDatalen & 0xFF00)  >> 8;				
	g_stu_rs485[index].sendBuff[5] = (g_stu_rs485[index].regDatalen & 0x00FF);
}

uint8_t openExpendBoxbuff[10] = {0x00,0x05,0x00,0x01,0x00,0x00,0x00};
uint8_t ExpendBoxbuff[9] = {0xFE,0x03,0x04,0x00,0x00,0x00,0x00,0x00,0x00};

int RS485_GetExpendBoxSensorAddress(uint8_t index)
{
	if(RS485Status(g_rs485mainportbuff[index].type) != RS485_EXPAND_BOX) ///非扩展盒保存数据
	{	
		g_flashData.Rs485Index[index] = g_rs485mainportbuff[index].index;
		g_flashData.Rs485type[index] = g_rs485mainportbuff[index].type;
		g_flashData.RS485Flag = HAL_OK;
		if(STMFLASH_WriteStruct(SYS_STU_DATA_ADDRESS,&g_flashData,sizeof(g_flashData)) == HAL_ERROR)
		{
			DEBUG_ERROR("flash write error\r\n");
			HAL_Delay(2000);
			return HAL_ERROR;			
		}
		DEBUG_ERROR("this not expend box");
		return HAL_ERROR;
	}
    
    if(g_rs485mainportbuff[index].type == RS485_EXPAND_BOX)
    {
        int len;
        uint8_t repbuff[20];
        uint8_t exboxid = index + 1;
        ///打开某个主板所接入相应扩展盒6个IO口，记录下标：第一个扩展盒：10 11 12 13 14 15
        ///第二个扩展盒：20 21 22 23 24 25 .... 第六个扩展盒：60 61 62 63 64 65
        for(int Rs485Id = 0 ; Rs485Id < 5 ; Rs485Id ++)
        {
            //sava data to flash
            //更改为外部485模式标记        
//            g_flashData.Rs485Index[g_rs485exboxindex+Rs485Id] = g_rs485exboxbuff[index+Rs485Id].index; ///扩展盒下标

            if(g_rs485exboxbuff[g_rs485exboxindex+Rs485Id].expendboxlive == HAL_OK) ///外部扩展盒接入标志
            {
//                DEBUG_NORMAL("port=%d connect sensor\r\n",Rs485Id);
                DEBUG(2,"g_rs485exboxbuff exbox port=%d connect sensor\r\n",Rs485Id);
                int temp = 0;
                if(Rs485Id == 4)
                {
                    temp = 6;
                }
                else
                {
                    temp = Rs485Id;				
                }
                openExpendBoxbuff[5] = 0x01 << temp;
                len = Rs485_Cmd(openExpendBoxbuff,7);
                if(len != g_stu_rs485[EXPENDBOX].revDataLen)
                {
                    DEBUG(2,"exbox port=%d sensor open faile\r\n",Rs485Id);				
                    g_rs485exboxbuff[index+Rs485Id].status = EXPEND_OPEN_ERROR;
                    continue;
                }
//                DEBUG_NORMAL("port=%d sensor open success\r\n",Rs485Id);
                DEBUG(2,"exbox port=%d sensor open success\r\n",Rs485Id);
                HAL_Delay(200);
                len = Rs485_Cmd(ExpendBoxbuff,7);
                memcpy(repbuff,g_rs485Revbuff,len);
//                DEBUG_NORMAL("port=%d sensor get address\r\n",Rs485Id);
                
                printRs485(repbuff,len);
                if(len > 5)
                {
                    //广播信息被返回了，有数据出来
//                    DEBUG_NORMAL("port=%d sensor return address = %02X\r\n",Rs485Id,repbuff[3]);
                    DEBUG(2,"exbox port=%d sensor return address = %02X\r\n",Rs485Id,repbuff[3]);
                    int j = 0;
                    for(j = 0 ; j < ARRAY(g_stu_rs485) ; j ++)
                    {
                        if(g_stu_rs485[j].addr == repbuff[3])
                        {
                            g_rs485exboxbuff[Rs485Id + g_rs485exboxindex].expendboxlive = HAL_OK;
                            g_rs485exboxbuff[Rs485Id + g_rs485exboxindex].data = &g_stu_rs485[j];		
                            g_rs485exboxbuff[Rs485Id + g_rs485exboxindex].index = (exboxid<<4)|Rs485Id; ///PortId<<4|扩展盒口
                            g_rs485exboxbuff[Rs485Id + g_rs485exboxindex].status = FIND_ADDRESS;
                            g_rs485exboxbuff[Rs485Id + g_rs485exboxindex].timeout = g_stu_rs485[j].timeOut;						
//                            DEBUG_NORMAL("port=%d sensor find ok name = %s\r\n",Rs485Id,g_stu_rs485[j].name);	
                            DEBUG(2, "exbox port=%d sensor find ok name = %s boxid: 0x%02X\r\n",Rs485Id + g_rs485exboxindex,g_stu_rs485[j].name,g_rs485exboxbuff[Rs485Id + g_rs485exboxindex].index);	                            
                            break;
                        }			
                    }
                    if(j == ARRAY(g_stu_rs485))
                    {
                        //没有找到
                        g_rs485exboxbuff[Rs485Id + g_rs485exboxindex].expendboxlive = HAL_ERROR;
                        g_rs485exboxbuff[Rs485Id + g_rs485exboxindex].status = ADDRESS_NOT_FIND;
                        continue;
                    }
                }
                else
                {
                    //广播信息没有返回，认为这个传感器不接受FE查询
                    //遍历
                    DEBUG_ERROR("port=%d sensor don`t return FE statr foreach\r\n",Rs485Id);				

                    int j = 0;
                    for(j = 1 ; j < ARRAY(g_stu_rs485) ; j ++)
                    {
                        RS485_PackageStuforSend(j,NBI_RS485_SEARCH_CODE); ///485 03命令缓存区
                        len = Rs485_Cmd(g_stu_rs485[j].sendBuff,g_stu_rs485[j].sendDataLen);
                        if(len == g_stu_rs485[j].revDataLen)
                        {
                            //找到了
                            g_rs485exboxbuff[Rs485Id+g_rs485exboxindex].expendboxlive = HAL_OK;
                            g_rs485exboxbuff[Rs485Id+g_rs485exboxindex].index = (index<<4)|Rs485Id;
                            g_rs485exboxbuff[Rs485Id+g_rs485exboxindex].data = &g_stu_rs485[j];
                            g_rs485exboxbuff[Rs485Id+g_rs485exboxindex].status = FIND_ADDRESS;
                            DEBUG_NORMAL("port=%d sensor find ok name = %s\r\n",Rs485Id,g_stu_rs485[j].name);							
                            break;
                        }					
                    }
                    if(j == ARRAY(g_stu_rs485))
                    {
                        //没有找到
                        g_rs485exboxbuff[Rs485Id+g_rs485exboxindex].expendboxlive = HAL_ERROR;
                        g_rs485exboxbuff[Rs485Id+g_rs485exboxindex].status = ADDRESS_NOT_FIND;	
                        continue;
                    }
                }
            }//(g_rs485Sendbuff[i].isLive == HAL_OK)
        }//for
        g_flashData.RS485Flag = HAL_OK;
//        if(STMFLASH_WriteStruct(SYS_STU_DATA_ADDRESS,&g_flashData,sizeof(g_flashData)) == HAL_ERROR)
//        {
//            DEBUG_ERROR(" flash write error\r\n");
//            HAL_Delay(2000);
//            return HAL_ERROR;
//        }
        
        g_rs485exboxindex += 5;  ///数组下标计数

        //关闭 扩展盒
        openExpendBoxbuff[5] = 0x00;
        len = Rs485_Cmd(openExpendBoxbuff,7);		
        if(len == 0)
        {
            DEBUG_ERROR("close expend box falie\r\n");		
            return HAL_ERROR;		
        }
        else
        {
            DEBUG_NORMAL("close expend box\r\n");
            return HAL_OK;
        }
    }
    return HAL_ERROR;
}


RS485_STATUS getRs485Status(int PortId)
{
	RS485_STATUS status = RS485_NONE;		
	uint8_t repbuff[20] = {0};	
	uint8_t expend_sensor = 0;
	int len = Rs485_Cmd(ExpendBoxbuff,7);   // 地址广播：get expend return data 	
	DEBUG_NORMAL("len = %d\r\n",len);
	memcpy(repbuff,g_rs485Revbuff,len);
	DEBUG_NORMAL("rs485 ExpendBox get data = ");
	printRs485(repbuff , len);
    ///判断广播回复地址：扩展盒地址应答
	if(len == g_stu_rs485[EXPENDBOX].revDataLen && repbuff[3] == 0) //扩展盒的地址 为0
	{
		//返回的数据长度大于0，认为是外部盒子
		DEBUG_NORMAL("expend box is ok\r\n");
		expend_sensor = repbuff[4];  	//扩展盒哪些口接入传感器
		
		checkExpendBox(expend_sensor,g_rs485exboxindex); ///扩展盒相应接口状态标志
        
        g_rs485mainportbuff[PortId].mainportlive = HAL_OK; ///主485接口存在传感器
        g_rs485mainportbuff[PortId].type = RS485_EXPAND_BOX; ///
          
		DEBUG_NORMAL("sensor get address success PortId : %d\r\n",PortId);
		
		status = RS485_EXPAND_BOX;
	}
	else if(len >5) ///非扩展盒地址应答
	{				
        //没有找到扩展盒但是单个口有了回复
        DEBUG_NORMAL("siganal ok and has rpy addr = %x\r\n",repbuff[3]);		
        g_rs485mainportbuff[PortId].status = ADDRESS_NOT_FIND;
        int i = 0;
    
        ////判断接口对应接入485传感器地址，记录
        for(i = 0 ; i < ARRAY(g_stu_rs485) ; i ++)
        {
            if(g_stu_rs485[i].addr == repbuff[3]) 
            {
                g_rs485mainportbuff[PortId].mainportlive = HAL_OK;
                g_rs485mainportbuff[PortId].data = &g_stu_rs485[i];	

                printf("---g_rs485mainportbuff: ");
        
                for(int i = 0; i < g_rs485mainportbuff[PortId].data->sendDataLen; i++)
                printf("%02x",g_rs485mainportbuff[PortId].data->sendBuff[i]);                
                printf("  \r\n");				
                g_rs485mainportbuff[PortId].index = PortId;
                g_rs485mainportbuff[PortId].status = FIND_ADDRESS;
                g_rs485mainportbuff[PortId].timeout = g_stu_rs485[i].timeOut;
                g_rs485mainportbuff[PortId].type = RS485_SIGNAL;
//                g_rs485index ++; ///计数
                DEBUG_NORMAL("add device ok id = %d\r\n",i);		
                break;
            }
        }
        if(i == ARRAY(g_stu_rs485))
        {
            //没有找到
            status = RS485_NONE;				
            DEBUG_ERROR("address = %d not in array\r\n",repbuff[3]);		
        }
        else
        {
            status = RS485_SIGNAL;
        }
	}
	else
	{
		//没有找到扩展盒，单个口也没有回复，开始遍历所有的外部设备的数据
		DEBUG_WARNING("siganal ok and foreach\r\n");		
		uint32_t startTime = HAL_GetTick();
		g_rs485mainportbuff[PortId].status = ADDRESS_NOT_FIND;
		for(int i = 0 ; i < ARRAY(g_stu_rs485) ; i ++)
		{
			RS485_PackageStuforSend(i,NBI_RS485_SEARCH_CODE);///获取预存485命令缓存
			len = Rs485_Cmd(g_stu_rs485[i].sendBuff,g_stu_rs485[i].sendDataLen);
			while(HAL_GetTick() - startTime < g_stu_rs485[i].timeOut && len != g_stu_rs485[i].revDataLen)
			{
				len = Rs485_Cmd(g_stu_rs485[i].sendBuff,g_stu_rs485[i].sendDataLen);				
				HAL_Delay(500);
			}
			if(len == g_stu_rs485[i].revDataLen)
			{
				//找到了
				DEBUG_NORMAL("device had find:%02x",g_stu_rs485[i].addr);
                g_rs485mainportbuff[PortId].mainportlive = HAL_OK;
				g_rs485mainportbuff[PortId].index = PortId;
				g_rs485mainportbuff[PortId].data = &g_stu_rs485[i];
				g_rs485mainportbuff[PortId].status = FIND_ADDRESS;
				g_rs485mainportbuff[PortId].timeout = g_stu_rs485[i].timeOut;		
				g_rs485mainportbuff[PortId].type = RS485_SIGNAL;
//				g_rs485index ++;
				status = RS485_SIGNAL;
				break;
			}
			else
			{
                DEBUG_NORMAL("device had find g_rs485index:%02x",PortId);
                g_rs485mainportbuff[PortId].mainportlive = HAL_ERROR;
				status = RS485_NONE;
			}
		}		
	}
	return status;
}
int Rs485_getStatus(int index)
{	
	return g_rs485mainportbuff[index].status;
}

/*
 *	_rs485_getdataforIndex:	读取485传感器数据
 *	输入参数：              index：主板接口ID，exindex：扩展盒接口ID
 *  返回值：                无实际意义
 */
HAL_StatusTypeDef _rs485_getdataforIndex(uint8_t index, uint8_t exindex)
{
	int getDataTimeOut = 4*1000;
    int len = 0;
	uint32_t startTime = HAL_GetTick();	
    
    if(g_rs485mainportbuff[index].type == RS485_EXPAND_BOX)
    {
        HAL_Delay(g_rs485exboxbuff[exindex].timeout);
        printf("---index: %d, g_rs485exboxbuff: 0x%02x\r\n",index,g_rs485exboxbuff[exindex].index);

        len = Rs485_Cmd(g_rs485exboxbuff[exindex].data->sendBuff,g_rs485exboxbuff[exindex].data->sendDataLen);
        //等待发送完成,				
        startTime = HAL_GetTick();
        while(HAL_GetTick() - startTime < getDataTimeOut && len != g_rs485exboxbuff[exindex].data->revDataLen)//
        {
            len = Rs485_Cmd(g_rs485exboxbuff[exindex].data->sendBuff,g_rs485exboxbuff[exindex].data->sendDataLen);				
            HAL_Delay(500);
        }
        if(len != g_rs485exboxbuff[exindex].data->revDataLen)
        {
            DEBUG_ERROR("exbox address error %02X\r\n",g_rs485exboxbuff[exindex].data->sendBuff[0]);							
            g_rs485exboxbuff[exindex].status = TIME_OUT;
            return HAL_ERROR;
        }
        memset(g_rs485exboxbuff[exindex].data->revBuff,0,len);
        memcpy(g_rs485exboxbuff[exindex].data->revBuff,g_rs485Revbuff,len);
        HAL_Delay(500);
        DEBUG_NORMAL("exbox id=%x sensor Rs485 revData = ",g_rs485exboxbuff[exindex].data->sendBuff[0]);
        printRs485(g_rs485exboxbuff[exindex].data->revBuff,len);				
        //get data
        for(int j = 0 ; j < g_rs485exboxbuff[exindex].data->regDatalen ; j ++)
        {
            g_rs485exboxbuff[exindex].Rs485Data[j] = 0;
            g_rs485exboxbuff[exindex].Rs485Data[j] = (g_rs485exboxbuff[exindex].data->revBuff[3+2*j]<<8)+g_rs485exboxbuff[exindex].data->revBuff[4+2*j];
            DEBUG_APP("exbox get exbox_485 data[%d] = 0x%04x\r\n",j,g_rs485exboxbuff[exindex].Rs485Data[j]);	
        }
    }
    else
    {
        printf("---index: %d, g_rs485mainportbuff: %d\r\n",index,g_rs485mainportbuff[index].index);
    	RS485_PStuforSend(g_rs485mainportbuff[index].index,NBI_RS485_SEARCH_CODE);
        HAL_Delay(g_rs485mainportbuff[index].timeout);
        
        len = Rs485_Cmd(g_rs485mainportbuff[index].data->sendBuff,g_rs485mainportbuff[index].data->sendDataLen);
        //等待发送完成,				
        startTime = HAL_GetTick();
        while(HAL_GetTick() - startTime < getDataTimeOut && len != g_rs485mainportbuff[index].data->revDataLen)//
        {
            printf("----g_rs485mainportbuff: %d\r\n",g_rs485mainportbuff[index].data->sendBuff[0]);
            len = Rs485_Cmd(g_rs485mainportbuff[index].data->sendBuff,g_rs485mainportbuff[index].data->sendDataLen);				
            HAL_Delay(500);
        }
        if(len != g_rs485mainportbuff[index].data->revDataLen)
        {
            DEBUG_ERROR("address error %02X\r\n",g_rs485mainportbuff[index].data->sendBuff[0]);							
            g_rs485mainportbuff[index].status = TIME_OUT;
            return HAL_ERROR;
        }
        memset(g_rs485mainportbuff[index].data->revBuff,0,len);
        memcpy(g_rs485mainportbuff[index].data->revBuff,g_rs485Revbuff,len);
        HAL_Delay(500);
        DEBUG(2,"id=%x sensor Rs485 revData = ",g_rs485mainportbuff[index].data->sendBuff[0]);
        printRs485(g_rs485mainportbuff[index].data->revBuff,len);				
        DEBUG(2,"\r\n");
        //get data
        for(int j = 0 ; j < g_rs485mainportbuff[index].data->regDatalen ; j ++)
        {
            g_rs485mainportbuff[index].Rs485Data[j] = 0;
            g_rs485mainportbuff[index].Rs485Data[j] = (g_rs485mainportbuff[index].data->revBuff[3+2*j]<<8)+g_rs485mainportbuff[index].data->revBuff[4+2*j];
            DEBUG_APP("get data[%d] = 0x%04x\r\n",j,g_rs485mainportbuff[index].Rs485Data[j]);	
        }
    } 	
	return HAL_OK;
}

HAL_StatusTypeDef RS485_getDataForSensor(uint8_t index, uint8_t exindex)
{
	int getDataTimeOut = 4*1000;
	int len;	
    int temp;
	DEBUG_NORMAL("start get sensor data id = %d\r\n",index);
	if(g_rs485mainportbuff[index].type == RS485_EXPAND_BOX) ///主板485标记口
	{
		DEBUG_NORMAL("id = %d is a expand box",index);
		for(int i = 0 ; i < 5 ;i ++)
		{
			uint32_t startTime = HAL_GetTick();		
			if(g_rs485exboxbuff[i + exindex].expendboxlive == HAL_OK)
			{
				DEBUG_NORMAL("id=%d start get data\r\n",i);
				temp = 0;
				if(i == 4)
				{
					temp = 6;
				}
				else
				{
					temp = i;				
				}
				openExpendBoxbuff[5] = 0x01 << temp;
				len = Rs485_Cmd(openExpendBoxbuff,7);
				startTime = HAL_GetTick();
				while(HAL_GetTick() - startTime < getDataTimeOut && len != g_stu_rs485[EXPENDBOX].revDataLen)
				{
					len = Rs485_Cmd(openExpendBoxbuff,7);				
					HAL_Delay(500);
				}
				if(len == 0)
				{
					DEBUG_ERROR("id=%d open expend faile\r\n",i);				
					g_rs485exboxbuff[i + exindex].status = ADDRESS_NOT_FIND;
					continue;
				}							
				_rs485_getdataforIndex(index, i + exindex);

                openExpendBoxbuff[5] = (0x01 << temp) & 0x00;
                len = Rs485_Cmd(openExpendBoxbuff,7);
                if(len == g_stu_rs485[EXPENDBOX].revDataLen)
                {
                    DEBUG_NORMAL("close expend box success\r\n");
                }
                else
                {
                    DEBUG_ERROR("close expend box faile\r\n");
                }                
			}
			else
			{
				g_rs485exboxbuff[i+exindex].status = NOT_LIVE;	
                openExpendBoxbuff[5] = 0x00;
                len = Rs485_Cmd(openExpendBoxbuff,7);                
			}                    
		}//for
		
	}//EXPAND_BOX
	else if(g_rs485mainportbuff[index].type == RS485_SIGNAL) 
	{
		//signal	
        _rs485_getdataforIndex(index,0);		
	}
	return HAL_OK;
}

HAL_StatusTypeDef RS485SendData(unsigned char *sendData,int sendLen,int revLen)
{
	int len = Rs485_Cmd(sendData, sendLen);
	if(len == revLen)
		return HAL_OK;
	else
		return HAL_ERROR;
}

HAL_StatusTypeDef _rs485_getPinStatus()
{
	get_rs485Data(g_rs485Revbuff);
	for(int id = 0 ; id < NBI_RS485_PIN_COUNT ; id++)
	{
		//遍历plus的6个口，分别是什么
		//打开io口		
		_rs485_openpin(id);
		HAL_Delay(1000);
		//判断是什么口，并且获取其中传感器的地址
		if(getRs485Status(id) != RS485_NONE) 
		{			
            ///扩展盒++5
			RS485_GetExpendBoxSensorAddress(id);
		}		
		else ///485下标保存
		{
//			g_rs485index ++;
            RS485_GetExpendBoxSensorAddress(id);
			printf("pin %d:not find device\r\n,",id);
		}
		
		printf("g_index = %d\r\n",id);
	}
	_rs485_clsoepin();				
	return HAL_OK;
}

int _rs485_getPinStatus_test(void)
{

	//遍历plus的6个口，分别是什么
	//打开io口
	get_rs485Data(g_rs485Revbuff);
	while(1)
	{
		for(int i = 0 ; i < NBI_RS485_PIN_COUNT ; i++)
		{
			_rs485_openpin(i);						
		//判断是什么口，并且获取其中传感器的地址
			HAL_Delay(3000);
			int len = Rs485_Cmd(ExpendBoxbuff,7);
			DEBUG_NORMAL("pin %d:len = %d \r\n",i,len);
			get_rs485Data(g_rs485Revbuff);
		}				
	}
}

HAL_StatusTypeDef Rs485_GetData()
{
	uint8_t exboxindex = 0;
    uint8_t IdIndex = 0;   
    uint8_t IdBuffer[6] = {0};
    
    ///检查哪些主板上共有几个接口接入传感器
    for(uint8_t CheckId = 0; CheckId < NBI_RS485_PIN_COUNT; CheckId++)
    {
        if(g_rs485mainportbuff[CheckId].mainportlive == HAL_OK)
        {
            IdBuffer[IdIndex] = CheckId;
            printf("---- start CheckId :%d----\r\n",CheckId);
            IdIndex++;
        }
    }
    printf("---- start test :%d----\r\n",IdIndex);
    ///只打开主板接入传感器IO 
	for(int i = 0; i < IdIndex; i++)
    {
        printf("IdBuffer[%d]: %d\r\n",i,IdBuffer[i]+1);
        _rs485_openpin(IdBuffer[i]);	
        RS485_getDataForSensor(IdBuffer[i], exboxindex); ///只读取存在ID口设备：主IO、扩展盒IO
        
        if(g_rs485mainportbuff[IdBuffer[i]].type == RS485_EXPAND_BOX)
        exboxindex += 5;
    } 
	return HAL_OK;
}

int InitRs485()
{
	DEBUG_NORMAL("start init rs485");
	HAL_Delay(1000);
	for(int i = 0; i < NBI_RS485_PIN_COUNT ; i++)
	{
//		if(g_flashData.RS485Flag == HAL_OK && g_flashData.Rs485Index[i] != 0)
//		{			
//			printf("%x,%d\r\n",g_stu_rs485[g_flashData.Rs485Index[i]].addr,g_flashData.Rs485type[i]);			
//			g_rs485mainportbuff[i].mainportlive = HAL_OK;
//			g_rs485mainportbuff[i].index = g_flashData.Rs485Index[i];
//			g_rs485mainportbuff[i].timeout = g_stu_rs485[g_flashData.Rs485Index[i]].timeOut;
//			g_rs485mainportbuff[i].data = &g_stu_rs485[g_flashData.Rs485Index[i]];
//			g_rs485mainportbuff[i].status = HAL_OK;
//			g_rs485mainportbuff[i].type = g_flashData.Rs485type[i];
//		}
//		else
//		{
//			printf("%d\r\n",i);			
//			g_rs485mainportbuff[i].mainportlive = HAL_ERROR;
//			g_rs485mainportbuff[i].index = 0x00;
//			g_rs485mainportbuff[i].timeout = 4*1000;
//			g_rs485mainportbuff[i].data = NULL;
//			g_rs485mainportbuff[i].status = HAL_ERROR;
//			g_rs485mainportbuff[i].type = RS485_NULL;
//		}
	}
	if(g_flashData.RS485Flag != HAL_OK)
	{
        _12VPowerOn( );
		_rs485_getPinStatus(); ///获取485接口状态：接扩展盒/单堵接入485/没接入485
	}
	else
	{
		return HAL_OK;
	}
	return HAL_OK;
}

int get_rs485Data(uint8_t *data)
{	
	uint8_t ch;
	int index = 0;
    
    DEBUG(2,"----get_rs485Data----: ");
	while(FIFO_UartReadByte(&usart_rs485,&ch) == HAL_OK)	
	{			
		data[index] = ch;		
		printf("%02X ",data[index]);
		index++;
	}
    DEBUG(2,"\r\n");
	return index;
}


