/*
 * user_sim.c	sim模块驱动文件
*/

#include "user_server.h"
#include "user_sim.h"
#include "user_flash_L072.h"

char G_A_Ack[SIM_RECIEVE_COUNT_MAX]           ={0};   // 保存服务器发来的ACK消息
char G_A_Downstream[SIM_RECIEVE_COUNT_MAX]    ={0};   // 保存服务器发来的下行控制命令
char G_Arg[SIM_RECIEVE_COUNT_MAX]             ={0};   // 下行控制中的参数部分

/*
 * SimRxSvMsgCallback Sim模块接收到服务器的数据将在此处理，函数在串口中断中被调用
*/
void SimRxSvMsgCallback(char *RxMsg,uint16_t Len)
{
    //DEBUG("RxMsg:%s\r\n",RxMsg);
    char *pt=strstr(RxMsg, "$ack@");
    if( pt!=NULL )
    {
        memcpy(G_A_Ack,pt,RxMsg+Len-pt);
    }
    else if(*RxMsg=='C')//控制消息
    {
        memcpy(G_A_Downstream,RxMsg,Len);
    }
    else
    {
        DEBUG("什么鬼\r\n");
    }
}

/*
 * ServerAck:	服务器回复Ack
 * 参数:	    检测的消息字符串
 * 返回值:	    0表示回复成功，1超时，2回复格式错误，3回复的seq比发送的seq大(正常应该回复发送出去的seq)
*/
uint8_t ServerAck(uint32_t *SensorSeq)
{
	//$ack@17081TEST001:$E=nbi_monitor,$seq=1
    uint32_t starttime=HAL_GetTick();
    uint8_t ret;
    while(HAL_GetTick()-starttime<5000 && G_A_Ack[0]==0);//5秒等待接收数据
    if(G_A_Ack[0]==0)
    {
        DEBUG("ACK:none\r\n");
        return 1;
    }
    else
    {
        uint32_t seq_recive=0;
        DEBUG("ACK:%s\r\n",G_A_Ack);
        sprintf(G_Arg,"$ack@%s:$E=%s,$seq=",DeviceID,Stream);
        if ( strstr(G_A_Ack,G_Arg) == NULL)
        {
            DEBUG("ACK Data Format ERROR\r\n");
            ret = 2;	//数据格式错误
        }
        else
        {
            ret = sscanf(G_A_Ack,"%*[^,],$seq=%u",&seq_recive);
            //ret返回转换成功的个数，有可能是部分转换成功，转换成功的参数值被改变，不成功的值未被改变
            //-1表示转换格式错误，比如第一个函数参数为NULL,0表示所有参数都没有转换成功，参数是指后面的那些，如cmd,arg,seq_char等，不是指函数的参数
            if (ret != 1)
            {
                DEBUG("ACK Data Format ERROR\r\n");
                ret = 2;	//数据格式错误
            }
            else
            {
                if(*SensorSeq == seq_recive )
                {
                    DEBUG("ACK OK\r\n");
                    ret = 0;	//回复成功
                }
                else
                {
                    DEBUG("seq_recive:%u\r\nseq_send:%u\r\n",seq_recive,*SensorSeq);
                    *SensorSeq=(*SensorSeq>seq_recive)?*SensorSeq:seq_recive;
                    DEBUG("resend \r\n");	//再发送一次数据，将客户端的seq与服务器的seq对应起来
                    ret = 3;
                }
            }
        }
        memset(G_A_Ack,0,sizeof(G_A_Ack));
    }
    
    return ret;
}

/*
 * IsControl:	检测服务器是否有控制命令
 * 参数:		设备ID、数据流、上一次控制命令的序列号,更新序列号为最新的
 * 返回值:		-1表示无控制命令
*/
int8_t ServerDowmsteam(uint32_t *CtrlSeq)
{
	/*state=0	执行成功
	*state=1	执行失败，升级失败在bootloader中做标志
	*state=2	不允许执行(切换版本)
	*state=3	命令参数错误
	*state=4	不支持命令
	*state=5	上次已经执行过此命令
	*state=6	其它错误
	*/
	//C17021Y2EP017:seq=1,n=SP,a=60
	//C17081TEST001:seq=1,n=IP,a=47.90.33.7|8033
	//C1703185FK030:seq=14,n=UD,a=http://down.nongbotech.cn/WatcherStandarApplication.hex|PR02_V3_170930
    //C1703185FK030:seq=14,n=UD,a=http://down.nongbotech.cn/WatcherStandarApplication.hex|PR02_V3_170930
	//char *service_ctrl = "C17021Y2EP017:seq=1,n=SP,a=60";
	//char *service_ctrl = "C123456789:seq=2,n=GL,a=0";
	//DEBUG("CTRL ServerReply:%x\r\n",ServerReply);
    if(G_A_Downstream[0]==0)
        return -1;
    char *pt = G_A_Downstream;
    int8_t ret = 0;
    char seq_char[11] = { 0 };
    char *arg_char=G_Arg;	//使用SimReBuff的内存空间，节省内存
    char cmd_char[3] = { 0 };	//控制命令最多2个字符
    uint32_t seq_recive = 0;
    DEBUG("ctrl:%s\r\n", G_A_Downstream);
    SetFlag(DOWNLINK_CONFIRM);//下行控制标志位
    if (*pt != 'C' || strstr(pt, DeviceID) == NULL || strstr(pt, ",a=") == NULL || strstr(pt, ",n=") == NULL || strstr(pt, ":seq=") == NULL)
    {
        DEBUG("Data Format ERROR\r\n");
        cmd_char[0] = 'X';
        cmd_char[1] = 'X';
        FlashWrite16(CTRL_NAME, (uint16_t *)cmd_char, 1);
        return 6;	//数据格式错误
    }
    ret = sscanf(pt, "%*[^:]:seq=%10[^,],n=%2[^,],a=%200s", seq_char, cmd_char, arg_char);
    //ret返回转换成功的个数，有可能是部分转换成功，转换成功的参数值被改变，不成功的值未被改变
    //-1表示转换格式错误，比如第一个函数参数为NULL,0表示所有参数都没有转换成功，参数是指后面的那些，如cmd,arg,seq_char等，不是指函数的参数

    DEBUG("cmd_char:%s\r\narg_char:%s\r\nseq_char:%s\r\n", cmd_char, arg_char, seq_char);
    if (ret != -1 && ret != 0)
        ret = sscanf(seq_char, "%u", &seq_recive);//-1将被转换成0xffffffff
    DEBUG("seq_downstream_recive:%u\r\n", seq_recive);
    if (seq_recive == 0 || cmd_char[0] == 0 || arg_char[0] == 0 || ret == 0 || ret == -1)
    {
        DEBUG("Data Format ERROR:NONE\r\n");
        return 6;
    }
    if (*CtrlSeq == seq_recive)
    {
        DEBUG("Already executed\r\n");
        return 5;
    }
    else
    {
        *CtrlSeq = seq_recive;
        DEBUG("Execute Command\r\n");						//收到的seq与本地的不相同就执行命令
        FlashWrite32(RECIEVE_SEQ, &seq_recive, 1);			//保存接受到的seq
        FlashWrite16(CTRL_NAME, (uint16_t *)cmd_char, 1);	//保存接受到的控制命令
        SetFlag(DOWNLINK_CONFIRM);							//下行控制回复标志
    }
    pt = cmd_char;
    /********************远程升级*********************/
    if (strncmp(pt, "UD", 2) == 0)
    {
        //arg="http://down.nongbotech.cn/Application.hex|pro2_v3_170930";
        //char url[URL_SIZE + 1] = { 0 };
        char *url=G_A_Ack;
        char stream[STREAM_SIZE + 1] = { 0 };
        pt = arg_char;
        ret = sscanf(pt, "%98[^|]|%32s", url, stream);
        if (url[0] == 0 || stream[0] == 0 || ret == 0 || ret == -1)
        {
            DEBUG("arg error\r\n");
            return 3;
        }
        //设置更新标志位，将在下次启动时进入bootloader更新
        SetFlag(UPDATE_RESET);
        FlashWrite16(URL, (uint16_t *)url, strlen(url) / 2 + 1);
        FlashWrite16(STREAM_ADDR, (uint16_t *)stream, strlen(stream) / 2 + 1);
        SetFlag(SOFT_RESET);
        return 0;//执行成功
    }
    /********************远程升级*********************/

    /*********************修改IP**********************/
    if (strncmp(pt, "IP", 2) == 0)//修改IP
    {
        char ip[32] = { 0 };
        char port[16] = { 0 };
        //arg=47.90.33.7|65535
        pt = arg_char;
        ret = sscanf(pt, "%31[^|]|%16s", ip, port);
        if (ip[0] == 0 || port[0] == 0 || ret == 0 || ret == -1)
        {
            DEBUG("arg error\r\n");
            return 3;
        }
        DEBUG("Change IP:\r\nip:%s\r\nport:%s\r\n", ip, port);
        FlashWrite16(SERVER_ADDR, (uint16_t *)ip, strlen(ip) / 2 + 1);
        FlashWrite16(SERVER_PORT_ADDR, (uint16_t *)port, strlen(port) / 2 + 1);
        SetFlag(SOFT_RESET);	//复位一次
        return 0;//执行成功
    }
    /*********************修改IP**********************/

    /******************修改采样周期**********************/
    if (strncmp(pt, "SP", 2) == 0)
    {
        char time_char[11] = { 0 };
        uint32_t time = 0;
        //arg=30;
        pt = arg_char;
        ret = sscanf(pt, "%10s", time_char);
        if (time_char[0] == 0 || ret == 0 || ret == -1)
        {
            DEBUG("arg error\r\n");
            return 3;
        }
        ret = sscanf(time_char, "%u", &time);
        if (time == 0 || ret == 0 || ret == -1)
        {
            DEBUG("arg error\r\n");
            return 3;
        }
        DEBUG("New Sample time：%u\r\n", time);
        FlashWrite32(SAMPLE_PERIOD_ADDR, &time, 1);
        SetFlag(SOFT_RESET);	//复位一次
        return 0;//执行成功
    }
    /******************修改采样周期**********************/

    /******************切换版本**********************/
    if (strncmp(pt, "SV", 2) == 0)
    {
        if (FlashRead16(APP_AREA_ADD) == APP_AREA_A && FlashRead16(AREA_B_RUN) == 0x31)		//当前运行在A区，同时B区可以运行
            FlashWrite16(APP_AREA_ADD, (uint16_t *)"1", 1);	//跳到B区
        else if (FlashRead16(APP_AREA_ADD) == APP_AREA_B && FlashRead16(AREA_A_RUN) == 0x31)	//当前运行在B区，同时A区可以运行
            FlashWrite16(APP_AREA_ADD, (uint16_t *)"0", 1);	//跳转到A区	
        else {
            DEBUG("Can't Switch Version \r\n");
            return 2;	//不允许执行
        }
        SetFlag(SOFT_RESET);	//复位一次
        return 0;//执行成功
    }
    /******************切换版本**********************/

    /*********************获取位置信息**********************/
    if (strncmp(pt, "GL", 2) == 0)//GetLocation
    {
        SetFlag(GET_LOCATION);
        SetFlag(SOFT_RESET);
        DEBUG("Get Location\r\n");
        return 0;//执行成功
    }
    /*********************获取位置信息**********************/

    return 4;//不支持的指令
}
