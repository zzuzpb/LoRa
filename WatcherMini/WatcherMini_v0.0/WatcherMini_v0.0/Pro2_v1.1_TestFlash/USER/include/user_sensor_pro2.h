/*
 * user_sensor_pro2.h	pro2设备的传感器驱动头文件，包括I2C传感器和RS485传感器
*/

#ifndef __USER_SENSOR_PRO2_H__
#define __USER_SENSOR_PRO2_H__

#include "user_config.h"


typedef struct SENSORDATA					//传感器数据
{
	uint8_t index;				//传感器数据位置标识（0光照 1空气温湿度 2 Socket0的传感器数据，3 Socket1的传感器数据，以此类推）
	uint8_t add;				//传感器地址(是485传感器的地址，光照、空气温湿度的可以不用赋值)
	uint8_t count;				//传感器数据域数量
	uint16_t data[2];			//最多2个数据
}T_SensorData;

/******************空气温湿度传感器******************/
/* Register addresses */
#define Hdc1080_ADDR			0x80	//设备I2C地址
#define HDC_TEMPERATURE_ADD		0x00
#define HDC_HUMIDITY_ADD		0x01
#define HDC_CONFIG_ADD          0x02
#define HDC_MANUFACTURE_ID_ADD	0xFE
#define HDC_DEVICE_ID_ADD 		0xFF
#define Manufacturer_ID_value 	0x5449
#define Device_ID_value 		0x1050
/*
 *	SensorHdc1080ReadData：	读取空气温湿度
 *	参数：					measure，保存温湿度数据，[0]为温度，1为湿度
 *  返回值:					1成功 0失败
 */
uint8_t SensorHdc1080ReadData(int16_t *measure);

/******************空气温湿度传感器******************/

/********************光照度传感器********************/
//#define MAX44009_ADDR		0x94	// MAX44009的设备I2C地址，地址脚接地
#define MAX44009_ADDR		0x96	// MAX44009的设备I2C地址，地址脚接VDD
#define LUX_HIGHT_ADDR		0x03	// Lux寄存器高字节的地址
#define LUX_LOW_ADDR		0x04	// Lux寄存器低字节的地址

/*
 *	SensorMax44009ReadData：读取光照度
 *	参数：					lux，保存光照度
 *  返回值:					0失败,成功返回光照度,与参数一样，只是方便有时候函数调用
 */
uint32_t SensorMax44009ReadData(uint32_t *lux);
/********************光照度传感器********************/


/*
 *	SensorInit：			传感器初始化
 *	参数：					无
 *  返回值:					1成功 0失败
 */
uint8_t SensorInit(void);
/*
 *	SernsorGetData：		获取所有传感器数据,所有数据都保存在sensor_data数组中
 *	参数pt_sensor_data：	指向获取到的数据的初始地址
 *  返回值:					0失败 成功返回1,并把sensor_data的地址保存到参数中
 */
uint8_t SernsorGetData(T_SensorData  *pt_sensor_data);

#endif /* __USER_SENSOR_PRO2_H__ */

