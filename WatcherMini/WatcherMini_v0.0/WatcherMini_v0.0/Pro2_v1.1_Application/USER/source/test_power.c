#include "user_main.h"
#include "user_bq24195.h"
#include "user_sim.h"
#include "user_led.h"
#include "user_adc.h"
#include "user_flash_L072.h"
#include "user_sensor_pro2.h"
#include "user_gps.h"
#include "bootloader_config.h"
#include <string.h>

void UserMain()
{
    int32_t Battery;
    uint16_t currentResult;
    uint8_t r0,r8,charge,*chargeResult,current,succeed,result,aux;

    succeed=1;
    printf("检查电源模块\r\n\r\n");
    HAL_Delay(2000);
    printf("插上电源适配器和电池,查看充电电源灯是否亮？？？？？\r\n\r\n");
    HAL_Delay(2000);
    
    printf("现在关闭充电，查看充电电源灯是否灭了？？？？？\r\n\r\n");
    Bq24195DisableCharge();
    HAL_Delay(3000);
    
    printf("现在打开充电，查看充电电源灯是否又亮了？？？？？\r\n\r\n");
    Bq24195EnableCharge();
    HAL_Delay(3000);
    
    if(AdcInit()==0)
    {
        printf("芯片初始化错误，重启\r\n");
        HAL_Delay(1000);
        HAL_NVIC_SystemReset();
    }
    else
    {
        AdcGetBattery(&Battery);
//        if(Battery==0)
//        {
//            printf("电量低休眠\r\n\r\n");
//            return;
//        }
//        else
//        {
//            printf("当前电量:%u\r\n\r\n",Battery);
//            
//        }
    }
    
    SetMiniSysVoltage(SYSTEM_VOLTAGE_3000MV
    +SYSTEM_VOLTAGE_400MV
    +SYSTEM_VOLTAGE_200MV
    +SYSTEM_VOLTAGE_100MV);
    SetWdgTimer(WATCHER_DOG_TIMER_DISABLE);
    SetInputCurrentLimit(INPUT_CURRENT_2000MA);
    SetChargeType(CHARGER_BATTERY);
    
    Bq24195ReadByte(0,&r0);
    
    current = r0 & INPUT_CURRENT_LIMIT;	
    Bq24195ReadByte(8,&r8);
    charge = r8 & CHARGE_STATUE;
    
    AdcGetBattery(&Battery); 
    
    if(r0&ENABLE_HIGH_IMPEDANCE_MODE){
        printf("先插入电池，再插入电脑USB，此情况不充电属于正常\r\n");
    }
    chargeResult = (uint8_t*)(
        charge==CHARGE_PRE_CHARGING?"预充电,正常OK\r\n":
        charge==CHARGE_FAST_CHARGING?"快速充电，正常OK\r\n":	
        charge==CHARGE_DONE_CHARGING?"充电完成，正常OK\r\n":"没充电，错误error\r\n");
    currentResult = 
        current==INPUT_CURRENT_100MA?100:
        current==INPUT_CURRENT_150MA?150:
        current==INPUT_CURRENT_500MA?500:
        current==INPUT_CURRENT_900MA?900:
        current==INPUT_CURRENT_1200MA?1200:
        current==INPUT_CURRENT_1500MA?1500:
        current==INPUT_CURRENT_2000MA?2000:
        current==INPUT_CURRENT_3000MA?3000:0;
    if(current==INPUT_CURRENT_100MA)
    {
        succeed=0;
        printf("充电错误\r\n");
    }
    printf("充电方式:%s充电电流:%dma\r\n电池电量:%d％\r\n"
            ,chargeResult,currentResult,Battery);

    Bq24195ReadByte(FAULT_REGISTER,&result);
    
    if(result!=0xff)
    {
        aux = result&CHARGE_FAULT;
        if(aux==CHARGE_NORMAL){
            printf("充电状态:正常OK\r\n");
        }else if(aux==CHARGE_INPUT_FAULT)
        {
            printf("充电状态:错误，输入电压过低\r\n");
            succeed=0;
        }else{
            printf("充电状态:错误error\r\n");
            succeed=0;
        }
        if(result&BATTERY_FAULT){
            printf("电池：error电池电压过高\r\n");
            succeed=0;
        }else{
            printf("电池:正常OK\r\n");
        }
    }
    printf("电池模块:%s\r\n\r\n",succeed==1?"正常OK":"错误ERROR");

}
