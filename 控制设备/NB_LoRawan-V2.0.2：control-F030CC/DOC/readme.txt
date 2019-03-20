
硬件IO接口图如下所示：
SX1278引脚接口说明：DIO4 DIO5控制取消掉

		SX1278					stm32F030CC
							 _ _ _ _ _ _ _ _ _ _ _ 
SPI     NSS	  --------	PA4 |					  |
		SCK	  --------	PA5 |    				  |
		MISO  --------	PA6 |					  |
		MOSI  --------	PA7 |					  |
						    |					  |
EXti	DIO0  --------	PB1 |                     |
		DIO1  --------	PB2 |					  |
		DIO2  --------	PB10|					  |
		DIO3  --------	PB11|					  |
							|					  |
CPIO	RESET --------	PB13|					  |
		DEBUG_LED ----- PA14|					  |
							|					  |
							|					  |
USART1	RX ------------	PB7	|					  |
		TX ------------	PB6	|					  |
							|					  |
USART5  RX ------------ PB4 |					  |
		TX ------------ PB3 |					  |
							|_ _ _ _ _ _ _ _ _ _ _|	



1：使用内部时钟HSI，不使用外部时钟原因：硬件存在有时上电RTC初始化失败

2：功能方面：
ADR默认开启，没有关闭

异常原因：
timer.c中TimerIrqHandler链表出错

电容：100pf

SX1276Write( 0x4d, 0x87 );///0x87打开 Set Pmax to +20dBm for PA_HP同时开启PABOOST设置高PA

McpsIndication.RxData


CLASS C存在问题：设备长期处于接收状态容易出现LoRa状态异常，因此必须某些时间段内上报数据心跳包，或者进行状态切换

