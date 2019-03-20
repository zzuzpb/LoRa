/*
 * user_sensor_pro2.c	pro2设备的传感器驱动文件，包括I2C传感器和RS485传感器，需要用到I2C和串口
*/

#include "user_sensor_pro2.h"


extern I2C_HandleTypeDef hi2c2;
extern RTC_HandleTypeDef hrtc;

/*
 *	SensorInit：			传感器初始化
 *	参数：					无
 *  返回值:					1成功 0失败
 */
uint8_t SensorInit(void)
{
	return 1;
}
/*
 *	SernsorGetData：		获取所有传感器数据,所有数据都保存在sensor_data数组中
 *	参数pt_sensor_data：		指向获取到的数据的初始地址
 *  返回值:					0失败 成功返回1,并把sensor_data的地址保存到参数中
 */
uint8_t SernsorGetData(T_SensorData  *pt_sensor_data)
{
	int16_t sh[2]={0};
    //int32_t sh[2]={0};
	uint32_t lux = 0;
    
    memset(pt_sensor_data, 0, 16);
    
	if(SensorMax44009ReadData(&lux)==0)
	{
		DEBUG("读取光照度失败\r\n");
		pt_sensor_data->count=2;
		pt_sensor_data->data[0]=0;
		pt_sensor_data->data[1]=0;
		//return 0;
	}
	else
	{
		pt_sensor_data->count=2;
		pt_sensor_data->data[0]=(uint16_t)((lux&0xffff0000)>>16);
		pt_sensor_data->data[1]=(uint16_t)(lux&0x0000ffff);
	}
	pt_sensor_data++;
	if(SensorHdc1080ReadData(sh)==0)
	{
		DEBUG("读取温湿度失败\r\n");
		pt_sensor_data->count=2;
		pt_sensor_data->data[0]=0;//温度
		pt_sensor_data->data[1]=0;//湿度
	}
	else
	{
		pt_sensor_data->count=2;
		pt_sensor_data->data[0]=(uint16_t)sh[0];//温度
		pt_sensor_data->data[1]=(uint16_t)sh[1];//湿度       
	}
	return 1;
}
/******************************************************读取空气温湿度***********************************************/
/*
 *	SensorHdc1080ReadData：	读取空气温湿度
 *	参数：					measure，保存温湿度数据，[0]为温度，1为湿度
 *  返回值:					1成功 0失败
 */
uint8_t SensorHdc1080ReadData(int16_t *measure)
{
	uint8_t Temperature = 0x00;
	uint8_t temdata[2]={0};
	float Temperature_Data, Humidity_Data;
	
	if(HAL_I2C_Master_Transmit(&hi2c2,Hdc1080_ADDR, &Temperature,1, 20)!=HAL_OK)
		return 0;
	HAL_Delay(20);
	if(HAL_I2C_Master_Receive(&hi2c2,Hdc1080_ADDR+1,temdata, 2, 20)!= HAL_OK ) /// hi2c->pBuffPtr++; ///做更改：工作在4Mhz，容易出现问题
		return 0;
	Temperature_Data = (temdata[0] << 8 )| temdata[1];
	measure[0] = (((Temperature_Data/ 65536)* 165) - 40)*10;	// 放大10倍解码需要除以10
	//负数测试
//	measure[0] = ((((Temperature_Data/ 65536)* 165) - 40)*10)*-1;	// 放大10倍解码需要除以10
//	DEBUG("Temperature_Data = %02x temdata[1] = %02x \r\n", temdata[0], temdata[1]);
//	DEBUG("Temperature_Data = %02f measure[0] = %d\r\n", Temperature_Data,measure[0]);

//	读取湿度
	uint8_t humdata[2] = {0};
	measure[1] = 0;			//清空数据
	uint8_t  Humidity	= 0x01;
	if(HAL_I2C_Master_Transmit(&hi2c2,Hdc1080_ADDR, &Humidity,1, 20)!=HAL_OK)
		return 0;
	HAL_Delay(20);
	if(HAL_I2C_Master_Receive(&hi2c2,Hdc1080_ADDR+1,humdata,2, 20) != HAL_OK )
		return 0;
	Humidity_Data = (humdata[0] << 8 )| humdata[1];
	measure[1] = ((Humidity_Data/ 65536)*100)*10;	// 放大10倍，方便保存，解码需要除以10
//	DEBUG("Humidity_Data = %02x humdata[1] = %02x \r\n", humdata[0], humdata[1]);
//	DEBUG("Humidity_Data = %02f measure[1] = %d\r\n", Humidity_Data,measure[1]);
	DEBUG("Temperature_Data = %d°C Humidity_Data = %d％RH\r\n", measure[0],measure[1]);
	
	return 1;
}

/******************************************************读取空气温湿度***********************************************/

/******************************************************读取光照度***********************************************/
/*
 *	SensorMax44009ReadData：读取光照度
 *	参数：					lux，保存光照度
 *  返回值:					0失败,成功返回光照度,与参数一样，只是方便有时候函数调用
 */
uint32_t SensorMax44009ReadData(uint32_t *lux)
{
	uint8_t hight,low;
	uint32_t mantissa, exp;
	if(HAL_I2C_Mem_Read(&hi2c2, MAX44009_ADDR, LUX_HIGHT_ADDR, 1, &hight, 1,  1000)!=HAL_OK)
		return 0;
	if(HAL_I2C_Mem_Read(&hi2c2, MAX44009_ADDR, LUX_LOW_ADDR, 1, &low, 1,  1000)!=HAL_OK)
		return 0;

	mantissa = ((hight&0xF)<<4) | (low&0xF);
	exp = (hight&0xF0)>>4;
	if (exp != 0xf)									//不对exp进行有效性判断了，因为测出的数据有mantissa为0 exp不为0的情况
	{
		*lux = (mantissa<<exp) * 4.5 *2.032;		// 本应乘以0.045，这里放大了100倍，解码需要除以100
	}
	DEBUG("lux:%u\r\n",*lux);
	
	return  *lux;
}
/******************************************************读取光照度***********************************************/

