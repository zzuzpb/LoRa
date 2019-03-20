#ifndef __BOOTLOADER_CONFIG_H
#define __BOOTLOADER_CONFIG_H

#ifdef __cplusplus
	extern "C" {
#endif

#define APP_AREA_A			0x30				//A区运行,对应字符串0
#define APP_AREA_B			0x31				//B区运行,对应字符串1

#define CANNOTRUN			0x30				//不可运行,对应字符串0
#define RUNNABLE			0x31				//可运行,对应字符串1

#define STREAM_ADDR			0x0801ffe0			//数据流存储地址
#define STREAM_SIZE			0x20				//数据流占用存储空间
#define DEVICE_ID_ADDR		0x0801ffc0			//ID存储地址
#define DEVICE_ID_SIZE		0x20				//ID占用存储空间

#define SERVER_PORT_ADDR	0x0801ffb0			//服务器端口存储地址
#define SERVER_PORT_SIZE	0x10				//以字符形式储存服务器端口
#define SERVER_ADDR			0x0801ff90			//服务器地址
#define SERVER_SIZE			0x20				//服务器地址最大长度

#define URL					0x0801feb0			//升级使用的网址
#define URL_SIZE			170					//网址长度占用存储空间,170
#define APP_AREA_ADD		0x0801fea0			//运行区域的变量的地址，A区为'0',B区为'1'
#define AREA_A_RUN			0x0801fea4			//A区域是否可以运行,'1'可运行
#define AREA_B_RUN			0x0801fea8			//B区域是否可以运行,'1'可运行
#define	APP_AREA_SIZE		0x10				//运行区域的变量占用的存储空间
#define	VER_A				0x0801fe88			//A区版本号
#define	VER_B				0x0801fe70			//B区版本号
#define VER_SIZE			0x18

#define SAMPLE_PERIOD_ADDR	0x0801fe50			//采样周期存储地址
#define SAMPLE_PERIOD_SIZE	0x10				//采样周期占用存储空间
#define	RECIEVE_SEQ			0x0801fe40			//接收的自增序号,用于下行控制
#define RECIEVE_SEQ_ACK		0x0801fe44			//从ACK中接收到的seq,作为RECIEVE_SEQ的备份
#define RECIEVE_SEQ_SIZE	0x10
#define	SEND_SEQ			0x0801fe30			//发送的自增序号,用于上行控制,由于网络等因素,该值可能不是严格的自增序号
#define SEND_SEQ_SIZE		0x10
//#define	SEND_SEQ_TOTAL		0x0801fe20			//发送的总次数,大小为SEND_SEQ_SIZE
//#define	SEND_SEQ_SUCCEED	0x0801fe10			//发送后成功收到回复的次数,大小为SEND_SEQ_SIZE

//#define	BOOT_TOTAL			0x0801fe00			//烧写的总次数
//#define BOOT_CMD_TOTAL		0x0801fe04			//因接收到升级命令而烧写的总数
//#define REBOOT_TOTAL		0x0801fe08			//因烧写失败而重新烧写的总数
//#define BOOT_SUCCEED		0x0801fdf0 			//烧写成功的总数

//#define INIT_SIMCOM_FAIL	0x0801fde0			//初始化sim失败总数
//#define SIMCOM_CMD_DO_ERR	0x0801fde4			//SIMCOM命令执行出错
//#define SIMCOM_CMD_DO_TMEOUT 0x801fde6			//SIMCOM命令执行超时
//#define SIMCOM_CMD_BUSY		0x0801fde8			//SIMCOM串口忙
//#define SIMCOM_CMD_SEND_ERR	0x0801fdea			//SIMCOM命令发送出错

//#define CONNECT_TCP_FAIL	0x0801fdd0			//连接tcp失败总数
//#define	CONNECT_TCP_NONE	0x0801fdd4			//无tcp连接
//#define	CONNECT_TCP_FAILURE	0x0801fdd8			//TCP连接失败，或者连接成功后服务器关闭连接
//#define	CONNECT_TCP_EXISTED	0x0801fddc			//已存在tcp连接

//#define SEND_TCP_DATA_FAIL	0x0801fdc0			//tcp发送失败总数
//#define SEND_TCP_NONE		0x0801fdc4			//当前无tcp连接
//#define SEND_TCP_FAILURE	0x0801fdc8			//存在过TCP连接，但已经被关闭
//#define SEND_TCP_TRANS_FAIL	0x0801fdcc			//tcp发送失败

//#define GPRSPROGRAM_FAIL	0x0801fdb0			//下载数据过程失败总数
//#define DOWNLOADDATA_ERR	0x0801fdb4			//下载数据错误或HEX文件本身错误
//#define DOWNLOAD_TIMEOUT	0x0801fdb8			//下载数据超时

//#define URL_FAIL			0x0801fda0			//URL错误总数
//#define URL_ERROR			0x0801fda4			//URL没有找到'/'
//#define URL_EMPTY			0x0801fda8			//URL为空

//#define MEMORY_OVERFLOW		0x0801fd90			//内存溢出,在发送数据时有可能出此错误		
//#define SHOWFLASH			0x0801fd80			//用于控制是否调试输出FLASH中的数据
//#define UPDATE_RETRY		0x0801fd70			//更新重试

#define CTRL_NAME			0x0801fd30			//下行控制的名称
#define	CTRL_NAME_SIZE		0x10
#define CTRL_STATE			0x0801fd20			//下行控制的状态,0成功,1失败,2禁止再次发送此控制命令
#define	CTRL_STATE_SIZE		0x10
#define FLAG_ADD			0x0801fd10			//备份寄存器的代替位置，用于记录各种标识位
#define	FLAG_ADD_SIZE		0x10

#define SWITCH_REPLY_ADD 	0x0801fd00			//用于保存限位开关回复服务器的数据
#define DOWNSTREAM_ADD		0x0801fc80			//用于保存旧的下行控制命令
#define DOWNSTREAM_SIZE		0x80

#define SEND_TOTAL			0x0801fc70			//发送的全部次数
#define SEND_FAIL			0x0801fc60			//发送失败的次数
#define SEND_SUCCEED		0x0801fc50			//发送成功的次数
#define SEND_RECIEVE_ACK	0x0801fc40			//发送成功并且收到服务器回复的次数

#define DATE_ADD            0X0801FC30          //日期，天数地址

#define RS485_CONNECT_ADD   0x0801fc20          //485连接备份
#define RS485_CONNECT_SIZE  0x10
#define SEND_FLASH_ADD      0x0801fc10          
#define RS485_CONNECT_SIZE  0x10
#define SAVE_FLASH_ADD      0x0801fc00          
#define RS485_CONNECT_SIZE  0x10

#define BOOTLOADER_SIZE			0x5000			//bootloder占用的flash空间	//20K
#define AREA_SISE				0xa000			//A或B分区占用的flash空间	//40K
#define ERROR_DATA              BOOTLOADER_SIZE+AREA_SISE*2 //错误记录区域
#define	DATA_AREA_SIZE			0x1000			//数据储存区域大小		//4K

#define ApplicationAddressA    	(0x8000000+BOOTLOADER_SIZE)			//用户程序A区的入口地址
#define ApplicationAddressB    	(ApplicationAddressA+AREA_SISE)		//用户程序B区的入口地址

#ifdef __cplusplus
}
#endif

#endif
