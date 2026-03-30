#include	"proto.h"
#include	"user_json.h"

#if SET_PROTO_VER==PROTO_VER_ZH202
u32  UnixTimestamp;

char *DEVICE_VER = "20260123";
u32  g_nbiot_last_read_time = 0x00;


uint8_t		g_tran_gprs_buf[NET_TRAN_SIZE] = { 0x00,0x00 ,0x00 ,0x00  };
st_pool		g_tran_gprs = {g_tran_gprs_buf, NET_TRAN_SIZE, 0 };

#ifdef SET_IP
bit g_IP_PORT_Change;
#endif

#ifdef	SET_SK_ROUTE
U8X g_bMac[17] = {'0','0',':','0','0',':','0','0',':','0','0',':','0','0',':','0','0'};
U8X g_bRealm[20];
bit g_isReRoute;
#endif

#ifdef	SET_CH395
extern void CH395SendData(UINT8 sockindex,UINT8 *databuf,UINT16 len);
#endif


CbNet_InitTypeDef  g_cbNet;





/**
 * @description: 获取网络包校验
 * @param {type} 
 * @return: 
 */
u8 Check_get_net_pack(u8 *Dat,u16 Len)
{	
	u16 i;
	u8 ReturnValue = 0;
	for(i = 0;i < Len; i++)
	{
		if(i != NET_SITE_CHK)
		{
			ReturnValue ^= Dat[i];
		}
		
	}
	return(ReturnValue);
}


u8 IsServerFail(void)
{
	#if SET_GATEWAY
	if(is_Reg_Net==0)
	{
		g_cbNet.server1Status = 0;
	}
	#endif
	
	//只要上线过即可
	if(g_cbNet.server1Status == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
//#if SET_GATEWAY
#if 1
u8	DEVICE_EEPROM[SMART_DEVICE_NUM*8];
u16 pGetListNum(void)
{
	u16 i;
	u16 Num = 0;
	u8 uid[6];
	memset(uid,0xff,6);
	for(i = 0; i < SMART_DEVICE_NUM; i++)
	{
		if(memcmp(&DEVICE_EEPROM[i * 8],uid, 6) != 0)
		{
			Num++;
		}
	}
	return Num;
}
u8 pGetListCheck(u16 bNum)
{
	u8 bCheck = 0;
	u8 channel,i;
	if(bNum==0)
	{
		return 0;
	}
	else
	{
		for(channel=0;channel<bNum;channel++)
		{
			for(i=0;i<8;i++)
			{
				bCheck += DEVICE_EEPROM[channel * 8 +i];
			}
		}
		return bCheck;
	}
}


/**
* @brief  智能设备是否学习
* @param  0不存在，其他第几个
* @retval None
*/
u16 IsExist_202(uint8 *uid)
{
	u16 i;
		
	for(i = 0; i < SMART_DEVICE_NUM; i++)
	{
		if (memcmp(&DEVICE_EEPROM[i * 8],uid, 6) == 0)
		{
			return i + 1;
		}	
	}
	return 0;
}

/**
* @brief  删除智能设备
* @param  探测器路数（0-39）
* @retval 
*/
void delete_smart_device(u16 No)
{
	u8 buf[8];
	if(No >= SMART_DEVICE_NUM) return;
    
  memset(buf,0xff,8);
  memcpy(&DEVICE_EEPROM[No*8],buf,8);
}

u8 cust_del_smart_device(u16 channal)
{
	bool isOk = false;
//	u8 buf[1];
//	u16 i;
	if(channal == 0xffff)
	{
		memset(&DEVICE_EEPROM,0xff,sizeof(DEVICE_EEPROM));
		isOk = true;
	}
	if(channal < SMART_DEVICE_NUM && channal >= 1) 
	{
		isOk = true;
		delete_smart_device(channal - 1);
	}
	return isOk;
}
#else
u16 pGetListNum(void)
{
	return 1;
}
u8 pGetListCheck(u16 bNum)
{
	u8 bCheck = 0;
	u8 i;

	for(i=0;i<6;i++)
	{
		bCheck += g_device_info.s_cpu_id[i];
	}
	bCheck += 0xfa;
	bCheck += DEVICE_ID;

	return bCheck;
	
}
#endif


/**
 * @description: 获取网络包数据长度
 * @param {type} 
 * @return: 
 */
u16 net_get_data_len(u8 *buf)
{
	u16 len = (buf[NET_SITE_CT] & 0x03);
	len = (len * 256) + buf[NET_SITE_CT + 1];
	return len;
}

/**
 * @description: 给发送的包中设置长度
 * @param {type} 
 * @return: 
 */
void net_set_data_len(u8 *buf, u16 len)
{
	buf[NET_SITE_CT] = buf[NET_SITE_CT] | ((len >> 8) & 0x03);
	buf[NET_SITE_CT + 1] = (len & 0xFF);
}
/**
 * @description: 获取包类型
 * @param {type} 
 * @return: 
 */
u8 net_get_ct(u8 *buf)
{
	return ((buf[NET_SITE_CT] & 0x7F) >> 2);
}

/**
 * @description: 组合两字节
 * @param {type} 
 * @return: 
 */
u16 custom_comb_2byte(u8 dat1, u8 dat2)
{
	u16 dat = dat1;
	dat = (dat << 8) + dat2;
	return dat;
}

/**
 * @description: 获取包唯一标识
 * @param {type} 
 * @return: 
 */
u16 net_get_sn(u8 *rx_buf)
{	
	return custom_comb_2byte(rx_buf[NET_SITE_SN],rx_buf[NET_SITE_SN+1]);
}


/**
 * @description: 创建包头
 * @param 
 * @return: 
 */
u8 net_create_head_new(u8 *Buf, u16 sn,u8 flag ,u8 pack_type)
{
	u8 i;
	u8 len = 0;
	u8 net_ch = 0;
	//if(g_cbNet.m_currentNet == NET_WIRED)
	{
		#if SET_CAT1
		net_ch = NET_TYPE_GPRS;
		#else
		net_ch = NET_TYPE_WIRE;
		#endif
	}
	//else net_ch = NET_TYPE_WIFI;
	
	Buf[len++] = NET_HEAD_FF;//魔术字0xFF
	Buf[len++] = NET_PN;//协议号
	Buf[len++] = (NET_V << 3) | net_ch;//版本和 通信通道
	Buf[len++] = 0;//校验
	Buf[len++] = sn >> 8;//包唯一序列号
	Buf[len++] = sn;//包唯一序列号
	Buf[len++] = flag;//
	Buf[len++] = (pack_type<<2);//CMD&LEN
	Buf[len++] = 0;//CMD&LEN
	//Buf[len++] = DEVICE_ID;
	Buf[len++] = DEVICE_ID_202;
	for(i=0;i<6;i++)
	{
		Buf[len++] = gMCUID[i];
	}
	return len;
}


void net_tx_printf_json(u16 len)
{
	net_printf("net发送,SN:%d ,LEN:%d---",net_get_sn(g_cbNet.TxBuf),len);

	if(net_get_ct(g_cbNet.TxBuf) == CT_JSON_DATA || net_get_ct(g_cbNet.TxBuf) == CT_HEARTBEAT)
	{
#if (DEBUG_NET > 0)
		printt_json((const char *)&g_cbNet.TxBuf[NET_SITE_DATA_START],net_get_data_len(g_cbNet.TxBuf));
#endif
	}
	
	if(net_get_ct(g_cbNet.TxBuf) == CT_HEX_DATA)
	{
#if (DEBUG_NET > 0)
		printfHex(&g_cbNet.TxBuf[NET_SITE_DATA_START],net_get_data_len(g_cbNet.TxBuf));
#endif		
	}
		
	
}

/*
* @brief:   发送bLen字节udp包
* @paramp   
* @return
*/
void net_send_data(u8 net_ch,u16 bLen)
{
	net_tx_printf_json(bLen);		
	
	g_cbNet.TxLen = bLen;
	#ifdef	SET_CH395
	CH395SendData(net_ch,g_cbNet.TxBuf,bLen);
	#endif

	//p_air720_port_send_data(net_ch,g_cbNet.TxBuf,bLen);
	g_cbNet.TxLen = bLen;
	p_my_pool_push_ch(&g_tran_gprs,g_cbNet.TxBuf,bLen,0x01);
}


/**
 * @description: 回复包
 * @param {type} 
 * @return: 
 */
u8 net_ack(u8 ch, u16 sn,u8 pack_type)
{
	u8 len = 0;

	u8 i;
	
	u8 net_ch = 0;
	g_cbNet.TxBuf[len++] = NET_HEAD_FF;//魔术字0xFF
	g_cbNet.TxBuf[len++] = NET_PN;//协议号
	g_cbNet.TxBuf[len++] = (NET_V << 3) | net_ch;//版本和 通信通道
	g_cbNet.TxBuf[len++] = 0;//校验
	g_cbNet.TxBuf[len++] = sn >> 8;//包唯一序列号
	g_cbNet.TxBuf[len++] = sn;//包唯一序列号
	g_cbNet.TxBuf[len++] = NET_FLAG_ACK | NET_FLAG_END_PACK;
	g_cbNet.TxBuf[len++] = (pack_type<<2);//CMD&LEN
	g_cbNet.TxBuf[len++] = 0;//CMD&LEN
	g_cbNet.TxBuf[len++] = DEVICE_ID_202;
	for(i=0;i<6;i++)
	{
		g_cbNet.TxBuf[len++] = gMCUID[i];
	}
	g_cbNet.TxBuf[NET_SITE_CHK] = Check_get_net_pack(g_cbNet.TxBuf,len);//校验
	#ifdef	SET_CH395
	CH395SendData(ch,g_cbNet.TxBuf,len);
	#endif
	//p_air720_port_send_data(ch,g_cbNet.TxBuf,len);
	g_cbNet.TxLen = len;
	p_my_pool_push_ch(&g_tran_gprs,g_cbNet.TxBuf,len,0x01);
	return len;
}

void net_proto_heartbeat_send(void)
{
	u16 tx_len = 0;
	u8 buf[10];
	u32 list_array[2];
	//u16 check = 0;
	memset(buf,0x00,sizeof(buf));
	net_printf("json heartbeat\r\n");

	tx_len += net_create_head_new(&g_cbNet.TxBuf[0],g_cbNet.sn_up[0]++,\
	NET_FLAG_M | NET_FLAG_END_PACK , CT_HEARTBEAT);

	/* 编码接收 操作码 */
	tx_len += user_json_encode_key_val((char*)&g_cbNet.TxBuf[tx_len],KEY_OP_CODE,CMD_HEARTBEAT,JSON_SITE_FIRST);

	//if(g_device_info.flagDectAc)
	
	#if SET_BAT_WORK
	buf[0] = 0;
	#else
	if(g_cbSta.bAcSta==AC_ON)
	{
		buf[0] = 1;
	}
	else
	{
		buf[0] = 0;
	}
	#endif
	
	if((g_cbSta.bBatVal%100)>=50)
	{
		buf[1] = g_cbSta.bBatVal/100+1;	//电池电压 
	}
	else
	{
		buf[1] = g_cbSta.bBatVal/100;
	}
	
	/* 编码接收 主机状态，数组。[0]=市电、[1]=电池、[2]=布撤防、[3]=报警 */
	tx_len += user_json_encode_key_array((char*)&g_cbNet.TxBuf[tx_len],KEY_STATUS,buf,4,JSON_SITE_NO_FIRST_LAST,NULL);
	
	#if 0
	/*+CSQ,31,99	取值 1 - 31	分4级 每级大约是7*/
	// if(is_Reg_Net)
	// {
		// if((g_run_info.m_csq >= 30) && (g_run_info.m_csq != 99))
		// {
				// buf[0] = 5;
		// }
		// else if((g_run_info.m_csq >= 22) && (g_run_info.m_csq < 30))
		// {
				// buf[0] = 4;
		// }
		// else if((g_run_info.m_csq >= 15) && (g_run_info.m_csq < 22))
		// {
				// buf[0] = 3;
		// }
		// else if((g_run_info.m_csq >= 8) && (g_run_info.m_csq < 15))
		// {
				// buf[0] = 2;
		// }
		// else
		// {
				// buf[0] = 1;
		// }
	// }
	// else
	{
		buf[0] = 0;
	}
	
	// if(g_cbWifiSta.isWifiLink)
	// {
		// if(g_cbWifiSta.rssi>90)
		// {
			// buf[1] = 5;
		// }
		// else if(g_cbWifiSta.rssi>70)
		// {
			// buf[1] = 4;
		// }
		// else if(g_cbWifiSta.rssi>50)
		// {
			// buf[1] = 3;
		// }
		// else if(g_cbWifiSta.rssi>30)
		// {
			// buf[1] = 2;
		// }
		// else 
		// {
			// buf[1] = 1;
		// }
	// }
	// else
	{
		buf[1] = 0;
	}	
	/* 编码接收 [0]=GRPS信号强度，0~5；[1]=WIFI信号强度，0~5 */
	tx_len += user_json_encode_key_array((char*)&g_cbNet.TxBuf[tx_len],KEY_SINGAL,buf,2,JSON_SITE_NO_FIRST_LAST,NULL);
	/* 编码接收 主机软件版本号，字符串。操作码 */
	tx_len += user_json_encode_key_string((char*)&g_cbNet.TxBuf[tx_len],KEY_VER,DEVICE_VER,true,JSON_SITE_NO_FIRST_LAST);
	#endif

	/* 编码接收 主机软件版本号，字符串。操作码 */
	tx_len += user_json_encode_key_string((char*)&g_cbNet.TxBuf[tx_len],KEY_VER,DEVICE_VER,true,JSON_SITE_NO_FIRST_LAST);
	
	//增加白名单校验码
	// LOG("WL:");
	// PrintHexStr(&DEVICE_EEPROM[0],sizeof(DEVICE_EEPROM));
	
	list_array[0] = pGetListNum();
	buf[0] = list_array[0];
	list_array[1] = pGetListCheck(buf[0]);

	tx_len += user_json_encode_mul_key_u16_array((char*)&g_cbNet.TxBuf[tx_len],KEY_LIST,list_array,2,JSON_SITE_LAST,NULL);

	/* 设置 data 长度 */
	net_set_data_len(g_cbNet.TxBuf,tx_len - NET_SITE_DATA_START);
	g_cbNet.TxBuf[NET_SITE_CHK] = Check_get_net_pack(g_cbNet.TxBuf,tx_len);//校验
	
	net_send_data(0,tx_len);
}

/**
* @brief:重连，连接服务器 发送心跳包
* @param
* @return
*/
void Login_Heart_package_snd(void)
{
	net_proto_heartbeat_send();
	//pRetPluse(TYPE_PLUSE_ALL,0,net);
}

/**
* @brief 请求同步时间
* @param  
* @retval None
*/
void net_proto_req_sync_time(void)
{
	u16 tx_len = 0;
	char buf[20];
	net_printf("req sync time\r\n");

	memset(buf,0x00,sizeof(buf));
	tx_len += net_create_head_new(&g_cbNet.TxBuf[0],g_cbNet.sn_up[0]++,\
	NET_FLAG_M | NET_FLAG_END_PACK , CT_JSON_DATA);

	/* 编码接收 操作码 */
	tx_len += user_json_encode_key_val((char*)&g_cbNet.TxBuf[tx_len],KEY_OP_CODE,CMD_GET_SYNC_TIME,JSON_SITE_FIRST);

	//sprintf(buf,"%lld",(u64)UnixTimestamp);
	sprintf(buf,"%u",UnixTimestamp);
	/* 编码接收 主机软件版本号，字符串。操作码 */
	tx_len += user_json_encode_key_string((char*)&g_cbNet.TxBuf[tx_len],KEY_TIME_TIME,buf,false,JSON_SITE_LAST);
	
	/* 设置 data 长度 */
	net_set_data_len(g_cbNet.TxBuf,tx_len - NET_SITE_DATA_START);
	g_cbNet.TxBuf[NET_SITE_CHK] = Check_get_net_pack(g_cbNet.TxBuf,tx_len);//校验
	
	net_send_data(0,tx_len);


	//启用重发机制	
	g_run_info.m_flg_sending = TRUE;	
	g_run_info.m_last_nbiot_send_time = my_time();
	#if SET_CAT1
	g_cbNet.ReTxLen = g_cbNet.TxLen;
	memcpy(g_cbNet.ReTxBuf,g_cbNet.TxBuf,g_cbNet.ReTxLen);
	#endif
}

void net_heart_gateway(void)
{
	static u8 heart_count1 = 0;
	if(g_cbNet.hreadbeat_timer1 >= NET_HEARTBEAT_TIME)
	{ 	
		//离线的情况下先只发心跳包
		if(IsServerFail())
		{
			//g_cbNet.hreadbeat_timer1 = 0;
			//离线时每5s一条心跳上报,加快上线速度
			g_cbNet.hreadbeat_timer1 = NET_HEARTBEAT_TIME-5;
			
			// p_my_pool_pop(&g_net_resend_pool); 
			// p_my_pool_pop(&g_tran_up_pool);
			Login_Heart_package_snd();
			
			heart_count1 = 2;  //上线后的下一条同步时间
		}
		else
		{
			g_cbNet.hreadbeat_timer1 = 0;
			
			if (++heart_count1 > 2)
			{ //发送2条心跳，一条同步时间
				heart_count1 = 0;
				net_proto_req_sync_time();
			}
			else
			{				
				Login_Heart_package_snd();
			}				
		}		
	}
}

/**
 * @description: 心跳定时发送，中间穿插网络同步时间
 * @param ch 0-xx
 * @return: 
 */
void net_heart_timer_send(void)
{
	net_heart_gateway();
	//net_heart_update(0,g_cbNet.m_currentNet);
}

/**
 * @description: 错误码上报
 * @param {type} 
 * @return: 
 */
u8 net_err_ack(u8 ch, u16 sn, u8 err,u8 pack_type)
{
	u8 i;
	u8 len = 0;
	u8 net_ch = 0;
	
	//net_ch = g_cbNet.m_currentNet;

	#if SET_CAT1
	net_ch = NET_TYPE_GPRS;
	#else
	net_ch = NET_TYPE_WIRE;
	#endif
	
	g_cbNet.TxBuf[len++] = NET_HEAD_FF;//魔术字0xFF
	g_cbNet.TxBuf[len++] = NET_PN;//协议号
	g_cbNet.TxBuf[len++] = (NET_V << 3) | net_ch;//版本和 通信通道
	g_cbNet.TxBuf[len++] = 0;//校验
	g_cbNet.TxBuf[len++] = sn >> 8;//包唯一序列号
	g_cbNet.TxBuf[len++] = sn;//包唯一序列号
	g_cbNet.TxBuf[len++] = NET_FLAG_ERR | NET_FLAG_END_PACK | err;
	g_cbNet.TxBuf[len++] = (pack_type<<2);//CMD&LEN
	g_cbNet.TxBuf[len++] = 0;//CMD&LEN
	g_cbNet.TxBuf[len++] = DEVICE_ID_202;
	for(i=0;i<6;i++)
	{
		g_cbNet.TxBuf[len++] = gMCUID[i];
	}
	g_cbNet.TxBuf[NET_SITE_CHK] = Check_get_net_pack(g_cbNet.TxBuf,len);//校验
	//net_send_data(ch,len);
	//net_send_data_ack(ch,len);
	#ifdef	SET_CH395
	CH395SendData(ch,g_cbNet.TxBuf,len);
	#endif

	p_my_pool_push_ch(&g_tran_gprs,g_cbNet.TxBuf,len,0x01);
	//p_air720_port_send_data(ch,g_cbNet.TxBuf,len);
	
	return len;
}

#if SET_GATEWAY
/**
 * @description: 透传数据上行
 * @param {type} 
 * @return: 
 */
void net_tran_data_up(void)
{
	u8 *buf = NULL;
	u16 tran_len = 0;
	u16 tx_len = 0;
	u16 bPos = 0;

	//正在等待应答
	if(g_run_info.m_flg_sending==TRUE)
	{
		return;
	}
	
	
	if((buf = p_my_pool_peek(&g_tran_up_pool, &tran_len)) != NULL)
	{
		tx_len += net_create_head_new(&g_cbNet.TxBuf[0],g_cbNet.sn_up[0]++,\
		NET_FLAG_M | NET_FLAG_END_PACK , CT_HEX_DATA);
		
		//合并包上报
		while((buf = p_my_pool_peek(&g_tran_up_pool, &tran_len)) != NULL)
		{
			if(tran_len + tx_len<620)			
			{
				if(buf[0]==CMD_LORA_DATA)
				{
					net_printf("Net tran up:%d\r\n",tran_len);
					memcpy(&g_cbNet.TxBuf[NET_SITE_DATA_START+bPos],buf,tran_len);
					tx_len += tran_len;
					bPos += tran_len;					
					p_my_pool_pop(&g_tran_up_pool);//出队
				}
			}	
			else
			{
				p_my_pool_clear(&g_tran_up_pool);
				break;
			}
			p_my_pool_pop(&g_tran_up_pool);//出队
		}
		
		/* 设置 data 长度 */
		net_set_data_len(g_cbNet.TxBuf,tx_len - NET_SITE_DATA_START);
		g_cbNet.TxBuf[NET_SITE_CHK] = Check_get_net_pack(g_cbNet.TxBuf,tx_len);//校验
			
		LOG("tx_len:%d",tx_len);
		
		if(g_bLedR==0)
			g_bLedR = 5*10;
		net_send_data(0,tx_len); 
		g_run_info.m_flg_sending = TRUE;
		g_run_info.m_last_nbiot_send_time = my_time();
	}
	
}
#endif

void net_data_up(void)
{
	u8  buf[100];
	u16 tran_len = 0;
	u16 tx_len = 0;
	u16 bPos = 0;

	if(g_readySendBuf[SITE_CMD])
	{
		//bPos = 3;
		memset(buf, 0x00, sizeof(buf));
		
		//lora协议
		buf[2+1] = g_readySendBuf[SITE_LEN] + LEN_TYPE_LORA; //帧长度
		buf[2+0] = buf[2+1] + 1;
		buf[2+2] = ++RF_Num;//通信表标识	
		EEPROM_Ptr[RF_NUM_ADDR] = RF_Num;
		user_fs_updata_memory();
		memcpy(&buf[2+3], g_device_info.s_cpu_id, 6);
		buf[2+9] = 0xfa;
		buf[2+10] = g_readySendBuf[SITE_CMD];		 //命令编码
		//数据
		memcpy(&buf[2+11], &g_readySendBuf[SITE_DATA_STAT], g_readySendBuf[SITE_LEN]);
		buf[2+buf[2+1] + 1] = LoRaProtoUpCheck(buf+2);
		

		//套hex壳
		//数据
		buf[0] =  CMD_LORA_DATA;
		buf[1] =	0;
		buf[2] =	buf[2+0];
		//信号
		buf[buf[2+0]+3] = CMD_LORA_SIG;
		buf[buf[2+0]+4] = 0;
		buf[buf[2+0]+5] = 2;
		buf[buf[2+0]+6] = 0;
		buf[buf[2+0]+7] = 100;
		
		tran_len = buf[2+0]+8;

		//套202壳
		tx_len += net_create_head_new(&g_cbNet.TxBuf[0],g_cbNet.sn_up[0]++,\
		NET_FLAG_M | NET_FLAG_END_PACK , CT_HEX_DATA);
	
		net_printf("Net tran up:%d\r\n",tran_len);
		memcpy(&g_cbNet.TxBuf[NET_SITE_DATA_START+bPos],buf,tran_len);
		tx_len += tran_len;
		bPos += tran_len;					
			
		g_readySendBuf[SITE_CMD] = 0;
		
		
		/* 设置 data 长度 */
		net_set_data_len(g_cbNet.TxBuf,tx_len - NET_SITE_DATA_START);
		g_cbNet.TxBuf[NET_SITE_CHK] = Check_get_net_pack(g_cbNet.TxBuf,tx_len);//校验
			
		LOG("tx_len:%d",tx_len);
		
		net_send_data(0,tx_len); 
		g_run_info.m_flg_sending = TRUE;
		g_run_info.m_last_nbiot_send_time = my_time();
		
		#if SET_CAT1
		g_cbNet.ReTxLen = g_cbNet.TxLen;
		memcpy(g_cbNet.ReTxBuf,g_cbNet.TxBuf,g_cbNet.ReTxLen);
		#endif
	}
}


void NB_TxPro(void)
{
	u8 *buf = NULL;
	u16 tran_len;
	if(g_run_info.m_flg_retry)
	{
		net_printf("重发:%d\r\n",g_run_info.m_last_nbiot_resend_count);
		g_run_info.m_flg_retry = FLASE;	
		#ifdef	SET_CH395
		CH395SendData(0,g_cbNet.TxBuf,g_cbNet.TxLen);
		#endif
		
		#if SET_CAT1
		p_air720_port_send_data(0,g_cbNet.ReTxBuf,g_cbNet.ReTxLen);
		#endif
		g_run_info.m_flg_sending = TRUE;
		g_run_info.m_last_nbiot_send_time = my_time();
	}
	else
	{	
		#if 0
		//事件上报
		//在线
		//不需要同步时间
		if(IsServerFail())
		{
			net_heart_timer_send();
		}
		else
		#endif
		
		{
			service.m_proto_set = PROTO_ZH202;
			fDealEventQueuePro();
			net_data_up();
		}

		if((buf = p_my_pool_peek(&g_tran_gprs, &tran_len)) != NULL)
		{
			if(FLASE==p_air720_port_send_data(0,&buf[1],tran_len-1))
			{
				LOG("4G SEND FAIL\r\n");
				Sleep_ms(100);

				//p_air720_set_ATE(0);
				if(FLASE==p_air720_port_send_data(0,&buf[1],tran_len-1))
				{
					g_run_info.m_nbiot_create_sock_ok = FLASE;
				}					
			}
			p_my_pool_pop(&g_tran_gprs);
		}

		//net_proto_resend_deal();//处理重发	
		#if SET_GATEWAY
		net_tran_data_up();
		#endif
	}
}

/**
 * @description: 同步网络时间
 * @param {type}
 * @return: 
 */
void net_sync_time(u8 ch,u8 *rx_buf)
{
	u32 temp_time;
	u16 json_len = net_get_data_len(rx_buf);//json的长度
	net_printf("Sync time:");
	if(user_json_get_int_from_key((const char *)&rx_buf[NET_SITE_DATA_START], json_len, KEY_TIME_TIME, &temp_time))
	{
		UnixTimestamp = temp_time;
		net_printf("%d\r\n", UnixTimestamp);

		#if SET_CAT1
		g_device_info.UnixTimestamp = UnixTimestamp;
		#endif
		
		net_ack(ch, net_get_sn(rx_buf),net_get_ct(rx_buf));//返回数据

		//net_proto_clear_resend_cmd(CMD_GET_SYNC_TIME);//清楚重发队列请求同步时间
		g_run_info.m_flg_sending = FLASE;	//去重
	}
}

/**
 * @description: 接卸json数据
 * @param 
 * @return: 
 */
void net_deal_json(u8 ch,u8 *rx_buf,u16 rx_len)
{
	u16 json_len = net_get_data_len(rx_buf);
	u8 cmd;
	net_printf("deal json:%d\r\n",json_len);
	printt_json((const char *)&rx_buf[NET_SITE_DATA_START],json_len);
	
	
	if(user_json_is_json((const char *)&rx_buf[NET_SITE_DATA_START], json_len) == false)
	{
		net_printf("json fmt error\r\n");
		net_err_ack(ch, custom_comb_2byte(rx_buf[NET_SITE_SN],rx_buf[NET_SITE_SN+1]), ERR_JSON_ERR,rx_buf[NET_SITE_CT]>>2);
	}	
	
  	else if(user_json_get_val_from_key((const char *)&rx_buf[NET_SITE_DATA_START], json_len, KEY_OP_CODE ,&cmd))
	{
		net_printf("命令:%02X",cmd);
		switch (cmd)
		{
		case CMD_ACK_SYNC_TIME: 	// 0X8A	//回复同步时间
			net_sync_time(ch, rx_buf);
				
			break;
		
		// #if SET_JSON_PROTO==0
		// case CMD_RESET_DEF:			// 0X0F	//请求恢复出厂设置(应以0X03指令回复)
			// net_proto_reset_def(ch, rx_buf, rx_len);
			// break;
		// #endif
		default:
			break;
		}
	}
	else
	{
		net_printf("没有找到命令\r\n");
	}
}


/*
	透传转发十六进制协议
*/
u16 net_get_tlv_len(u8 *buf)
{
	u16 len;
	len = buf[1];
	len = (len * 256) + buf[2];
	return len;
}

void net_deal_hex(u8 ch,u8 *rx_buf,u16 rx_len)
{
	u8 *buf = NULL;
	u16 len = 0;

	u16 hex_len = net_get_data_len(rx_buf);
	u16 bPos = 0;
	u16 bLen = 0;
	//u8 bCadType = 0;
	u16 bDeviceCh = 0;
	u8 UpUid[7] = {0};
	
	u16 i;

	net_printf("deal hex:%d\r\n",hex_len);
	printt_hex(&rx_buf[NET_SITE_DATA_START],hex_len);	
	
	//TLV解析接收数据
	while(hex_len>2)
	{	
		bLen = net_get_tlv_len(&rx_buf[NET_SITE_DATA_START+bPos]);
		switch(rx_buf[NET_SITE_DATA_START+bPos])
		{
			
			case CMD_WAKE_DELAY:
				net_printf("CMD_WAKE_DELAY\r\n");
				// memcpy(UpUid, &rx_buf[NET_SITE_DATA_START+bPos+3],6);
				// bDeviceCh = IsExist_202(UpUid);
				// if(bDeviceCh)
				// {
					// Wake_list[bDeviceCh-1] = 1;
				// }
				break;
			case CMD_LORA_DATA:
				net_printf("CMD_LORA_DATA\r\n");
			
//				if(g_bLedB==0)
//					g_bLedB = 5*10;	
				
//				GateWay_Rec_Net_Lora(&rx_buf[NET_SITE_DATA_START+bPos+3]);
				break;
			//暂时不增加唤醒功能
			case CMD_LORA_CAD:		
				net_printf("CMD_LORA_CAD\r\n");
				break;
			//暂时不切换频率
			case CMD_LORA_FRE:
				net_printf("CMD_LORA_FRE\r\n");
				break;			
			case CMD_LIST_UID:
				net_printf("CMD_LIST_UID\r\n");
				break;
			case CMD_LIST_CLR:
				net_printf("CMD_LIST_CLR\r\n");
				//#if SET_GATEWAY
				#if 1
				cust_del_smart_device(0xffff);
				#endif
				break;
			case CMD_LIST_DEL:
				net_printf("CMD_LIST_DEL\r\n");
				//#if SET_GATEWAY
				#if 1
				//遍历UID删除
				for(i=0;i<bLen/7;i++)
				{
					memcpy(UpUid, &rx_buf[NET_SITE_DATA_START+bPos+3+8*i],7);
					bDeviceCh = IsExist_202(UpUid);
					if(bDeviceCh)
					{
						cust_del_smart_device(bDeviceCh);
					}
				}				
				#endif
				break;
			case	CMD_LIST_ADD:
				net_printf("CMD_LIST_ADD\r\n");
				
				//#if SET_GATEWAY
				#if 1
				//遍历UID添加
				for(i=0;i<bLen/8;i++)
				{
					memcpy(UpUid, &rx_buf[NET_SITE_DATA_START+bPos+3+8*i],7);
					bDeviceCh = IsExist_202(UpUid);
					
					//遍历UID为不存在的新UID
					if(bDeviceCh==0)
					{
						//找到空的位置
						memset(UpUid,0xff,7);
						bDeviceCh = IsExist_202(UpUid);
						//net_printf("CMD_LIST_ADD:%d\r\n",bDeviceCh);
						if(bDeviceCh==0)
						{
							bDeviceCh = 1;
						}
						memcpy(&DEVICE_EEPROM[(bDeviceCh-1) * 8],&rx_buf[NET_SITE_DATA_START+bPos+3+i*8],8);
					}
				}	
				#endif
				// LOG("WL:");
				// PrintHexStr(&DEVICE_EEPROM[0],sizeof(DEVICE_EEPROM));
				break;		
			default:
				break;
		}
		bPos += bLen+3;
		if(hex_len>bLen+3)
		{		
			hex_len -= bLen+3;
		}
		else
		{
			hex_len = 0;
		}
	}

	net_ack(ch, net_get_sn(rx_buf),net_get_ct(rx_buf));//返回数据
}	

void NET_RxdDataPro(u8 *Buf,u16 len)
//void NET_RxdDataPro(void)	
{
	u8 proto_type = 0;
	{
#if DEBUG_NET > 0
		net_printf("net接收,SN:%d ,LEN:%d---",net_get_sn(Buf),len);
		//升级包数据不打印
		if(len<512)
		{
			user_printf_hex(Buf, len);
		}
		else
		{
			user_printf_hex(Buf, 20);
		}
		len +=	1;
#endif

		{			
			g_cbNet.sn_down = custom_comb_2byte(Buf[NET_SITE_SN],Buf[NET_SITE_SN+1]);//下行包唯一序号
			
			if(Buf[NET_SITE_DI] == DEVICE_ID_202 && memcmp(&Buf[NET_SITE_DUID],&gMCUID[0],6) == 0 && \
			Buf[NET_SITE_CHK] == Check_get_net_pack(Buf,len-1))
			{//新协议
				proto_type = PROTOCOL_NEW;
			}
			#if 0
			else if(Buf[0] == DEVICE_ID_236 && memcmp(&Buf[1],&gMCUID[0],6) == 0 && Buf[len - 2] == fCheck9120(Buf,len-2,1))
			{//旧协议，远程升级用
				proto_type = PROTOCOL_OLD;
			}
			#endif
			else
			{
				net_printf("服务器发错/校验错误\r\n");
				// net_deal_json(g_NetSndCh,Buf,len-1);
				net_err_ack(0, custom_comb_2byte(Buf[NET_SITE_SN],Buf[NET_SITE_SN+1]), ERR_HEARD_ERR,Buf[NET_SITE_CT]>>2);
				return;
			}
			
			#if 0
			if(proto_type == PROTOCOL_OLD)
			{// 旧协议远程升级使用，根据命令处理
				printf("旧协议,%02X\r\n",Buf[POS_NET_CMD] & 0x7f);
				if((g_cbNet.server2Status[g_NetSndCh%2] == 0)&&(g_cbNet.m_currentNet!=NET_GPRS))
				{
					g_cbNet.server2Status[g_NetSndCh%2] = 1;//服务1通信正常,需要控电服务正常才通讯正常	
				}
				else if((g_cbNet.server2Status[g_NetSndCh%2+2] == 0)&&(g_cbNet.m_currentNet==NET_GPRS))
				{
					g_cbNet.server2Status[g_NetSndCh%2+2] = 1;//服务1通信正常,需要控电服务正常才通讯正常	
				}

				switch (Buf[POS_NET_CMD] & 0x7f)
				{
				#if SET_OTA_REMOTE
				case CMD_NET_IAP://升级包
					RemoteUpgrade(Buf);
					break;
				#endif
				case CMD_NET_PLUSE://查询心跳
					pRetPluse(Buf[POS_PLUSE_TYPE],0,g_cbNet.m_currentNet);
					break;	
				case CMD_NET_IP://设置IP
					switch (Buf[POS_NET_TYPE] >> 6)
					{
					case CMD_TYPE_CHECK:
						printf("query IP.\r\n");
						pReadIpPara(CMD_NET_IP, LEN_NET_IP, Buf);
						break;
					// case CMD_TYPE_SET:
					// 	printf("set IP.\r\n");
					// 	pSetIpPara(CMD_NET_IP, LEN_NET_IP, Buf);
					// 	break;
					default:
						break;
					}
					break;	
				//查询独立参数，只回复有线网卡参数
				case CMD_NET_PAR: //设置独立参数
					switch (Buf[POS_NET_TYPE] >> 6)
					{
					case CMD_TYPE_SET:
						printf("set indepdent para\r\n");
						NET_setupIndependentPar(Buf,1);
						break;
					default:
						NET_setupIndependentPar(Buf,0);
						break;
					}
					break;
				case 0x0d:
					Net_ret_para(Buf);
					break;
				case 0x0e:
					Net_ret_fq(Buf);
					break;
				default:
					Net_ret(Buf);
					//不回复
					break;
				}
			}
			else
			#endif
				
			if(proto_type == PROTOCOL_NEW)
			{//新协议		
				g_cbNet.server1Status = 1;
							
				if(Buf[NET_SITE_FLAG] & NET_FLAG_ACK)
				{//确认包
					net_printf("收到确认包\r\n");
					//net_proto_ack_deal(0,Buf,len-1);
					g_run_info.m_flg_sending = FLASE;	//去重发
				}
				else if(Buf[NET_SITE_FLAG] & NET_FLAG_ERR)
				{//错误包
					net_printf("收到错误包\r\n");
				}
				else
				{
					#if 0
					if (IsUpgradeNowFlag && upgradeOverFlag == 0)
					{ //升级中，不处理新协议
						return;
					}
					#endif
					
					net_printf("新协议\r\n");
					switch (net_get_ct(Buf))
					{
					case CT_HEARTBEAT:				//	0	//心跳包。
					case CT_CHANGE_CIPHER_SPEC: 	//	20	//更改加密规则。
					case CT_ALERT:						//	21	//告警。
					case CT_HANDSHAKE:				//	22	//握手。
						break;
					case CT_APPLICATION_DATA:		//	23	//应用数据（二进制）
						break;
					case CT_JSON_DATA:				//	24	//应用数据（UTF8编码的JSON）。
						net_deal_json(0,Buf,len-1);
						break;
					case CT_HEX_DATA:
						net_deal_hex(0,Buf,len-1);
						break;
					default:
						break;
					}
				}
			}		
		}	
	}
}

/*
	接收数据处理
*/
void pNetCut(u16 Len,u8 SocketIndex)
{
	u16 bLen;
	u16 bPos = 0;
	u16 Tmp_Len = 0;
	
	bPos = 0;
	bLen = Len;
  	while(bLen>10)
	{
		if(g_cbNet.rx_buf[NET_SITE_DI+bPos] == DEVICE_ID_202 && memcmp(&g_cbNet.rx_buf[NET_SITE_DUID+bPos],&gMCUID[0],6) == 0)
		{			
			Tmp_Len = net_get_data_len(&g_cbNet.rx_buf[bPos])+16;		
			
			if(Tmp_Len<sizeof(g_cbNet.rx_buf)-bPos)
			{
				//还是需要压入缓冲
				//p_my_pool_push_ch(&g_net_pool, &g_cbNet.rx_buf[bPos], Tmp_Len,SocketIndex);
				//直接处理，不需要加缓冲,处理后发送才加缓冲
				NET_RxdDataPro(&g_cbNet.rx_buf[bPos],Tmp_Len); /* 接收数据处理 */
			}
						
			bPos += Tmp_Len; 
			if(bLen>Tmp_Len)
			{
				bLen = bLen-Tmp_Len;
			}
			else
			{
				bLen = 0;
			}
		}
		else
		{
			bLen = 0;
		}		
	}
}
#endif
