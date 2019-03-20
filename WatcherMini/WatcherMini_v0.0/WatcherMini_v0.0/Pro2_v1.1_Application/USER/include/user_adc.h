/*
 * user_adc.h	adc驱动头文件，用于电量采集、传感器数据采集(水压)
*/

#ifndef __USER_ADC_H__
#define __USER_ADC_H__


#include "user_config.h"

#define VREFINT_ADC (*((uint16_t *)0x1FF80078)) //参考电压的ADC值
#define REFINT_VDD 3							//测试参考电压的ADC值时的供电电压，datasheet上有写

/*
 * AdcInit:				初始化ADC
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t AdcInit(void);

/*
 *AdcGetValue: 			获取ADC某通道上的平均值
 *channel:				通道,具体值为ADC_CHANNEL_0~ADC_CHANNEL_18
 *time:					次数
 *返回值:				获取到的ADC某通道上的平均值
*/
uint32_t AdcGetValue(uint32_t channel,uint8_t time);

/*
 *AdcGetVoltage: 		获取ADC某通道上的电压值,相对电压采用内部电压
 *channel:				通道,具体值为ADC_CHANNEL_0~ADC_CHANNEL_18
 *返回值:				获取到的ADC某通道上的电压值,返回0表示失败
*/
float AdcGetVoltage(uint32_t channel);

int32_t AdcGetBattery(int32_t *Battery);
/*
 *AdcGetChargePower: 		获取充电电压
 *Battery:				电池电量，0-100%
 *返回值:				电池电量，0-100%
 */
float AdcGetChargePower(float ChargePower);
#endif /* __ADC_H__ */

