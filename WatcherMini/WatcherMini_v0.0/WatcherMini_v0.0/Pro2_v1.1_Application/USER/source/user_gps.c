/*
 * user_gps.c	gps模块驱动文件
*/

#include "user_gps.h"

extern UART_HandleTypeDef huart2;

char GpsReChar[1]={0};					//接收的一个字节
char GpsReBuff[GPS_RECIEVE_COUNT_MAX]={0};	//接收到的数据
char LocationData[GPS_RECIEVE_COUNT_MAX]={0};	//接收到的数据
//char GpsTxBuff[GPS_RECIEVE_COUNT_MAX]={0};	//发送的数据
uint16_t GpsReCount=0;					//接收到的字节数
uint8_t GpsReData;


/*
 *GpsInit: 		获取ADC某通道上的电压值,相对电压采用内部电压
 *参数:			无		
 *返回值:		0失败 1成功
*/
uint8_t  GpsInit(void)
{
	//MX_USART2_UART_Init();
	GpsPowerOn();
	HAL_UART_Receive_IT(&huart2,(uint8_t *)GpsReChar,1);//打开串口接收中断
	return 1;
}

/*
 *GpsPowerOff: 	关闭GPS电源
 *参数:			无		
 *返回值:		0失败 1成功
*/
uint8_t  GpsPowerOff(void)
{
	HAL_GPIO_WritePin(OUT_GPS_Power_On_GPIO_Port,OUT_GPS_Power_On_Pin,GPIO_PIN_RESET);
	return 1;
}

/*
 *GpsPowerOn: 	打开GPS电源
 *参数:			无		
 *返回值:		0失败 1成功
*/
uint8_t  GpsPowerOn(void)
{
    HAL_GPIO_WritePin(OUT_SIM_Power_ON_GPIO_Port, OUT_SIM_Power_ON_Pin, GPIO_PIN_SET);//拓展小板的总电源开关
	HAL_GPIO_WritePin(OUT_GPS_Power_On_GPIO_Port,OUT_GPS_Power_On_Pin,GPIO_PIN_SET);
	return 1;
}


/*
 * GpsCheckReply:	检测Gps模块的回复状态，主要用于修改AtCmdStruct结构体的AtRlplyState
 * 参数:			无
 * 返回值:			无
*/
void GpsCheckReply(void)
{
	if(GpsReCount==GPS_RECIEVE_COUNT_MAX)			//溢出则丢弃之前的数据
		GpsReCount=0;
	GpsReBuff[GpsReCount++]=GpsReChar[0];		//保存接收到的数据
	//DEBUG("%c",GpsReChar[0]);
	
	if ( GpsReBuff[GpsReCount-2]=='\r' && GpsReBuff[GpsReCount-1]=='\n' )	// 收到"\r\n"，有新命令回复
	{
		char *pt=NULL;
		if (GpsReCount>15)//
		{
			pt = strstr(GpsReBuff,"$GPRMC");//只处理GPRMC的信息
			if(pt!=NULL)
			{
				memcpy(LocationData,GpsReBuff,GpsReCount);
			}			
		}
		GpsReCount = 0;
	}
	HAL_StatusTypeDef ret=HAL_UART_Receive_IT(&huart2,(uint8_t *)GpsReChar,1);//重新打开串口接收中断
	if(ret!=HAL_OK)//重新打开串口接收中断
    {
        DEBUG("重启串口总线:%x,%x\r\n",ret,HAL_UART_GetError(&huart2));
        if (HAL_UART_Init(&huart2) != HAL_OK)
        {
            while(1);//独立看门狗会复位
        }
        do
        {
            ret = HAL_UART_Receive_IT(&huart2,(uint8_t *)GpsReChar,1);
        }while(ret!=HAL_OK);//独立看门狗会复位
    }
}
/*
 * SimGpsGetLocation:	获取经纬度(开始定位)
 * Longitude:			经度，传入字符数组的起始地址,函数执行成功后该数组会被修改
 * Latitude:			纬度，传入字符数组的起始地址,函数执行成功后该数组会被修改
 * time:				定位用的时间,超时时间,若定位成功，返回实际的定位时间
 * 返回值:				1成功 0失败 2表示无GPS或者GPS模块错误
*/
uint8_t GpsGetLocation(double *Longitude,double *Latitude,uint32_t *time)
{
	nmea_msg gpsx;
    uint16_t count=0;
	uint32_t StartTime=HAL_GetTick();
    uint32_t StartTime1=HAL_GetTick();
	while(1)
	{
		if(LocationData[0]!=0)
		{
            StartTime1=HAL_GetTick();
			DEBUG("Get Data:%s",LocationData);
			if(strstr(LocationData,",A,")!=NULL)//	“,A,”表示定位成功
			{
				DEBUG("count:%u\r\n",count);
                count++;
                if( count>30 )
                    break;
                memset(LocationData,0,sizeof(LocationData));
			}
			else
			{
				memset(LocationData,0,sizeof(LocationData));
				if(HAL_GetTick()-StartTime>*time)//定位超时
				{
					break;
				}
			}
		}
        else if(HAL_GetTick()-StartTime1>3000)//3秒内没有收到GPS模块的信息回复，认为定位模块错误
		{
			//DEBUG("无GPS模块或GPS模块错误\r\n");
			return 2;
		}
	}
	if(LocationData[0]==0)
	{
		return 0;
	}
	//unsigned char *LocationData="$GPRMC,023543.00,A,2308.28715,N,11322.09875,E,0.195,,240213,,,A*78";
	//unsigned char *LocationData="$GPRMC,024813.640,A,3158.4608,N,11848.3737,E,10.05,324.27,150706,,,A*50";
//	DEBUG("Get Data:%s\r\n",LocationData);
//	GPS_Analysis(&gpsx,(u8*)GpsReBuff);	//分析字符串
//	NMEA_GPGSV_Analysis(&gpsx,LocationData);	//GPGSV解析
//	NMEA_GPGGA_Analysis(&gpsx,LocationData);	//GPGGA解析 	
//	NMEA_GPGSA_Analysis(&gpsx,LocationData);	//GPGSA解析
	NMEA_GPRMC_Analysis(&gpsx,(u8*)LocationData);	//GPRMC解析
//	NMEA_GPVTG_Analysis(&gpsx,LocationData);	//GPVTG解析
	Gps_Msg_Show(gpsx);					//显示信息
	//NMEA_GPRMC_Analysis将经纬度扩大100000倍
	if(gpsx.ewhemi=='W')//西经
		*Longitude=(double)gpsx.longitude/-100000.0;
	else
		*Longitude=(double)gpsx.longitude/100000.0;
		
	if(gpsx.nshemi =='S')//南纬
		*Latitude=(double)gpsx.latitude/-100000.0;
	else
		*Latitude=(double)gpsx.latitude/100000.0;
	*time=HAL_GetTick()-StartTime;

//	*Latitude=-123.456;
//	*Longitude=123.456;
	return 1;
}
void Gps_Msg_Show(nmea_msg gpsx)
{
 	float tp;		   
	char dtbuf[50];
	char * fixmode_tbl[4]={"Fail","Fail"," 2D "," 3D "};	//fix mode字符串 
	tp=gpsx.longitude;	   
	sprintf((char *)dtbuf,"Longitude:%.5f %1c   ",tp/=100000,gpsx.ewhemi);	//得到经度字符串
 	DEBUG("%s\r\n",dtbuf);	 	   
	tp=gpsx.latitude;	   
	sprintf((char *)dtbuf,"Latitude:%.5f %1c   ",tp/=100000,gpsx.nshemi);	//得到纬度字符串
 	DEBUG("%s\r\n",dtbuf);	 	 
	tp=gpsx.altitude;	   
 	sprintf((char *)dtbuf,"Altitude:%.1fm     ",tp/=10);	    			//得到高度字符串
 	DEBUG("%s\r\n",dtbuf);	 			   
	tp=gpsx.speed;	   
 	sprintf((char *)dtbuf,"Speed:%.3fkm/h     ",tp/=1000);		    		//得到速度字符串	 
 	DEBUG("%s\r\n",dtbuf);	 				    
	if(gpsx.fixmode<=3)														//定位状态
	{  
		sprintf((char *)dtbuf,"Fix Mode:%s",fixmode_tbl[gpsx.fixmode]);	
		DEBUG("%s\r\n",dtbuf);			   
	}	 	   
	sprintf((char *)dtbuf,"Valid satellite:%02d",gpsx.posslnum);	 		//用于定位的卫星数
 	DEBUG("%s\r\n",dtbuf);	    
	sprintf((char *)dtbuf,"Visible satellite:%02d",gpsx.svnum%100);	 		//可见卫星数
 	DEBUG("%s\r\n",dtbuf);		 
	sprintf((char *)dtbuf,"UTC Date:%04d/%02d/%02d   ",gpsx.utc.year,gpsx.utc.month,gpsx.utc.date);	//显示UTC日期
	DEBUG("%s\r\n",dtbuf);		    
	sprintf((char *)dtbuf,"UTC Time:%02d:%02d:%02d   ",gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec);	//显示UTC时间
  	DEBUG("%s\r\n",dtbuf);		  
}

//从buf里面得到第cx个逗号所在的位置
//返回值:0~0XFE,代表逗号所在位置的偏移.
//       0XFF,代表不存在第cx个逗号							  
u8 NMEA_Comma_Pos(u8 *buf,u8 cx)
{	 		    
	u8 *p=buf;
	while(cx)
	{		 
		if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//遇到'*'或者非法字符,则不存在第cx个逗号
		if(*buf==',')cx--;
		buf++;
	}
	return buf-p;	 
}
//m^n函数
//返回值:m^n次方.
u32 NMEA_Pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}
//str转换为数字,以','或者'*'结束
//buf:数字存储区
//dx:小数点位数,返回给调用函数
//返回值:转换后的数值
int NMEA_Str2num(u8 *buf,u8*dx)
{
	u8 *p=buf;
	u32 ires=0,fres=0;
	u8 ilen=0,flen=0,i;
	u8 mask=0;
	int res;
	while(1) //得到整数和小数的长度
	{
		if(*p=='-'){mask|=0X02;p++;}//是负数
		if(*p==','||(*p=='*'))break;//遇到结束了
		if(*p=='.'){mask|=0X01;p++;}//遇到小数点了
		else if(*p>'9'||(*p<'0'))	//有非法字符
		{	
			ilen=0;
			flen=0;
			break;
		}	
		if(mask&0X01)flen++;
		else ilen++;
		p++;
	}
	if(mask&0X02)buf++;	//去掉负号
	for(i=0;i<ilen;i++)	//得到整数部分数据
	{  
		ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	}
	if(flen>5)flen=5;	//最多取5位小数
	*dx=flen;	 		//小数点位数
	for(i=0;i<flen;i++)	//得到小数部分数据
	{  
		fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
	} 
	res=ires*NMEA_Pow(10,flen)+fres;
	if(mask&0X02)res=-res;		   
	return res;
}	  							 
//分析GPGSV信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPGSV_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p,*p1,dx;
	u8 len,i,j,slx=0;
	u8 posx;   	 
	p=buf;
	p1=(u8*)strstr((const char *)p,"$GPGSV");
	len=p1[7]-'0';								//得到GPGSV的条数
	posx=NMEA_Comma_Pos(p1,3); 					//得到可见卫星总数
	if(posx!=0XFF)gpsx->svnum=NMEA_Str2num(p1+posx,&dx);
	for(i=0;i<len;i++)
	{	 
		p1=(u8*)strstr((const char *)p,"$GPGSV");  
		for(j=0;j<4;j++)
		{	  
			posx=NMEA_Comma_Pos(p1,4+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].num=NMEA_Str2num(p1+posx,&dx);	//得到卫星编号
			else break; 
			posx=NMEA_Comma_Pos(p1,5+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].eledeg=NMEA_Str2num(p1+posx,&dx);//得到卫星仰角 
			else break;
			posx=NMEA_Comma_Pos(p1,6+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].azideg=NMEA_Str2num(p1+posx,&dx);//得到卫星方位角
			else break; 
			posx=NMEA_Comma_Pos(p1,7+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].sn=NMEA_Str2num(p1+posx,&dx);	//得到卫星信噪比
			else break;
			slx++;	   
		}   
 		p=p1+1;//切换到下一个GPGSV信息
	}   
}
//分析GPGGA信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPGGA_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p1,dx;			 
	u8 posx;    
	p1=(u8*)strstr((const char *)buf,"$GPGGA");
	posx=NMEA_Comma_Pos(p1,6);								//得到GPS状态
	if(posx!=0XFF)gpsx->gpssta=NMEA_Str2num(p1+posx,&dx);	
	posx=NMEA_Comma_Pos(p1,7);								//得到用于定位的卫星数
	if(posx!=0XFF)gpsx->posslnum=NMEA_Str2num(p1+posx,&dx); 
	posx=NMEA_Comma_Pos(p1,9);								//得到海拔高度
	if(posx!=0XFF)gpsx->altitude=NMEA_Str2num(p1+posx,&dx);  
}
//分析GPGSA信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPGSA_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p1,dx;			 
	u8 posx; 
	u8 i;   
	p1=(u8*)strstr((const char *)buf,"$GPGSA");
	posx=NMEA_Comma_Pos(p1,2);								//得到定位类型
	if(posx!=0XFF)gpsx->fixmode=NMEA_Str2num(p1+posx,&dx);	
	for(i=0;i<12;i++)										//得到定位卫星编号
	{
		posx=NMEA_Comma_Pos(p1,3+i);					 
		if(posx!=0XFF)gpsx->possl[i]=NMEA_Str2num(p1+posx,&dx);
		else break; 
	}				  
	posx=NMEA_Comma_Pos(p1,15);								//得到PDOP位置精度因子
	if(posx!=0XFF)gpsx->pdop=NMEA_Str2num(p1+posx,&dx);  
	posx=NMEA_Comma_Pos(p1,16);								//得到HDOP位置精度因子
	if(posx!=0XFF)gpsx->hdop=NMEA_Str2num(p1+posx,&dx);  
	posx=NMEA_Comma_Pos(p1,17);								//得到VDOP位置精度因子
	if(posx!=0XFF)gpsx->vdop=NMEA_Str2num(p1+posx,&dx);  
}
//分析GPRMC信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPRMC_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p1,dx;			 
	u8 posx;     
	u32 temp;	   
	float rs;  
	p1=(u8*)strstr((const char *)buf,"GPRMC");//"$GPRMC",经常有&和GPRMC分开的情况,故只判断GPRMC.
	posx=NMEA_Comma_Pos(p1,1);								//得到UTC时间
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//得到UTC时间,去掉ms
		gpsx->utc.hour=temp/10000;
		gpsx->utc.min=(temp/100)%100;
		gpsx->utc.sec=temp%100;	 	 
	}	
	posx=NMEA_Comma_Pos(p1,3);								//得到纬度
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//得到°
		rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
		gpsx->latitude=gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//转换为° 
	}
	posx=NMEA_Comma_Pos(p1,4);								//南纬还是北纬 
	if(posx!=0XFF)gpsx->nshemi=*(p1+posx);					 
 	posx=NMEA_Comma_Pos(p1,5);								//得到经度
	if(posx!=0XFF)
	{												  
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//得到°
		rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
		gpsx->longitude=gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//转换为° 
	}
	posx=NMEA_Comma_Pos(p1,6);								//东经还是西经
	if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);		 
	posx=NMEA_Comma_Pos(p1,9);								//得到UTC日期
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 				//得到UTC日期
		gpsx->utc.date=temp/10000;
		gpsx->utc.month=(temp/100)%100;
		gpsx->utc.year=2000+temp%100;	 	 
	} 
}
//分析GPVTG信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPVTG_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p1,dx;			 
	u8 posx;    
	p1=(u8*)strstr((const char *)buf,"$GPVTG");							 
	posx=NMEA_Comma_Pos(p1,7);								//得到地面速率
	if(posx!=0XFF)
	{
		gpsx->speed=NMEA_Str2num(p1+posx,&dx);
		if(dx<3)gpsx->speed*=NMEA_Pow(10,3-dx);	 	 		//确保扩大1000倍
	}
}  
//提取NMEA-0183信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void GPS_Analysis(nmea_msg *gpsx,u8 *buf)
{
	NMEA_GPGSV_Analysis(gpsx,buf);	//GPGSV解析
	NMEA_GPGGA_Analysis(gpsx,buf);	//GPGGA解析 	
	NMEA_GPGSA_Analysis(gpsx,buf);	//GPGSA解析
	NMEA_GPRMC_Analysis(gpsx,buf);	//GPRMC解析
	NMEA_GPVTG_Analysis(gpsx,buf);	//GPVTG解析
}

//GPS校验和计算
//buf:数据缓存区首地址
//len:数据长度
//cka,ckb:两个校验结果.
void Ublox_CheckSum(u8 *buf,u16 len,u8* cka,u8*ckb)
{
	u16 i;
	*cka=0;*ckb=0;
	for(i=0;i<len;i++)
	{
		*cka=*cka+buf[i];
		*ckb=*ckb+*cka;
	}
}

