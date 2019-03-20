#ifndef NBI_rs485_H__
#define NBI_rs485_H__


#include "stm32l0xx_hal.h"


#define NBI_RS485_SEND_DATA_LEN     20 
#define NBI_RS485_REV_DATA_LEN      50 
#define NBI_RS485_REV_TIME_OUT      1000
#define NBI_RS485_SEND_BUFF_LEN     13
#define NBI_RS485_PIN_COUNT         6
#define NBI_RS485_EXBOX_COUNT       5


#define NBI_RS485_SEARCH_CODE       0x03
#define NBI_RS485_SET_CODE          0x05



typedef enum
{
	EXPENDBOX = 0,	
	QXZ = 1,
	PH,
	OXY,	
	ANDAN,
	DIANGAN,
	ST_TW,
	ST_EC,
	ST_SWR = 8,
	ST_YMW,
	ST_YMS = 10,	
	ST_PH,
	ST_GL,	
	ST_YL,		
	ST_FS,		
	ST_FX = 15,			
	ST_AP,			
	ST_CO2,			
	TEMP_NULL,
}enum_RS485_index;
typedef struct 
{
	char addr;
	int  regAddress;
	int  regDatalen;
//	int 	dataLen[5];
	char sendDataLen;
	char revDataLen;
	uint32_t timeOut;
	uint8_t sendBuff[NBI_RS485_SEND_DATA_LEN];
	uint8_t revBuff[NBI_RS485_REV_DATA_LEN];	
	char name[10];
}stu_NBI_Rs485;

typedef enum
{
	RS485_NULL = 0,
	RS485_SIGNAL = 1,
	RS485_EXPAND_BOX = 2,
	RS485_NONE,
	
}RS485_STATUS;

typedef enum
{
	TIME_OUT = 5,
	LEN_ERROR = 6,
	NOT_LIVE,
	EXPEND_OPEN_ERROR,
	ADDRESS_NOT_FIND,
	FIND_ADDRESS,
}RS485_STATUS_CODE;


typedef struct
{
	char            index;  //接口ID号
    char            mainportlive; ///主板485口接入标志，6个口
	char            expendboxlive;  ///扩展口标志
	int             timeout;	
	uint16_t        Rs485Data[5];
	stu_NBI_Rs485   *data;
	char            status;
	char            type;
}stu_Rs485_data;

extern stu_Rs485_data g_rs485mainportbuff[NBI_RS485_PIN_COUNT];
extern stu_Rs485_data g_rs485exboxbuff[NBI_RS485_EXBOX_COUNT];

uint8_t _12VPowerOn(void);
uint8_t _12VPowerOff(void);

extern int InitRs485(void);
extern RS485_STATUS getRs485Status(int PortId);
extern HAL_StatusTypeDef RS485_getDataForSensor(uint8_t index, uint8_t exindex);
extern HAL_StatusTypeDef Rs485_GetData(void );
extern int get_rs485Data(uint8_t *data);


extern unsigned int U4rever(unsigned int data);
extern unsigned short U2rever(unsigned short data);

extern unsigned int U4(unsigned char* data);
extern unsigned short U2(unsigned char* data);

#endif

