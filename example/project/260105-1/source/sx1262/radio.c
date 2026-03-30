/*!
 * \file      radio.c
 *
 * \brief     Radio driver API definition
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include <math.h>
#include <string.h>
#include "radio.h"
#include "sx126x.h"
#include "sx126x-board.h"
#include "config.h"
#if (USE_SX1262 > 0)
uint8  iStatus_RF = 0;							

uint8  iRF_TX_Buf[64];		   				// RFM98射频发送数据缓存
uint8  iRF_RX_Buf[64];							// RFM98射频接收数据缓存
uint8  iRF_TX_Buf_bak[64];
//uint8  iRF_RX_Buf_Temp[64];				// RFM98射频接收数据缓存数据备份
uint32  fTimer_RF_TX = 0; 					// RFM98数据发送等待时间

#define CRC_EN                                      1
#define	CRC_OFF										0
#if SET_LGW_NATION
	#define TX_OUTPUT_POWER                             12
	#define	CRC_TYPE									CRC_EN
#else
	#define TX_OUTPUT_POWER                             22        // dBm
	// #define TX_OUTPUT_POWER                             20        // dBm
	#define	CRC_TYPE									CRC_OFF
#endif

#define LORA_SPREADING_FACTOR                       9         // [SF7..SF12]
#define LORA_CODINGRATE                             4         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#if SET_LGW_NATION
	#define LORA_PREAMBLE_LENGTH                        8
#else															  
	#define LORA_PREAMBLE_LENGTH                        12	//108       // Same for Tx and Rx
#endif

#define LORA_SYMBOL_TIMEOUT                         100       // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
#define LORA_FIX_LENGTH_PAYLOAD_LEN                 19

#if SET_LORA_RL
#else
	#define CONFIG_32M_TCXO  //温补
#endif

/**
 * @description: 
 * @param mode: 0 上电复位，1 发射模式配置, len：发送数据长度（mode = 1时有效）
 * @return: 
 */
void RF_Init(u8 mode, u8 len)
{
	unsigned char buf[10];
	LOG("RFM96 Init\r\n");
//-------初始化Lora
	if(mode == 0)
	{		
		SX126xIoInit();		//初始化控制脚
		SX126xReset();    //复位设备
		SX126xWakeup(); 	//唤醒设备
	}
	
	
	if(g_device_info.isok_get_Lora==0)
	{
		return ;
	}
		
//-----判断状态，和是否通讯正常	
{	
	RadioStatus_t status ;
	SX126xReadRegisters(REG_LR_CRCPOLYBASEADDR, buf, 2 ); //读取CRC数据
	//dPrintf_Lora_Log(dLoraColor,"LoraCRC_Wither1021:%x%x\r\n",(u16)buf[0],(u16)buf[1]);//	dLora_Str_Hex_Log(buf,2);//打印内容为0x1021，与默认内容一样，SPI通信正常
	status = SX126xGetStatus();
	//dPrintf_Lora_Log(dLoraColor,"LoraStatues:%x\r\n",(u16)status.Value);//UART_PutStr("ST:");//UART_PutHex(status.Value);
}
//-----配置模式和数据包类型	
	SX126xSetStandby(STDBY_RC);							//STDBY_RC模式
	SX126xSetPacketType(PACKET_TYPE_LORA);  			//数据包类型为LORA 
//-----启动32MHz输入
	#ifdef CONFIG_32M_TCXO
	{
		CalibrationParams_t calibParam;				 
		/*配置IO3引脚为32M输出，可配置输出电压，和等待起振时间，作为TXCO的输入*/
		SX126xSetDio3AsTcxoCtrl( TCXO_CTRL_1_8V, SX126xGetBoardTcxoWakeupTime( ) << 6 ); // convert from ms to SX126x time base
		calibParam.Value = 0x7F;
		SX126xCalibrate( calibParam );  //上电复位校准
	}
	#endif
//-----设置天线
	if(IS_PROTO_SHIKE)
	{
		SX126xSetRfFrequency(RF_FREQUENCY_SHIKE); 	 //434	//设置载波
	}
	else
	{
		#if SET_ALARM_VER
		SX126xSetRfFrequency(RF_FREQUENCY_SHIKE);
		#else
		SX126xSetRfFrequency(RF_FREQUENCY); 	 //444	//设置载波
		#endif
	}
	
	SX126xSetDio2AsRfSwitchCtrl(true); 		 //DIO2 控制外部RF开关  //true:DIO2被选中用于控制射频开关。
	SX126xSetRfTxPower( TX_OUTPUT_POWER ); //配置发射功率和斜坡时间
//SX126xSetDio2AsRfSwitchCtrl(false); //DIO2 控制外部PA开关
//-----设置调制解调和收发缓存	
	SX126xSetBufferBaseAddress(0x00,0x00); //将TX和RX基址存储在所选协议调制解调器的寄存器中
	
	SX126xSetRegulatorMode(USE_DCDC);	//使用DCDC
	
	if(IS_PROTO_SHIKE)
	{
		buf[0] = LORA_SF_SHIKE;   		 //SF  //扩频因子
	}
	else
	{
		#if SET_ALARM_VER
		buf[0] = LORA_SF_SHIKE;
		#else
		buf[0] = LORA_SF9;   		 //SF  //扩频因子
		#endif
	}
	
	if(IS_PROTO_SHIKE)
	{
		buf[1] = LORA_BANDWIDTH_SHIKE; //BW	 //带宽
	}
	else
	{
		#if SET_ALARM_VER
		buf[1] = LORA_BANDWIDTH_SHIKE; //BW	 //带宽
		#else
		buf[1] = LORA_BANDWIDTH; //BW	 //带宽
		#endif
	}
	
	
	buf[2] = LORA_CR_4_8; 	 //CR	 //前向纠错能力  //编码率
	// SX126xWriteCommand(RADIO_SET_MODULATIONPARAMS, buf, 3);
	buf[3] = 1; // LowDataRateOptimize，低速率优化
	SX126xWriteCommand(RADIO_SET_MODULATIONPARAMS, buf, 4);  //为给定的调制参数计算和设置选定的协议调制解调器的值
//-----设置数据包格式
	buf[0] = (LORA_PREAMBLE_LENGTH >> 8) & 0xFF; //high  //前导码长度是一个 16 位的值，代表无线电将发送的 LoRa符号的数量。//数据包总长度？？
	buf[1] = LORA_PREAMBLE_LENGTH;               //low	
	buf[2] = 0x00;                               //Fixed length packet(固定长度数据包)，0x00可变 //当字节头类型为0x00时，即显示头，负载长度、编码率和头CRC将添加到LoRa头并传输到接收器
	if(mode == 1 && len)
	{//负载长度
			buf[3] = len;//Size of the payload (in bytes) to transmit or maximum size of the payload that the receiver can accept.     
	}
	else
	{
			buf[3] = 21; //Size of the payload (in bytes) to transmit or maximum size of the payload that the receiver can accept.
	}
	
	buf[4] = CRC_TYPE;			//CRCType  off  //关闭CRC
	buf[5] = 0;			//Standard IQ setup  //？
	SX126xWriteCommand(RADIO_SET_PACKETPARAMS, buf, 6 );		//该命令用于设置数据包处理模块的格式

	// SX126xSetStandby( STDBY_RC );
	// buf[0] = 0x14;    
	// buf[1] = 0x24; 
	// buf[0] = 0x34;    
	// buf[1] = 0x44; 
	// SX126xWriteRegisters(REG_LR_SYNCWORD, buf, 2 );
	// SX126xReadRegisters(REG_LR_SYNCWORD, buf, 2 );
	// user_printf_hex(buf,2);//打印内容为0x1021，与默认内容一样，SPI通信正常
}


void RadioSleep( void )
{
    SleepParams_t params = { 0 };

    params.Fields.WarmStart = 1;
    SX126xSetSleep( params );
    DelayMs(2);
}
#if USE_SX1262
const uint16_t RxIrqMask = IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT;
const uint16_t TxIrqMask = IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT;
void RF_send_data(u8 *TxBuf,u8 len)
{
    //u16 irq = 0;
    unsigned char buf[10] = {0};

    SX126xSetPaOpt(0);//默认SetPaConfig
    SX126xSetStandby(STDBY_RC);//待机模式
    SX126xSetPacketType(PACKET_TYPE_LORA);//LoRa模式

	if(IS_PROTO_SHIKE)
	{
		SX126xSetRfFrequency(RF_FREQUENCY_SHIKE);
	}
	else
	{
		#if SET_ALARM_VER
		SX126xSetRfFrequency(RF_FREQUENCY_SHIKE);
		#else
		SX126xSetRfFrequency(RF_FREQUENCY);//设置频率
		#endif
	}
    
    
    SX126xSetRfTxPower( TX_OUTPUT_POWER );//发射功率
    SX126xSetBufferBaseAddress(0x00,0x00);//设置buf地址
    SX126xWriteBuffer(0, TxBuf, len);
	if(IS_PROTO_SHIKE)
	{
		buf[0] = LORA_SF_SHIKE;   		 //SF  //扩频因子
	}
	else
	{
		#if SET_ALARM_VER
		buf[0] = LORA_SF_SHIKE;   		 //SF  //扩频因子
		#else
		buf[0] = LORA_SF9;   		 //SF  //扩频因子
		#endif
	}
	
  if(IS_PROTO_SHIKE)
	{
		buf[1] = LORA_BANDWIDTH_SHIKE; //BW	 //带宽
	}
	else
	{
		#if SET_ALARM_VER
		buf[1] = LORA_BANDWIDTH_SHIKE; //BW	 //带宽
		#else
		buf[1] = LORA_BANDWIDTH; //BW	 //带宽
		#endif
	}
    buf[2] = LORA_CR_4_8; //CR
    buf[3] = 1; // LowDataRateOptimize，低速率优化
    SX126xWriteCommand(RADIO_SET_MODULATIONPARAMS, buf, 4);

    buf[0] = (LORA_PREAMBLE_LENGTH >> 8) & 0xFF; //high
    buf[1] = LORA_PREAMBLE_LENGTH;               //low
    buf[2] = 0x00;                               //	Fixed length packet(固定长度数据包)，0x00可变
  // 发射数据长度
    buf[3] = len;//Size of the payload (in bytes) to transmit or maximum size of the payload that the receiver can accept.     
    buf[4] = CRC_TYPE;                                  //CRCType  off
    buf[5] = 0;                                  //Standard IQ setup
    SX126xWriteCommand( RADIO_SET_PACKETPARAMS, buf, 6 );

	// SX126xSetBufferBaseAddress(0x00,0x46);
    SX126xSetDioIrqParams(TxIrqMask, TxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE);
    SX126xClearIrqStatus(IRQ_RADIO_ALL);
  // SX126xSetTx(0);
		
    //SX126xSetTx(20000);//超时xx*15.625us
		SX126xSetTx(0);
}

#if 0
u8 RF_cad_check_channel2(u32 LORA_FQ)
{

    // uint16_t cadIrqMask = IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED;
    u16 irqRegs = 0;
    u8 ret = 0;
    int i = 0;
    /*
    UART_PutStr("开始设置参数");
    UART_PutHex(g_runTime_ms >> 8);
    UART_PutHex(g_runTime_ms);
    UART_PutStr("\r\n");
 	*/
    SX126xSetStandby(STDBY_RC);
    SX126xSetRfFrequency(LORA_FQ);
    SX126xSetCadParams(LORA_CAD_04_SYMBOL, 28, 10, LORA_CAD_ONLY, 0xff); //
    // SX126xSetCadParams( LORA_CAD_02_SYMBOL, 22, 10,  LORA_CAD_ONLY, (((u32)2000 * 1000) / 15.625 ) );//
    // SX126xClearIrqStatus(IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED); //clear flags
    SX126xClearIrqStatus(IRQ_RADIO_ALL);
    SX126xSetDioIrqParams(IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED, IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED, IRQ_RADIO_NONE, IRQ_RADIO_NONE);

    // // RF_ready_to_rx();
    // SX126xSetStandby(STDBY_RC);
    // SX126xSetRx(0xFFFFFF);//接收模式
    // // SX126X_CAD_Sample();
    SX126xSetCad();

    // #if SysDEBUG
    //     UART_PutStr("开始检测");
    //     UART_PutHex(g_runTime_ms >> 8);
    //     UART_PutHex(g_runTime_ms);
    //     UART_PutStr("\r\n");
    // #endif
    for (i = 0; i < 30; i++)
    {
        if(READ_RF_IRQ())
        {
            irqRegs = SX126xGetIrqStatus();
            if ((irqRegs & IRQ_CAD_DONE) == IRQ_CAD_DONE)
            {
                if ((irqRegs & IRQ_CAD_ACTIVITY_DETECTED) == IRQ_CAD_ACTIVITY_DETECTED)
                { //检测到信道被占用
#if 1
                    UART_PutStr("------------------信道占用\r\n");
                    UART_PutStr("\r\n");
#endif
                    ret = 1;
                    //if (LORA_FQ == RF_SLEEP_CALL)
                    //    iStatus_RF = RFM98_TX_TO_RX;
                    break;
                }
                else
                { //检测到信道空
                    // #if SysDEBUG
                    // UART_PutStr("信道空");
                    // UART_PutHex(g_runTime_ms >> 8);
                    // UART_PutHex(g_runTime_ms);
                    // UART_PutStr("\r\n");
                    // #endif

                    //if (LORA_FQ == RF_SLEEP_CALL)
                    //    iStatus_RF = RFM98_RX_TO_SLEEP;
										break;
                }
            }
        }
        DelayMs(1); //延时1ms
    }
    return ret;
}
#endif

#if 1
u8 RF_cad_check_channel2(u32 LORA_FQ)
{
    // uint16_t cadIrqMask = IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED;
    u16 irqRegs = 0;
    u8 ret = 0;
    int i = 0;
	
    /*
    UART_PutStr("开始设置参数");
    UART_PutHex(g_runTime_ms >> 8);
    UART_PutHex(g_runTime_ms);
    UART_PutStr("\r\n");
 	*/
    SX126xSetStandby(STDBY_RC);
    SX126xSetRfFrequency(LORA_FQ);
    SX126xSetCadParams(LORA_CAD_04_SYMBOL, 28, 10, LORA_CAD_ONLY, 0xff); //
    // SX126xSetCadParams( LORA_CAD_02_SYMBOL, 22, 10,  LORA_CAD_ONLY, (((u32)2000 * 1000) / 15.625 ) );//
    // SX126xClearIrqStatus(IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED); //clear flags
    SX126xClearIrqStatus(IRQ_RADIO_ALL);
    SX126xSetDioIrqParams(IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED, IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED, IRQ_RADIO_NONE, IRQ_RADIO_NONE);

    SX126xSetCad();

    for (i = 0; i < 30; i++)
    {
        if(READ_RF_IRQ())
        {
            irqRegs = SX126xGetIrqStatus();
            if ((irqRegs & IRQ_CAD_DONE) == IRQ_CAD_DONE)
            {
                if ((irqRegs & IRQ_CAD_ACTIVITY_DETECTED) == IRQ_CAD_ACTIVITY_DETECTED)
                { //检测到信道被占用
#if 1
                    UART_PutStr("------------------信道占用\r\n");
                    UART_PutStr("\r\n");
#endif
                    ret = 1;
//                    if (LORA_FQ == RF_SLEEP_CALL)
//                        iStatus_RF = RFM98_TX_TO_RX;
                    break;
                }
                else
                { //检测到信道空
                    // #if SysDEBUG
                    // UART_PutStr("信道空");
                    // UART_PutHex(g_runTime_ms >> 8);
                    // UART_PutHex(g_runTime_ms);
                    // UART_PutStr("\r\n");
                    // #endif

//                    if (LORA_FQ == RF_SLEEP_CALL)
//                        iStatus_RF = RFM98_RX_TO_SLEEP;
										break;
                }
            }
        }
        DelayMs(1); //延时1ms
		p_RunPro();
    }

//    if (LORA_FQ == RF_SLEEP_CALL)
//    {
//        if (iStatus_RF != RFM98_TX_TO_RX)
//        {
//            iStatus_RF = RFM98_RX_TO_SLEEP;
//        }
//    }
			
    return ret;
}
#endif


/**
 * @description: 进入接收模式
 * @param {type} 
 * @return: 
 */
void RF_ready_to_rx(void)
{
    unsigned char buf[10] = {0};
    SX126xSetStandby(STDBY_RC);				
    SX126xSetPacketType(PACKET_TYPE_LORA);

    if(IS_PROTO_SHIKE)
		{
			SX126xSetRfFrequency(RF_FREQUENCY_SHIKE);
		}
		else
		{
			#if SET_ALARM_VER
			SX126xSetRfFrequency(RF_FREQUENCY_SHIKE);
			#else
			SX126xSetRfFrequency(RF_FREQUENCY);//设置频率
			#endif
		}
    
    SX126xSetRfTxPower( TX_OUTPUT_POWER );//发射功率
    SX126xSetBufferBaseAddress(0x00,0x00);//设置buf地址
    if(IS_PROTO_SHIKE)
		{
			buf[0] = LORA_SF_SHIKE;   		 //SF  //扩频因子
		}
		else
		{
			#if SET_ALARM_VER
			buf[0] = LORA_SF_SHIKE;   		 //SF  //扩频因子
			#else
			buf[0] = LORA_SF9;   		 //SF  //扩频因子
			#endif
		}
    if(IS_PROTO_SHIKE)
		{
			buf[1] = LORA_BANDWIDTH_SHIKE; //BW	 //带宽
		}
		else
		{
			#if SET_ALARM_VER
			buf[1] = LORA_BANDWIDTH_SHIKE; //BW	 //带宽
			#else
			buf[1] = LORA_BANDWIDTH; //BW	 //带宽
			#endif
		}
    buf[2] = LORA_CR_4_8; //CR
    buf[3] = 1; // LowDataRateOptimize，低速率优化
    SX126xWriteCommand(RADIO_SET_MODULATIONPARAMS, buf, 4);

    buf[0] = (LORA_PREAMBLE_LENGTH >> 8) & 0xFF; //high
    buf[1] = LORA_PREAMBLE_LENGTH;               //low
    buf[2] = 0x00;                               //	Fixed length packet(固定长度数据包)，0x00可变
   //发射数据长度
    buf[3] = 64;//Size of the payload (in bytes) to transmit or maximum size of the payload that the receiver can accept.  

	buf[4] = CRC_TYPE;                                  //CRCType  off
    buf[5] = 0;                                  //Standard IQ setup
    SX126xWriteCommand( RADIO_SET_PACKETPARAMS, buf, 6 );

		SX126xSetDioIrqParams(RxIrqMask,RxIrqMask,IRQ_RADIO_NONE,IRQ_RADIO_NONE);//设置IO1接收中断
    SX126xClearIrqStatus(IRQ_RADIO_ALL);//清中断
  //SX126xSetRx(0);//接收模式
    SX126xSetLoRaSymbNumTimeout(0);
    SX126xSetRx(0xFFFFFF);//接收模式
}

#endif

#if USE_SX1262
/* RFM98模块处理 */
void RF_Process(void)
{
	
	uint8 i = 0;
    uint16_t irqRegs; 


	switch(iStatus_RF)
	{
	// 不处理
	case RFM98_NONE:	  			
		break;
	
/******************发送流程*******************/	
	// RFM98发送模式
	case RFM98_RX_TO_TX:				 					 	
		if(iRF_TX_Buf[0]){
		  	iStatus_RF = RFM98_DATA_TX;		
				
		}else{
		 	iStatus_RF = RFM98_TX_FAIL;			
		}
		break;
	// RFM98发送数据
	case RFM98_DATA_TX:							
		UART_PutStr("LORA-tx:");
	  	user_printf_hex(&iRF_TX_Buf[1],iRF_TX_Buf[0]);
	
		RF_send_data((uint8 *)&(iRF_TX_Buf[1]),iRF_TX_Buf[0]);
		iStatus_RF = RFM98_WAIT_TX;
		fTimer_RF_TX = 0;		
		break;
  // 等待发送
	case RFM98_WAIT_TX:			 
		break;
	// 发送数据成功
	case RFM98_TX_OK:
		SX126xClearIrqStatus(IRQ_RADIO_ALL);//清中断	
		SX126xSetStandby(STDBY_RC);		
		iStatus_RF = RFM98_TX_TO_RX;  //进入接收模式
	    break;
	// 发送数据错误
	case RFM98_TX_FAIL:	
				
        RF_Init(0, 21);
		iStatus_RF = RFM98_TX_TO_RX;	
		break;
/******************接收流程*******************/		
	// RFM98模块转接收模式
	case RFM98_TX_TO_RX:	
		RF_ready_to_rx();  //设置成接收模式
		iStatus_RF = RFM98_DATA_RX;	 //下一个状态
		iTimer_RFM98_nRX = 0;  
		break;
	// RFM98接收数据
	case RFM98_DATA_RX:
			//return;
	
			//if (RF_IRQ_PIN)
			if(READ_RF_IRQ())
			{ // 收到数据高电平中断  //有中断
					irqRegs = SX126xGetIrqStatus(); //获取中断标志
					if ((irqRegs & IRQ_RX_DONE) == IRQ_RX_DONE)
					{//接收完成中断
						if (SX126xGetPayload(&iRF_RX_Buf[0], &i, 64) == 0)
						{//获取有效数据
							if ((i > 0) && i <= 64)
							{
									UART_PutStr("LORA-Rx:");
								  user_printf_hex(&iRF_RX_Buf[0],i);

									if (iRF_RX_Buf[0] < 64)   
									{ // 帧长度正确处理    
											App_Rec_Parser(iRF_RX_Buf);
									}
							}
						}
					}
					SX126xClearIrqStatus(IRQ_RADIO_ALL);//清中断

					if(iStatus_RF == RFM98_DATA_RX)
					{
							iStatus_RF = RFM98_TX_TO_RX;  //重新进入等待接收模式
					} 
			}
			
#if (EN_LORA_SLEEP > 0)						
        if (fIsNeedSleep()) //需要休眠
        {
            iStatus_RF = RFM98_RX_TO_SLEEP;
        }
#endif			
        break;
				
				

#if (EN_LORA_SLEEP > 0)			
	case RFM98_RX_TO_SLEEP:  //休眠模式
		iTimer_RFM98_nRX = 0;
		iStatus_RF = RFM98_SLEEP;
	
		SX126xSetStandby(STDBY_RC);	
		RadioSleep();	// RFM96_Sleep();                    //Sleep
	
		//dPrintf_Lora_Log(WHITE,"LORA_ENTER_SLEEP\r\n"); 	
		break;
	case RFM98_SLEEP:			
		break;
	case RFM98_QUITE_SLEEP:
		//dPrintf_Lora_Log(WHITE,"LORA_QUIT_SLEEP\r\n"); 
		iStatus_RF = RFM98_TX_TO_RX;
		break;
#endif
	
    default:			
		break;
	}
}

/**
 * @description: LoRa 发射等待超时 10ms 跑一次
 * @param {*}
 * @return {*}
 */
void rf_wait_tx_timeout(void)
{
    if (iStatus_RF == RFM98_WAIT_TX)
    {
        if(READ_RF_IRQ())
        {                             // Packet send over 发送完成了IRQ 变为H,平时L ----持续时间
            iStatus_RF = RFM98_TX_OK; // 数据发送成功处理
        }
        else
        {
            if (++fTimer_RF_TX >= 500)
            { // 5秒超时错误处理
                fTimer_RF_TX = 0;
                iStatus_RF = RFM98_TX_FAIL;
            }
        }
    }
}

#endif

#endif

