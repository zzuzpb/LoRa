/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "main.h"
#include "stm32f2xx_hal.h"
#include "stm32f2xx_hal_tim.h"
#include "utilities.h"
#include "timerserver.h"
#include "delay.h"
#include "spi.h"
#include "tim.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"
#include "gpio-board.h"
#include "radio.h"
#include "stmflash.h"
#include "power.h"
#include "debug.h"
#include "sx1276/sx1276.h"
#include "timer-board.h"
#include "sx1276-board.h"
#include "LoRaMac-api-v3.h"
#include "LoRa-cad.h"
#include "user-app.h"
#include "sim7600.h"
#include "w5500.h"
#include "w5500_conf.h"
#include "socket.h"
#include "utility.h"
#include "dhcp.h"
/*app函数头文件*/
#include "tcp_demo.h"


#if defined( USE_USB_CDC )
#include "uart-usb-board.h"
#endif

/*!
 * Define indicating if an external IO expander is to be used
 */
#define BOARD_IOE_EXT

/*!
 * Generic definition
 */
#ifndef SUCCESS
#define SUCCESS                                     1
#endif

#ifndef FAIL
#define FAIL                                        0
#endif


/*!
 * Unique Devices IDs register set ( STM32F1xxx )
 */
#define         ID1                                 ( 0x1FFFF7E8 )
#define         ID2                                 ( 0x1FFFF7EC )
#define         ID3                                 ( 0x1FFFF7F0 )

/*!
 * Random seed generated using the MCU Unique ID
 */
#define RAND_SEED                                   ( ( *( uint32_t* )ID1 ) ^ \
                                                      ( *( uint32_t* )ID2 ) ^ \
                                                      ( *( uint32_t* )ID3 ) )

enum BoardPowerSource
{
    USB_POWER = 0,
    BATTERY_POWER
};

/*!
 * System Clock Configuration
 */
void SystemClockConfig( void );

/*!
 * \brief Initializes the target board peripherals.
 */
void BoardInitMcu( void );

/*!
 * \brief Initializes the boards peripherals.
 */
void BoardInitPeriph( void );

/*!
 * \brief De-initializes the target board peripherals to decrease power
 *        consumption.
 */
void BoardDeInitMcu( void );

/*!
 * \brief Get the current battery level
 *
 * \retval value  battery level ( 0: very low, 254: fully charged )
 */
uint8_t BoardGetBatteryLevel( void );

/*!
 * Returns a pseudo random seed generated using the MCU Unique ID
 *
 * \retval seed Generated pseudo random seed
 */
uint32_t BoardGetRandomSeed( void );

/*!
 * \brief Gets the board 64 bits unique ID
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint8_t *id );

/*!
 * \brief Get the board power source
 *
 * \retval value  power source ( 0: USB_POWER,  1: BATTERY_POWER )
 */
uint8_t GetBoardPowerSource( void );


#endif // __BOARD_H__
