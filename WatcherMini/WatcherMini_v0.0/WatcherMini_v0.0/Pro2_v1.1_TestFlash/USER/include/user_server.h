/*
 * user_sim.h	sim模块驱动头文件
*/

#ifndef __USER_SERVER_H__
#define __USER_SERVER_H__

#include "user_config.h"

uint8_t ServerAck(uint32_t *SensorSeq);
int8_t ServerDowmsteam(uint32_t *CtrlSeq);

#endif /* __USER_SIM_H__ */
