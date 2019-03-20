/**
  ******************************************************************************
  * 文件名程: 
  * 作    者: 
  * 版    本: 
  * 编写日期: 
  * 功    能: 
  ******************************************************************************
  * 说明：
  * 芯片充电输入电压为3-16.5V
  * 充满为4.2V
  * 重充电为低于4.1V
  ******************************************************************************
  */

/* 包含头文件 ----------------------------------------------------------------*/
#include "user_battery.h"
#include "adc.h"
#include "gpio.h"
#include "power.h"
#include "debug.h"

/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
/* 扩展变量 ------------------------------------------------------------------*/
/* 私有函数原形 --------------------------------------------------------------*/
/* 函数体 --------------------------------------------------------------------*/

/*
 * Bq24195Init:			初始化供电电源芯片
 * 参数:				无
 * 返回值:				无
*/
void Bq24195Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOA_CLK_ENABLE(); 
    __HAL_RCC_GPIOB_CLK_ENABLE();               		 //开启GPIOB时钟
   
    GPIO_InitStruct.Pin = Out_CH_CE_Pin_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(Out_CH_CE_Pin_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = In_CH_INT_Pin_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(In_CH_INT_Pin_GPIO_Port, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = IN_CH_STATE1_Pin|IN_CH_STATE2_Pin|IN_CH_PG_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(IN_CH_STATE1_GPIO_Port, &GPIO_InitStruct);
}


/*
 * 开始充电
*/
void BatEnableCharge(void)
{
    HAL_GPIO_WritePin(Out_CH_CE_Pin_GPIO_Port,Out_CH_CE_Pin_Pin,GPIO_PIN_RESET);
}

/*
 * 关闭充电
*/
void BatDisableCharge(void)
{
    HAL_GPIO_WritePin(Out_CH_CE_Pin_GPIO_Port,Out_CH_CE_Pin_Pin,GPIO_PIN_SET);;
}

/*
 * 检查充电状态
 * 返回值：  
 *      0 -- 正在充电
 *      1 -- 未充电
 *      2 -- 充电完成
 *      3 -- 未插入充电器
*/
uint8_t BatCheck(uint8_t *bat)
{
    uint8_t s1 = (uint8_t)HAL_GPIO_ReadPin(IN_CH_STATE1_GPIO_Port,IN_CH_STATE1_Pin);
    uint8_t s2 = (uint8_t)HAL_GPIO_ReadPin(IN_CH_STATE2_GPIO_Port,IN_CH_STATE2_Pin);
    uint8_t PG = (uint8_t)HAL_GPIO_ReadPin(IN_CH_PG_GPIO_Port,IN_CH_PG_Pin);
       
    if (CheckBattery()<=3)					//电池电量低
    {
        DEBUG(2,"battery extremely low %dmV;enter standby\r\n",RF_Send_Data.Battery*6+3600);
        IntoLowPower( ); 					//电池电量特别低，直接重置进入休眠
    }

    float bat_v,charge_v;
    
//    *bat = BatGetBattery(&bat_v);
//    DEBUG("电池电压：%f\r\n",bat_v);
//    DEBUG("电池电量：%u\r\n",*bat);
//    
//    BatGetChargePower(&charge_v);
//    DEBUG("充电电压：%f\r\n",charge_v);
    if(charge_v<3)//充电电压小于3V
    {
        DEBUG(2,"未插入充电器\r\n");
        return 3;
    }
    
    if(PG==1)
    {
        DEBUG(2,"充电休眠，充电输入电压小于电池电压\r\n");
        //return 4;
    }
    switch( (s2<<1) | s1 )
    {
        case 0x01:
            DEBUG(2,"正在充电\r\n");
            return 0;
        case 0x02:
            DEBUG(2,"充电完成\r\n");
            return 2;
        default:
            DEBUG(2,"未充电\r\n");
            return 1;
    }
}

/*
 *AdcGetBattery: 		获取电池电压
 *Battery:				电池电压
 *返回值:				电池电量，0-100%
 */
uint8_t BatGetBattery(float *Battery_V)
{
//	*Battery_V=AdcGetVoltage(ADC_CHANNEL_1)*2*1.0312;//粗略计算
//    uint8_t bat;
//    if(*Battery_V<3.6)
//    {
//        bat=0;
//	}
//    else
//    {
//        bat=(int32_t)((*Battery_V-3.6)/(4.18-3.6)*100);//电路电阻接2M、2M，最高充电电压为4.2V
//	}
//    if(bat>100)
//        bat=100;
//    return bat;
}

/*
 *AdcGetChargePower: 		获取充电电压
 *Battery:				电池电量，0-100%
 *返回值:				电池电量，0-100%
 */
float BatGetChargePower(float *ChargePower)
{
//	*ChargePower=(AdcGetVoltage(ADC_CHANNEL_0)*1.0312)*222/22;//电路电阻接220K、2M
//	return *ChargePower;
}
