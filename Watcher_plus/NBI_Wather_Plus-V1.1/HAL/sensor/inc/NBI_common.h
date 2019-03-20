#ifndef __NBI_COMMON_H
#define __NBI_COMMON_H


#define COMMAND_COUNT				    20

#define NBI_SERVER_FLAG                 0
#define NBI_SOFT_FLAG      			    8
//#define NBI_SERVER_SP_FLAG            9
//#define NBI_SERVER_SP_FLAG            10
#define NBI_SERVER_SP_FLAG              11
#define NBI_SERVER_UD_FLAG              12
#define NBI_SERVER_SV_FLAG              13
#define NBI_SERVER_IP_FLAG              14
#define NBI_SERVER_GL_FLAG              15


#define GET_CRC(__X__,DATA)    ((__X__)[1] = ((DATA & 0xff00) >> 8), (__X__)[0] = (DATA & 0x00ff))
#define ARRAY(__X__)   (sizeof(__X__)/sizeof(__X__[0]))


#define RS485_DATA_LEN				100
#define REMOTE_IP_DATA_LEN		16
#define REMOTE_PORT_DATA_LEN	8
#define DEVICE_ID_DATA_LEN		20
#define VERSION_DATA_LEN			20
#define SYS_STU_DATA_LEN			100
#define NBI_TCP_FAILE_LEN     8

#define   BASE_ADDRESS    			 0x0801E000
#define   RS485_ADDRESS   			(0x10 + BASE_ADDRESS)			//6个空间 4字节
#define   REMOTE_IP_ADDRESS  		(RS485_ADDRESS + RS485_DATA_LEN)
#define   REMOTE_PORT_ADDRESS  	(REMOTE_IP_ADDRESS + REMOTE_IP_DATA_LEN)
#define   DEVICE_ID_ADDRESS    	(REMOTE_PORT_ADDRESS + REMOTE_PORT_DATA_LEN)
#define   VERSION_ADDRESS				(DEVICE_ID_ADDRESS + DEVICE_ID_DATA_LEN)
#define   SYS_STU_DATA_ADDRESS	(VERSION_ADDRESS + VERSION_DATA_LEN)
#define   NBI_TCP_FAILE_COUNT   (SYS_STU_DATA_ADDRESS+SYS_STU_DATA_LEN)


#define   NBI_SYS_ERROR						     (0xFF)
//兼容伟衍的代码
#define NBI_SEND_FAIL				0x0801fc60
#define FLAG_ADD						0x0801fd10			//在这里写入   0x02   表示需要重新下载程序
#define NBI_APP_AREA_ADDR		0x0801fea0			
#define NBI_AREA_A_RUN			0x0801fea4			
#define NBI_AREA_B_RUN			0x0801fea8	
#define	NBI_VER_A						0x0801fe88
#define	NBI_VER_B						0x0801fe70	
#define NBI_REMOTE_DOWN_IP	0x0801feb0


#define NBI_STREAM_ADDR     0x0801ffe0



		


#define BOOTLOADER_SIZE			0x5000			
#define AREA_SISE						0xa000			
#define	DATA_AREA_SIZE			0x1000				

#define ApplicationAddressA    	(0x8000000+BOOTLOADER_SIZE)			
#define ApplicationAddressB    	(ApplicationAddressA+AREA_SISE)		






#endif


