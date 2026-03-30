#include "AIR720.h"

#if SET_CAT1

#ifdef	SET_IP

#define	AIR720_printf		LOG


u8 hextoi(u8 hexch)
{
	if ((hexch >= '0') && (hexch <= '9'))
		return hexch - '0';
	else if ((hexch >= 'A') && (hexch <= 'F'))
		return hexch - 'A' + 10;
	else if ((hexch >= 'a') && (hexch <= 'f'))
		return hexch - 'a' + 10;
	else
		return -1;
}

/**
 * @description: 当前发送缓存是否有数据， 等待重发等
 * @param {*}
 * @return {*}
 */
unsigned char note_cur_is_send(void)
{
    unsigned char ret = 0;
    
		if(g_run_info.m_flg_sending==TRUE)
		{
			ret = 1;
		}
    
    return ret;
}

u8 isVailIpInt(u8 *ip)
{
	if((ip[0]==0)&&(ip[1]==0)&&(ip[2]==0)&&(ip[3]==0))
	{
		return 0;
	}

	if((ip[0]==0xff)&&(ip[1]==0xff)&&(ip[2]==0xff)&&(ip[3]==0xff))
	{
		return 0;
	}
	return 1;
}

/*
判断是IP还是域名
简单判断 IP：有三个点。并且每位IP小于255
*/
u8 check_str_is_ip(u8 * ip_str,u8 len)
{
	u8  Tmp_ip_str[40];
	char  * Token;
	u8  Token_Count = 0;
	int  tmp = 0;
	u8  flag = TRUE;
	
	//判断长度 最短 0.0.0.0 	最长255.255.255.255
	if((len < 7) || (len > 15)) 
		return FLASE;
	
	memset(Tmp_ip_str,0x00,sizeof(Tmp_ip_str));
	memcpy(Tmp_ip_str,ip_str,len);	//因为strtok会破坏原始字符串，所以，复制给数组，不改变传入的指针的数据
/* 	PrintString1("IP_Str:");
	PrintString1(Tmp_ip_str); */
	Token = strtok((char *)Tmp_ip_str,".");
	while(Token != NULL)
	{
		Token_Count++;
		if(Token_Count <= 3)
		{
			tmp = atoi(Token);
			if((tmp > 255) || (tmp < 0))
			{
				flag = FALSE;
				break;
			}
		}
		Token = strtok(NULL,".");
		if(Token_Count >= 5)
		{
			flag = FALSE;
			break;
		}
	}
	if (Token_Count < 4)
	{
		flag = FLASE;
	}
		
	return flag;
}

/*
ip地址字符串转4个字节
*/
u8 ip_str_to_ByteArray(u8 * ip_str,u8 *ByteArray)
{
	u8  i = 0;
	char  Tmp_str[40];
	char  * Token;
	u8 Tmp_Len = strlen((const char *)ip_str);
	if(Tmp_Len > MAX_IP_STR_LEN)
		Tmp_Len = MAX_IP_STR_LEN;
	memset(Tmp_str,0x00,sizeof(Tmp_str));
	memcpy(Tmp_str,ip_str,Tmp_Len);
	
	for(i=0;i<4;i++)
	{
		if(i == 0)
		{
			Token = strtok(Tmp_str,".");
		}
		else
		{
			Token = strtok(NULL,".");
		}
		if(Token == NULL)
		{
			return FLASE;
		}
		ByteArray[i] = atoi(Token);
	}
	return TRUE;
}

/*
4字节IP转字符串	255 255 255 255 转字符串 255.255.255.255
*/
u8 ip_byteArray_to_ip_str(u8 * ByteArray,u8 * ip_str)
{
	u8  i = 0;
	u8  pos = 0;
	for(i=0;i<4;i++)
	{
		#ifdef	SET_WIFI
		if(ByteArray[i]>=100)
		{
			ip_str[pos++] = (ByteArray[i] / 100 % 10) + '0';
			ip_str[pos++] = (ByteArray[i] / 10 % 10) + '0';
			ip_str[pos++] = (ByteArray[i] / 1 % 10) + '0';
		}
		else if(ByteArray[i]>=10)
		{
			ip_str[pos++] = (ByteArray[i] / 10 % 10) + '0';
			ip_str[pos++] = (ByteArray[i] / 1 % 10) + '0';
		}
		else
		{
			ip_str[pos++] = (ByteArray[i] / 1 % 10) + '0';
		}
		#else
		ip_str[pos++] = (ByteArray[i] / 100 % 10) + '0';
		ip_str[pos++] = (ByteArray[i] / 10 % 10) + '0';
		ip_str[pos++] = (ByteArray[i] / 1 % 10) + '0';
		#endif
		if(i<=2)
		{
			ip_str[pos++] = '.';
		}
		
	}
	#ifdef	SET_WIFI
	ip_str[pos++] = '\0';
	#endif
	return TRUE;
}

/*
校验IP地址
*/
u8 p_check_air720_Udp_ipAndPort(u8 * IP_buf,u16 port)
{
	const u8  ErrIPBuf0[4] = {0xFF,0xFF,0xFF,0xFF};	//255.255.255.255	和 0.0.0.0认为是错误IP
	const u8  ErrIPBuf1[4] = {0x00,0x00,0x00,0x00};
	u8  T_Len = strlen((const char *)IP_buf);
	if(T_Len == 0)
		return FLASE;
	if(check_str_is_ip(IP_buf,T_Len) == TRUE)
	{//如果是IP的话，确认一下IP地址是否正确
		u8 Tmp_IP_ByteArray[4];
		ip_str_to_ByteArray(IP_buf,Tmp_IP_ByteArray);
		if((memcmp(Tmp_IP_ByteArray,ErrIPBuf0,4) == 0) || (memcmp(Tmp_IP_ByteArray,ErrIPBuf1,4) == 0))
		{
			return FLASE;
		}
	}
	if(port == 0)
		return FLASE;
	
	return TRUE;
}
#endif


//#include <stdio.h>


u8 isLte_Onff,isC_Lte;
extern st_run_info  g_run_info;
extern char *g_c_m26l_rbuf;

extern bit g_IP_PORT_Change;

/*
判断缓存里面是否有IPD,由于strstr容易被字符串停止符打断导致出错
*/
u8 is_have_IPD(char *line,u16 max_len)
{
	u16 i = 0;
	for(i=0;i<max_len;i++)
	{
		#ifdef	SET_WIFI
		if((line[i] == '+') &&(line[i+1] == 'I') && (line[i+2] == 'P') && (line[i+3] == 'D'))
		#else
			#if MultiLink_EN
				
				#if MODE_TYPE==AIR720
				if((line[i] == '+') && (line[i+1] == 'R') && (line[i+2] == 'E')&& (line[i+3] == 'C')&& (line[i+4] == 'E')&& (line[i+5] == 'I'))
				#elif MODE_TYPE==ML307R
				if((line[i] == 'M') && (line[i+1] == 'I') && (line[i+2] == 'P')&& (line[i+3] == 'U')&& (line[i+4] == 'R')&& (line[i+5] == 'C'))
				#elif MODE_TYPE==N716
				if((line[i] == 'D') && (line[i+1] == 'P') && (line[i+2] == 'R')&& (line[i+3] == 'E')&& (line[i+4] == 'C')&& (line[i+5] == 'V'))
				#elif MODE_TYPE==N701
				if((line[i] == 'K') && (line[i+1] == 'T') && (line[i+2] == 'R')&& (line[i+3] == 'E')&& (line[i+4] == 'C')&& (line[i+5] == 'V'))
				
				#else
				if((line[i] == '\"') && (line[i+1] == 'r') && (line[i+2] == 'e')&& (line[i+3] == 'c')&& (line[i+4] == 'v')&& (line[i+5] == '\"'))
				#endif
			#else
			if((line[i] == 'I') && (line[i+1] == 'P') && (line[i+2] == 'D'))
			#endif
		#endif
		{
			return TRUE;
		}
	}
	return FLASE;
}


u8 CheckIPD(char *line, u16 max_len,u8 * ReturnPos)
{
	u16 i = 0;
	u8 ReturnNum = 0;
	for(i=0;i<max_len;i++)
	{
		#ifdef	SET_WIFI
		if((line[i] == '+') && (line[i+1] == 'I') && (line[i+2] == 'P')&& (line[i+3] == 'D'))	
		#else
			//+RECEIVE:
			#if MultiLink_EN
				#if MODE_TYPE==AIR720
				if((line[i] == '+') && (line[i+1] == 'R') && (line[i+2] == 'E')&& (line[i+3] == 'C')&& (line[i+4] == 'E')&& (line[i+5] == 'I'))
				#elif MODE_TYPE==ML307R
				if((line[i] == 'M') && (line[i+1] == 'I') && (line[i+2] == 'P')&& (line[i+3] == 'U')&& (line[i+4] == 'R')&& (line[i+5] == 'C'))
				#elif MODE_TYPE==N716
				if((line[i] == 'U') && (line[i+1] == 'D') && (line[i+2] == 'P')&& (line[i+3] == 'R')&& (line[i+4] == 'E')&& (line[i+5] == 'C'))
				#elif MODE_TYPE==N701
				if((line[i] == 'K') && (line[i+1] == 'T') && (line[i+2] == 'R')&& (line[i+3] == 'E')&& (line[i+4] == 'C')&& (line[i+5] == 'V'))	
				#else
				if((line[i] == '\"') && (line[i+1] == 'r') && (line[i+2] == 'e')&& (line[i+3] == 'c')&& (line[i+4] == 'v')&& (line[i+5] == '\"'))
				#endif
			#else
			if((line[i] == 'I') && (line[i+1] == 'P') && (line[i+2] == 'D'))
			#endif
		#endif
		{
			ReturnPos[ReturnNum] = i;
			ReturnNum++;
			if(ReturnNum >= 5)	//防止溢出，会出现问题
			{
				return ReturnNum;
			}
		}
	}
	return ReturnNum;
}



/*四、SN12初始化、联网指令：*/
u8 p_air720_common_func(u8 send_string, char *at_cmd, char *match_cmd, u16 wait_ms, u8 (*proc_cb)(char *),u8 iWait_OK)
{
	#if TEST_WIFI
	return 0;
	#else
	char    *line = NULL;
	u8	    ret_val = FALSE;
	//u8      max_line = 0x00;
    
	#ifdef	SET_DIAL
	u16       data_len = 0x00;
	u32 	 start_time = my_time();
	#else
	int      data_len = 0x00;
	u32 	 start_time = my_time();
	#endif
		
	//清一下缓冲,防止上一个OK导致立刻退出
    //if (strstr(g_c_m26l_rbuf, "IPD") == NULL)
	if(is_have_IPD(g_c_m26l_rbuf,g_u16_m26l_rindex) == FLASE)
	{
		p_m26l_clear_recv_buffer();
	}

	/* 发送命令 */
	if (send_string == TRUE)
	{
		p_uart_send_string("AT+");
		p_uart_send_string(at_cmd);
	}
	p_uart_send_string("\r\n");

	/* 等待数据过来 */
//	data_len = p_m26l_wait_data(TRUE, wait_ms,TRUE);                // 获取模块返回数据       
	/* 换行符自动变成0x00 */
	
	while ((my_time() - start_time) < wait_ms)
	{
		//p_RunPro();	//弱实时任务
		Sleep_ms(10);
		if (strstr(g_c_m26l_rbuf, "ERROR") != NULL){            // ERROR处理
			break;
		}

		if ((match_cmd == NULL) || (iWait_OK == TRUE)){             // 2019.01.12：增加其他字符的判断需求
			if(strstr(g_c_m26l_rbuf, "OK\r\n") != NULL)            // OK 处理
			{
					break;                          
			} 
		}else if(strstr(g_c_m26l_rbuf, match_cmd) != NULL){        // 匹配字符串处理     
			if(iWait_OK == FALSE){
					break;    
			}             
		}
	}
	
	/*接收到OK，紧接着接收到IPD，但未超时，要等超时，否则会误清掉*/
	while((my_time() - start_time) < 1000)
	{
		Sleep_ms(10);
		//if(my_time()-g_uart_ctrl_2.m_last_rx_time>20)
		if(COM1.RX_TimeOut == 0)
		{
			/*PrintString1("\r\n[wait_time_ok]\r\n");*/
			break;
		}
	}
	
	//g_b_m26l_r_ok = TRUE;
	/*0x0d替换为0x00 ：拆分为行*/
	//p_m26l_format_data(g_u16_m26l_rindex);                         
    data_len = g_u16_m26l_rindex;      
        
	/* 按行处理数据 */
	//while (((line = p_m26l_get_line(line, data_len)) != NULL) && ((++max_line) < C_M26_MAX_PROC_LINE))
	line = g_c_m26l_rbuf;
	{
		if (strlen(line) > 0)
		{
//			#if SET_NB_LOG
//			PrintString1(line);
//			PrintString1("\r\n");
//			#endif
			
			if (strstr(line, "OK"))
			{
				if (match_cmd == NULL)
				{
					ret_val = TRUE;
				}
				else
				{
					if(strstr(line, match_cmd) != NULL)
					{
						ret_val = TRUE;
					}
				}
				#if(0)
				//当匹配字符串是包含OK的话，容易被误判 如SHUT OK会在这边误判，因此增加这些
				else if(strstr(match_cmd,"OK") != NULL)
				{
					if(strlen(match_cmd) > strlen("OK"))
					{
						ret_val = TRUE;
					}
				}
				#endif
			}
#ifdef SET_4G
//			if(strstr(line,"pdpdect") != NULL)
//			{
//				g_isWifiConnect = 1;
//			}
#endif
			
#ifdef	SET_WIFI
			if(strstr(line,"WIFI CONNECTED") != NULL)
			{
				g_isWifiConnect = 1;
			}
			
			// if(strstr(line,"WIFI GOT IP") != NULL)
			// {
				// g_isWifiConnect = 1;
				// isGsmCREG = 1;
			// }
			
#endif		

#ifdef	SET_DIAL
			if(strstr(line,"RING") != NULL)
			{
				g_isRing = 1;
			}
#endif	
			
				
#if(MultiLink_EN > 0)
			//if(strstr(line,"+RECEIVE") != NULL)
			if(is_have_IPD(line,data_len) == TRUE)
#else
			//if(strstr(line,"+IPD") != NULL)
			if(is_have_IPD(line,data_len) == TRUE)
#endif
			{
				//同时接收到多组数据
				u8  IPD_Pos[10];		
				u8  IPD_NUM = 0;
				u8 i = 0;
				memset(IPD_Pos,0x00,sizeof(IPD_Pos));
				IPD_NUM = CheckIPD(line,data_len,IPD_Pos);
				if(IPD_NUM)
				{
					LOG("GET URC:%d\n",IPD_NUM);
				}
//				
				for(i=0;i<IPD_NUM;i++)
				{
					char *ipd = line+IPD_Pos[i];

					//LOG("G:%s",ipd);
					p_air720_proc_read_data(ipd);
				}
			}
			
			//200325		由于收到+CSQ: 0,99 后面会接着OK 会受到判断OK的影响导致返回True，因为把回调提前 
			if (proc_cb != NULL)
			{
				/* 剩下的交由回调处理 */
				ret_val = proc_cb(line);
			}
			if ((match_cmd != NULL) && (strstr(line, match_cmd) != NULL))
			{
				ret_val = TRUE;
				/*
				if(proc_cb != NULL)
				{
					ret_val = proc_cb(line);
				}
				else
				{
					ret_val = TRUE;
				}*/
			}

			//else
			{
				/* 交由主动上传数据函数处理 */
				//防止重复处理？
				g_m26l_active_receive_pro(line);
			}
		}
	}	
	/* 清理接收缓冲区 */
	//p_m26l_clear_recv_buffer();
	p_m26l_clear_buffer(data_len);		//200409
	
	return ret_val;
	#endif
}

#ifdef	SET_4G
u8 Check_buf_is_str(char * buf,u8 len)
{//检查缓冲是不是0-9字符及AF，af之类
	u8 i = 0;
	for(i=0;i<len;i++)
	{
		//if(!(((buf[i] >= 'A') && (buf[i] <= 'Z')) && ((buf[i] >= 'a') && (buf[i] <= 'z')) && ((buf[i] >= '0') && (buf[i] <= '9'))))
		if(!(((buf[i] >= 'A') && (buf[i] <= 'Z')) || ((buf[i] >= 'a') && (buf[i] <= 'z')) || ((buf[i] >= '0') && (buf[i] <= '9'))))
		{
			return FLASE;
		}
	}
	return TRUE;
}
#endif

/*
检查UDP链路
*/
u8 p_air720_check_socket(void)
{
	//单链路直接判断CONNECT OK 		多链路交给回调处理
	#if(MultiLink_EN > 0)
	return p_nblc_common_func(TRUE, "CIPSTATUS",NULL, 1000, p_air720_check_socket_proc,FALSE);
	#else
		#ifdef	SET_WIFI
		return p_nblc_common_func(TRUE, "CIPSTATUS", "+CIPSTATUS:", 1000, NULL,FALSE);
		#else
		return p_nblc_common_func(TRUE, "CIPSTATUS", "CONNECT OK", 1000, NULL,FALSE);
		#endif
	#endif
}

/*
设置AIR720多连接方式	1:打开（即支持多条链路）	0关闭
*/
u8 p_air720_set_cipmux(u8 Type)
{
	if(Type == 1)
	{
		return p_nblc_common_func(TRUE, "CIPMUX=1", "OK", 1000, NULL,FALSE);
	}
	else
	{
		//return p_nblc_common_func(TRUE, "CIPMUX=0", "OK", 1000, NULL,FALSE);
		return p_nblc_common_func(TRUE, "CIPMUX=0", NULL, 1000, NULL,TRUE);
	}
	
}

#ifdef	SET_4G
u8 p_air720_proc_imei(char *line)
{	
	if(strstr(line,"OK") == NULL)		
		return FLASE;
	do
	{
		if('0' <= *line && *line <= '9')
		{
			memcpy(g_run_info.m_imei,line,sizeof(g_run_info.m_imei));
			g_run_info.m_imei[15] = '\0';
			if(Check_buf_is_str(g_run_info.m_imei,sizeof(g_run_info.m_imei)-1) == FLASE)
			{
/* 				PrintString1("m_imei:");
				PrintString1(g_run_info.m_imei); */
				memset(g_run_info.m_imei,0x00,sizeof(g_run_info.m_imei));
				return FLASE;
			}
			return TRUE;
		}
	}while(*line++ != 0x00);
	
	return FLASE;
}
u8 p_air720_proc_imsi(char *line)
{
	if(strstr(line,"OK") == NULL)		//200415增加判断，防止没插卡返回+CME ERROR：13 误把这个认为是IMEI
		return FLASE;
		
	do
	{
		if('0' <= *line && *line <= '9')
		{
			memcpy(g_run_info.m_imsi,line,sizeof(g_run_info.m_imsi));
			g_run_info.m_imsi[15] = '\0';
			if(Check_buf_is_str(g_run_info.m_imsi,sizeof(g_run_info.m_imsi)-1) == FLASE)
			{
/* 				PrintString1("m_imsi:");
				PrintString1(g_run_info.m_imsi); */
				memset(g_run_info.m_imsi,0x00,sizeof(g_run_info.m_imsi));
				return FLASE;
			}
			return TRUE;
		}
	}while(*line++ != 0x00);
	return FLASE;
}

u8 p_air720_proc_iccid(char *line)
{	
	if(strstr(line,"OK") == NULL)
		return FLASE;
		
	do
	{
		if('0' <= *line && *line <= '9')
		{
			memcpy(g_run_info.m_iccid,line,sizeof(g_run_info.m_iccid)-1);
			g_run_info.m_iccid[20] = 0;
			if(Check_buf_is_str(g_run_info.m_iccid,sizeof(g_run_info.m_iccid)-1) == FLASE)
			{
/* 				PrintString1("m_iccid:");
				PrintString1(g_run_info.m_iccid); */
				memset(g_run_info.m_iccid,0x00,sizeof(g_run_info.m_iccid));
				return FLASE;
			}
			return TRUE;
		}
	}while(*line++ != 0x00);
	return FLASE;
}


u8 p_air720_get_imei(void)
{
	#if MODE_TYPE==ML307R
	return p_nblc_common_func(TRUE, "CGSN=1",NULL,3000, p_air720_proc_imei,TRUE);
	#else
	//return p_nblc_common_func(TRUE, "CGSN","OK",3000, p_air720_proc_imei,FALSE);	
	return p_nblc_common_func(TRUE, "CGSN",NULL,3000, p_air720_proc_imei,TRUE);	
	#endif
}

u8 p_air720_get_imsi(void)
{
	//return p_nblc_common_func(TRUE, "CIMI","OK",3000, p_air720_proc_imsi,FALSE);
	return p_nblc_common_func(TRUE, "CIMI",NULL,3000, p_air720_proc_imsi,TRUE);
}

u8 p_air720_get_iccid(void)
{
	#if MODE_TYPE==ML307R
	return p_nblc_common_func(TRUE, "MCCID",NULL,3000, p_air720_proc_iccid,TRUE);
	#elif MODE_TYPE==N701
	return p_nblc_common_func(TRUE, "ICCID",NULL,1000, p_air720_proc_iccid,TRUE);
	#else
	//return p_nblc_common_func(TRUE, "CCID","OK",3000, p_air720_proc_iccid,FALSE);
	return p_nblc_common_func(TRUE, "CCID",NULL,3000, p_air720_proc_iccid,TRUE);
	#endif
}

#ifdef	SET_DIAL
u8 p_air720_set_not_upgrade(void)
{
	p_uart_send_string("ATI\r\n");
	return p_nblc_common_func(FLASE, NULL,"OK",3000, NULL,FALSE);
}
#endif


#if 0
u8 p_air720_get_ati(void)
{
	p_uart_send_string("ATI\r\n");
	return p_nblc_common_func(FLASE, NULL,"OK",3000, NULL,FALSE);
}
#endif

#if 0
void Uart_Change_BaudRate(u32 BaudRate)
{
	T2L = (65536 - MAIN_Fosc / BaudRate / 4);
	T2H = (65536 - MAIN_Fosc / BaudRate / 4) >> 8;
}
#endif

u8 p_air720_set_IPR(void)
{
	//return p_nblc_common_func(TRUE,"IPR=9600;&W","OK",5000,NULL,FLASE);
	//return p_nblc_common_func(TRUE,"IPR=115200;&W","OK",5000,NULL,FLASE);
	return p_nblc_common_func(TRUE,"IPR=0;&W","OK",5000,NULL,FLASE);
}


//AT+READADC=3
//+READADC: 3, 3837
//OK

u8 p_nblc_proc_vbatt(char *line)
{	
	char *s = NULL;
	if (strstr(line, "READADC:"))	
	{
		//第2个逗号后面
		s = strstr(line,",");
		if(s != NULL)
		{
			g_cbSta.bBatVal = atoi(&s[1]);
			AIR720_printf("BAT:%d\r\n",g_cbSta.bBatVal);
						
		
			service.m_battery = g_cbSta.bBatVal;
			
			if(fIsAlm()==0)
			{
				//电池低压事件自行判断,低压需要多次确认
				if(service.m_battery<=service.m_battery_set)
				{
						service.m_battery_state = 1;				
						if(g_cbSta.isBattLowSnd==0)
						{
							g_cbSta.isBattLowSnd = 1;
							Event_AddEvent(EVENT_BAT);
							LOG("低压报警\r\n");
						}	
				}
				//一次即可确认低压恢复，200mV的阈值
				//else if(service.m_battery>service.m_battery_set+200)
				//300mV阈值
				else if(service.m_battery>service.m_battery_set+300)
				{
						if(service.m_battery_state==1)
						{
							service.m_battery_state = 0;
							g_cbSta.isBattLowSnd = 0;
							//去除低压恢复事件，但低压状态可以变更
							#if 0	
								Event_AddEvent(EVENT_BAT_RST);
								LOG("低压恢复\r\n");
							#endif			
						}
				}	
			}
			return TRUE;
		}	
	}
	return FALSE;
}
void NB_ReadVbatt(void)
{
	#if MODE_TYPE==N716
	p_nblc_common_func(TRUE, "READADC=3", NULL, 1000, p_nblc_proc_vbatt,FALSE);
	#endif
}

#if SET_BLE_LEARN
uint8 p_air720_init(void)
{
	static u8 sim_status	= FALSE;
	static u8 ca_status	= FALSE;
	static u8 at_ok_times	= 0x00;
	static u32  timout;
	switch (g_run_info.m_nbiot_init_sta)
	{
		case STA_4G_INT_BOOT:
			sim_status = FALSE;
			ca_status	= FALSE;
			at_ok_times	= 0x00;
			p_sn12_gpio_init();	//IO口一样，直接调用NB的初始化函数
			/* 启动sn12*/
			p_sn12_boot();
		
			
		
			AIR720_printf("Start AT\r\n");
			g_run_info.m_nbiot_init_sta = STA_4G_INT_AT;
			if(g_device_info.test_mode)
			{
				timout = my_time() + 15000;
			}
			else
			{
				timout = my_time() + C_SN12_WAIT_AT_SEC;
			}
			
			break;
		case STA_4G_INT_AT:
			#if SET_TEST_MODE
			g_run_info.m_nbiot_init_sta = STA_4G_INT_AT;
			return 1;
			#endif
			if(my_time() < timout)
			{				
				if(g_bWaitTimeOut10ms==0)
				{
					if (p_nblc_communic_test() == TRUE)
					{
						if (++at_ok_times >= 2)
						{
							//p_air720_set_IPR();	//写死波特率

							/* 开启全功能 */
							//p_air720_set_npsmr();
							
							p_air720_set_ATE(0);
							//测试开回显
							//p_air720_set_ATE(1);
							is_GetAt = TRUE;
							is_get_card = FLASE;
							
							AIR720_printf("AT OK\r\n");

							
//							p_uart_send_string("AT+CMVERSION\r\n");	
//							Sleep_ms(1000);
//							p_uart_send_string("AT+CGMR\r\n");	
//							Sleep_ms(1000);
							//CMGR
							//AT+CGMR
							//p_air720_get_ver();
							

							//p_air720_get_batt();

							g_run_info.m_nbiot_init_sta = STA_4G_INT_CARD;
							timout = my_time() + C_SN12_WAIT_SIM_SEC;
						}
					}
					g_bWaitTimeOut10ms = 50;
				}
				
			}
			else
			{
				AIR720_printf("AT ERR\r\n");
				g_run_info.m_nbiot_init_sta = STA_4G_INT_0;
				
				if(g_device_info.test_mode)
				{
					g_cbSta.bNetTimeOut1s = 0;
				}
				
			}
			break;
		case STA_4G_INT_CARD:
			if(my_time() < timout)
			{
				if(g_bWaitTimeOut10ms==0)
				{
//					if((Para_info.method&0x01) !=0x01)
//					{
//						g_run_info.m_nbiot_init_sta = STA_4G_INT_0;
//						break;
//					}
					
					if(p_air720_get_cpin() == FALSE)
					{
						g_bWaitTimeOut10ms = 100;
						break;
					}
					
					#if 1
					if(p_air720_get_imei() == FLASE)
					{
						g_bWaitTimeOut10ms = 100;
						break;
					}			
					//不需要用到
					
					if(p_air720_get_imsi() == FLASE)
					{
						Sleep_ms(1000);
						break;
					}
					#endif
					if(p_air720_get_iccid() == FLASE)
					{
						g_bWaitTimeOut10ms = 100;
						break;
					}
					

					#if 0	//修改成与NB一样的流程，能获取到ICCID，就不用获取卡了
					if(p_air720_get_cpin() == FALSE)
					{
						Sleep_ms(1000);
						continue;
					}
					#endif
					sim_status = TRUE;
					
					is_get_card = TRUE;
					AIR720_printf("GET_CARD OK\r\n");

					p_air720_get_csq();

					g_run_info.m_nbiot_init_sta = STA_4G_INT_REG;
					timout = my_time() + C_SN12_WAIT_CS_PS_SEC;
				}
			}
			else
			{
				AIR720_printf("GET_CARD ERR\r\n");
				g_run_info.m_nbiot_init_sta = STA_4G_INT_0;

				//250303:没有卡不再重启，直接关机
				if(sim_status == FALSE)
				{
					g_cbSta.bNetTimeOut1s = 0;
				}
			}
			break;
		case STA_4G_INT_REG:
			if(my_time() < timout)
			{	
				if(g_bWaitTimeOut10ms==0)
				{
					if((p_air720_get_cgatt() == TRUE) && (p_air720_get_CGREG() == TRUE))				
					{
						ca_status = TRUE;
						is_Reg_Net = TRUE;

						AIR720_printf("GET_NET OK\r\n");

						g_run_info.m_nbiot_init_sta = STA_4G_INT_0;
						g_run_info.m_nbiot_init_ok = 1;
					}
					else
					{
						g_bWaitTimeOut10ms = 100;
					}	
				}
			}
			else
			{
				AIR720_printf("GET_NET ERR\r\n");
				g_run_info.m_nbiot_init_sta = STA_4G_INT_0;
			}
			break;
		default:
			g_run_info.m_nbiot_init_sta = STA_4G_INT_0;
			break;
	}


}
#else
u8 p_air720_init(void)
{
	u8 sim_status	= FALSE;
	u8 ca_status	= FALSE;
	u8 at_ok_times	= 0x00;
	//u8 csq_status	 = FALSE;
	u32  timout;
	//u8 i;
	p_sn12_gpio_init();	//IO口一样，直接调用NB的初始化函数
	/* 启动sn12*/
	p_sn12_boot();
    
	AIR720_printf("Start AT\r\n");

	/* 等待启动完成 */
	for (/*u32*/ timout = my_time() + C_SN12_WAIT_AT_SEC; my_time() < timout; )
	{
		if (p_nblc_communic_test() == TRUE)
		{
			if(g_cbSta.isBattRead==0)
			{
				g_cbSta.isBattRead = 1;
				
				NB_ReadVbatt();				
			}
			
			if (++at_ok_times >= 2)
			{
				timout = 0x00;
			}
		}
		Sleep_ms(500);
	}

	p_air720_set_IPR();	//写死波特率
	
	if (at_ok_times >= 2)
	{
		/* 开启全功能 */
		p_air720_set_npsmr();
		
		//测试开回显
		#if SET_TEST
		p_air720_set_ATE(1);
		#else
		p_air720_set_ATE(0);
		#endif
		is_GetAt = TRUE;
		is_get_card = FLASE;
		
		AIR720_printf("AT OK\r\n");
		
	
		/* 查卡20次*/
		//for(i=0;i<20;i++)
		for (/*u32*/ timout = my_time() + C_SN12_WAIT_SIM_SEC; my_time() < timout; )
		{
			#if SET_CAT1

			#if MODE_TYPE==ML307R
			#elif MODE_TYPE==N701
			#else
			//开USB日志
			//AT^TRACECTRL=0,1,2。
			//AT+CFUN=1,1
			p_uart_send_string("AT^TRACECTRL=0,1,2\r\n");
			Sleep_ms(300);
			//AT^TRACECTRL=1,1,5
			p_uart_send_string("AT^TRACECTRL=1,1,5\r\n");
			Sleep_ms(300);
//			p_uart_send_string("AT+CFUN=1,1\r\n");
//			Sleep_ms(300);
			p_nblc_communic_test();

			#endif
			
			// p_uart_send_string("ATI\r\n");
			// Sleep_ms(300);
			p_nblc_common_func(TRUE, "GMR", NULL, 300, NULL,FALSE);
			#else
			if((Para_info.method&0x01) !=0x01)
			{
				break;
			}
			#endif
			
			if(p_air720_get_cpin() == FALSE)
			{
				Sleep_ms(500);
				continue;
			}
			
			//不需要用到
			#if 0
			if(p_air720_get_imei() == FLASE)
			{
				Sleep_ms(1000);
				continue;
			}
			if(p_air720_get_imsi() == FLASE)
			{
				Sleep_ms(1000);
				continue;
			}
			if(p_air720_get_iccid() == FLASE)
			{
				Sleep_ms(1000);
				continue;
			}
			#endif

			#if 0	//修改成与NB一样的流程，能获取到ICCID，就不用获取卡了
			if(p_air720_get_cpin() == FALSE)
			{
				Sleep_ms(1000);
				continue;
			}
			#endif
			sim_status = TRUE;
			
			is_get_card = TRUE;
			AIR720_printf("GET_CARD OK\r\n");
			break;
		}	
	}
	else
	{
		AIR720_printf("AT ERR\r\n");
	}

	/* 查卡成功,轮询发送读 ca cs ps 附着指令 */
	if((sim_status == TRUE) /*&& (csq_status == TRUE)*/)
	{
		is_Reg_Net = FLASE;
		
		//APN
		#if MODE_TYPE==EC600
		p_nblc_common_func(TRUE, "QICSGP=1", NULL, 1000, NULL,TRUE);
		#endif

		/* 查询附着状态 */
		for (/*u32*/ timout = my_time() + C_SN12_WAIT_CS_PS_SEC; my_time() < timout; )
		{
			#if USE_HARDWARE_V20
			u8 sim2Cnt = 0;
			#endif
			
			#if SET_CAT1==0
			if((Para_info.method&0x01) !=0x01)
			{
				break;
			}
			#endif
			
			p_air720_get_csq();
			
			/* 网络附着状态*/
			if (ca_status != TRUE)
			{
				if((p_air720_get_cgatt() == TRUE) && (p_air720_get_CGREG() == TRUE))				
				{
					ca_status = TRUE;
					is_Reg_Net = TRUE;

					AIR720_printf("GET_NET OK\r\n");
				}
				else
				{
					Sleep_ms(1000);
				}
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		AIR720_printf("GET_CARD ERR\r\n");
	}

	if(is_Reg_Net==0)
		AIR720_printf("GET_NET ERR\r\n");
		
	return ca_status;
}
#endif
/*
关闭及开启回显
*/
u8 p_air720_set_ATE(u8 Type)
{
	if(Type == 0)
	{
		p_uart_send_string("ATE0\r\n");	
		
	}
	else
	{
		p_uart_send_string("ATE1\r\n");	
	}
	return p_nblc_common_func(FLASE, NULL, "OK", 5000, NULL,FALSE);

}


/*
开启全功能
*/
u8 p_air720_set_npsmr(void)
{	//200407 增加等待OK
	return p_nblc_common_func(TRUE, "CFUN=1", "OK", 5000, NULL,FALSE);
}


/*
读卡
*/
u8 p_air720_get_cpin(void)
{
	//双卡双待
	#if USE_HARDWARE_V20
	return p_nblc_common_func(TRUE, "QDSPIN?", ",READY", 1000, NULL,FALSE);
	#else
	/* +CPIN: READY	READY前面空格不可省略	*/	
	return p_nblc_common_func(TRUE, "CPIN?", "+CPIN: READY", 1000, NULL,FALSE);
	#endif
}

#if 0
/*
获取注网状态
*/
u8 p_air720_get_CREG(void)
{
	if(p_nblc_common_func(TRUE, "CREG?", "+CREG: 0,1", 1000, NULL,FALSE) == FLASE)
		return p_nblc_common_func(TRUE, "CREG?", "+CREG: 0,5", 1000, NULL,FALSE);
	else
		return TRUE;
}
#endif


u8 p_air720_get_CGREG(void)
{
	#if MODE_TYPE==ML307R
	if(p_nblc_common_func(TRUE, "CEREG?", "+CEREG: 0,1", 1000, NULL,FALSE) == FLASE)
		return p_nblc_common_func(TRUE, "CEREG?", "+CEREG: 0,5", 1000, NULL,FALSE);
	else
		return TRUE;	
	#elif	MODE_TYPE==N701
	return p_nblc_common_func(TRUE, "CGATT?", "+CGATT: 1", 1000, NULL,FALSE);
	#else
	if(p_nblc_common_func(TRUE, "CGREG?", "+CGREG: 0,1", 1000, NULL,FALSE) == FLASE)
		return p_nblc_common_func(TRUE, "CGREG?", "+CGREG: 0,5", 1000, NULL,FALSE);
	else
		return TRUE;	
	#endif
	
}



u8 p_air720_set_cgatt(void)
{
	return p_nblc_common_func(TRUE,"CGATT=1","OK",1000,NULL,FLASE);
}

/*
获取当前注网情况
AIR720模块：+CGATT: 1	有空格
AIR724模块: +CGATT:1	无空格
*/
u8 p_air720_get_cgatt(void)
{
	return p_nblc_common_func(TRUE, "CGATT?", "+CGATT: 1", 1000, NULL,FALSE);
}


/*
关闭移动场景，网络相关状态恢复为初始化状态（ipinitial）
*/
u8 p_air720_set_cipshut(void)
{
	#if MODE_TYPE==AIR720
	return p_nblc_common_func(TRUE, "CIPSHUT", "SHUT OK", 2000, NULL,FALSE);
	#elif MODE_TYPE==ML307R
	return p_nblc_common_func(TRUE, "MIPCALL=0,1", NULL, 20000, NULL,FALSE);
	#elif MODE_TYPE==N716
	return p_nblc_common_func(TRUE, "XIIC=0", NULL, 20000, NULL,FALSE);
	#else
	return p_nblc_common_func(TRUE, "QIDEACT=1", NULL, 20000, NULL,FALSE);
	#endif
}


/*
判断信号值是否正常
*/
u8 p_air720_proc_csq(char *line)
{
	char *s;
	#if 0
	PrintString1("ENTER CSQ CALLBACK");
	PrintString1("{");
	PrintString1(line);
	PrintString1("}\r\n");
	#endif
	
	
	#if NB_MODULE==NB_N701
	char *tmp;
	
	//if (strstr(line, "CSQ:"))
	if (strstr(line, "CESQ:"))
	{
		tmp = strstr(line, ",");
		
		
		tmp = strstr(&tmp[1], ",");	//去掉“，”
		tmp = strstr(&tmp[1], ",");
		tmp = strstr(&tmp[1], ",");
		tmp = strstr(&tmp[1], ",");
		
		//rsrp 质量
		g_run_info.m_csq = atoi(&tmp[1]);
		//g_run_info.m_csq = atoi(&line[5]);
		
		return TRUE;
	}
	return FALSE;
	#else
	s = strstr(line,"+CSQ:");
	if(s != NULL)
	{
		g_run_info.m_csq = atoi(&s[6]);
		//信号正常值是0-31	99表示未知或不可测
		if((g_run_info.m_csq >= 1) && (g_run_info.m_csq <= 31))
		{
			#if 0
			PrintString1("Air720_GetCSQ_OK");
			#endif
			return TRUE;
		}
	}
	return FLASE;
	#endif
}

/*
获取信号,返回成功或者失败	
*/
u8 p_air720_get_csq(void)
{
	#if NB_MODULE==NB_N701
	return p_nblc_common_func(TRUE, "CESQ", NULL, 1000, p_air720_proc_csq,FALSE);
	//return p_nblc_common_func(TRUE, "CSQ", "OK", 1000, p_air720_proc_csq,FALSE);
	#else
	return p_nblc_common_func(TRUE, "CSQ", "CSQ", 1000, p_air720_proc_csq,TRUE);
	#endif
}
#endif


u8 p_air720_proc_read_data(char *line)
{
	//u16  i;
	char *s = NULL;
	u16 recv_len = 0;
	char *b = g_c_m26l_rbuf;
	u8 SocketIndex;
	
	#if(MultiLink_EN > 0)
		#if MODE_TYPE==AIR720
		//+RECEIVE,1,5:
		//12345
		if(strstr(line,"+RECEIVE") == NULL)
		#elif MODE_TYPE==ML307R
		//+MIPURC: "rudp",0,10,1234567890
		if(strstr(line,"MIPURC") == NULL)
		#elif MODE_TYPE==N716
		//+UDPRECV: 0,10,1234567890
		if(strstr(line,"UDPRECV") == NULL)
		#elif MODE_TYPE==N701
		//+SKTRECV: 1,5,"3233343536"
		if(strstr(line,"KTRECV") == NULL)
		#else
		//+QIURC: "recv",0,16
		//?\0!鯸0\0``\0;EVx櫁v
		if(strstr(line,"\"recv\"") == NULL)
		#endif
	{
		return FLASE;
	}
	#else
	//+IPD,5:12345		
		#ifdef	SET_WIFI
		if(strstr(line,"+IPD") == NULL)
		#else
		//需要匹配+IPD
		if(strstr(line,"IPD") == NULL)
		#endif
	{
		return FLASE;
	}
	#endif 
		
	#if(MultiLink_EN > 0)
	#if MODE_TYPE==AIR720
	//+RECEIVE,1,5:
	//12345
	s = strstr(line,",");	//第一个逗号
	if(s != NULL)
	{
		SocketIndex = atoi((s+1));
		s++;
		s = strstr(s,",");	//第二个逗号
		recv_len = atoi((s+1));

		#if MODE_TYPE==AIR720
		s++;
		s = strstr(s,":");
		//b = s+1;
		b = s+3;
		#else
		s++;
		s = strstr(s,"\x0d");	//0x0d 0x0a
		b = s+2;
		#endif
	}
	#elif MODE_TYPE==ML307R
	//+MIPURC:"rudp",0,10,1234567890

	s = strstr(line,",");	//第一个逗号
	if(s != NULL)
	{
		SocketIndex = atoi((s+1));
		s++;
		s = strstr(s,",");	//第二个逗号
		recv_len = atoi((s+1));
		s++;
		s = strstr(s,",");	//第三个逗号
		b = s+1;
	}
	#elif MODE_TYPE==N716
	//+UDPRECV: 0,10,1234567890
	
	

	s = strstr(line,":");	//第一个冒号
	if(s != NULL)
	{
		SocketIndex = atoi((s+1));
		s++;
		s = strstr(s,",");	//第一个逗号
		recv_len = atoi((s+1));
		s++;
		s = strstr(s,",");	//第二个逗号
		b = s+1;
	}
	#elif MODE_TYPE==N701
	//+SKTRECV: 0,5,"3233343536"
	
	//LOG("RCV1\n");

	s = strstr(line,":");	//第一个冒号
	if(s != NULL)
	{
		//LOG("RCV2\n");
		SocketIndex = atoi((s+1));
		s++;
		s = strstr(s,",");	//第一个逗号
		recv_len = atoi((s+1));
		s++;
		s = strstr(s,",");	//第二个逗号
		//b = s+1;
		s++;
		s++;
		//bcd转换
		for (u8 i = 0; i < recv_len; i++)
		{
			b[i]  = hextoi(s[i * 2]) << 4;
			b[i] |= hextoi(s[i * 2 + 1]);
		}
		b[recv_len] = 0;
	}
	#endif

	#else
	//+IPD,5:12345	
	s = strstr(line,",");
	if(s != NULL)
	{
		recv_len = 0;
		SocketIndex = atoi((s+1));
		recv_len = atoi((s+3));
		s += 3;
		if(sock==1)
		{
			//第2个逗号
			s = strstr(s,",");
			if(s != NULL)
			{
				s++;
				memcpy(&g_bRouteIp[0],&s[0],sizeof(g_bRouteIp));
				for(i=0;i<sizeof(g_bRouteIp);i++)
				{
					if(g_bRouteIp[i]==',')
					{
						g_bRouteIp[i] = '\0';
					}
				}
			}
			else
			{
				return FLASE;
			}
			
		}	
		s = strstr(s,":");
		b = s+1;
	}
	else
	{
		return FLASE;
	}
	#endif
		
	#if SET_PROTO_VER==PROTO_VER_JSON	
	/* 服务器协议处理 */
		g_run_info.m_nb_stat.m_nbiot_recv_cnt++;
		if(p_prto_dispose_data(b, recv_len) == TRUE){
         //           p_nblc_communic_test();                       // 测试OK
                }

		/* 重设需要处理时间 */
		g_run_info.m_nbiot_start_proc_time = my_time();

                
		return TRUE;
	#elif SET_PROTO_VER==PROTO_VER_ZH202
								
	if(recv_len<sizeof(g_cbNet.rx_buf))
		memcpy(g_cbNet.rx_buf,b,recv_len);

	if(SocketIndex==0)
	{
		g_run_info.m_nbiot_err_cnt = 0;
		g_cbNet.UDPCh1Timer120s = 1;
	
		g_cbSta.bNetTimeOut1s = TIME_OUT_NET;
		g_cbSta.isNbNetOk = 1;	
		//只要有数据则去除重发和正在发送状态
		//g_run_info.m_flg_sending = FLASE;
		//g_run_info.m_flg_retry = FLASE;
		g_run_info.m_nbiot_create_sock_ok = TRUE;
		//g_run_info.m_last_nbiot_connect_time = my_time();
		//g_cbSta.bDevTimeOut1s = 3;	//等待平台下发数据
	}
	pNetCut(recv_len,SocketIndex);


	
	return TRUE;
	#endif
}






#if(MultiLink_EN > 0)
/*
激活移动场景
*/
u8 p_air720_set_ciicr(void)
{
	#if MODE_TYPE==AIR720
	return p_nblc_common_func(TRUE, "CIICR", "OK", 1000, NULL,FALSE);
	#elif MODE_TYPE==ML307R
	return p_nblc_common_func(TRUE, "MIPCALL=1,1", "OK", 60000, NULL,FALSE);
	#elif MODE_TYPE==N716
	return p_nblc_common_func(TRUE, "XIIC=1", "OK", 60000, NULL,FALSE);
	#elif MODE_TYPE==N701
	return p_nblc_common_func(TRUE, "CGATT=1", "OK", 60000, NULL,FALSE);
	#else
	return p_nblc_common_func(TRUE, "QIACT=1", "OK", 60000, NULL,FALSE);
	#endif
}
/*
设置APN
*/
u8 p_air720_set_cstt(void)
{
	return p_nblc_common_func(TRUE, "CSTT", "OK", 1000, NULL,FALSE);
}
#endif


u8 p_air720_check_local_ip(char * line)
{
	/*
	10.122.135.245	//查找到三个.表示IP正常
	*/
	char *s;
	s = line;
	if(strstr(s,".") != NULL)
	{
		s = strstr(s,".");
		s++;		
		if((s[0]>'0')&&(s[0]<='9'))
		{
			is_GetIp = 1;
			return TRUE;
		}
		else
		{
			if(strstr(s,".") != NULL)
			{
				s = strstr(s,".");
				s++;		
				if((s[0]>'0')&&(s[0]<='9'))
				{
					is_GetIp = 1;
					return TRUE;
				}
			}
			else
			{
				if(strstr(s,".") != NULL)
				{
					s = strstr(s,".");
					s++;		
					if((s[0]>'0')&&(s[0]<='9'))
					{
						is_GetIp = 1;
						return TRUE;
					}
				}
			}
		}
	}
	//需要获取MAC地址
	return FALSE;
}

/*
获取本地IP
*/
u8 p_air720_get_local_ip(void)
{
	#if MODE_TYPE==AIR720
	return p_nblc_common_func(TRUE, "CIFSR", NULL, 1000, p_air720_check_local_ip,FALSE);
	#elif MODE_TYPE==ML307R
	return p_nblc_common_func(TRUE, "MIPCALL?", NULL, 1000, p_air720_check_local_ip,FALSE);
	#elif MODE_TYPE==N716
	return p_nblc_common_func(TRUE, "XIIC?", NULL, 1000, p_air720_check_local_ip,FALSE);
	#elif MODE_TYPE==N701
	return p_nblc_common_func(TRUE, "CGPADDR=1", NULL, 5000, p_air720_check_local_ip,FALSE);
	#else
	return p_nblc_common_func(TRUE, "QIACT?", NULL, 1000, p_air720_check_local_ip,FALSE);
	#endif
}




#if(MultiLink_EN > 0)
/*
创建多通道的UDP链路
*/
u8 p_air720_start_udp_mux_connect(u8 channel,u8 * IP_buf,u16 port)
{
	u32 	start_time = my_time();
	u16 	wait_ms = 20000;		//20秒
	
	#if MODE_TYPE==N701
	if(channel==0)
		channel = 1;
	#endif
	
	#if 0
	if(p_check_air720_Udp_ipAndPort(IP_buf,port) == FALSE)
	{
		return FALSE;
	}
	else
	#endif
	{
		#if(1)
		char  Tmp_str[50];
		//sprintf(Tmp_str,"CIPSTART=%1d,UDP,%03d.%03d.%03d.%03d,%d",channel,IP_buf[0],IP_buf[1],IP_buf[2],IP_buf[3],port);	
		//return p_nblc_common_func(TRUE, Tmp_str, "CONNECT OK", 1000, NULL,FALSE);
		
		//也要判断已经存在？？
		#if MODE_TYPE==AIR720
		#if SET_DOMAIN
		if((EEPROM_Ptr[DHCPAddr]&BIT_DOMAIN) == BIT_DOMAIN)
		{
			#if SET_MQTT
			if(channel==0)
			{
				sprintf(Tmp_str,"AT+CIPSTART=%1d,TCP,%s,%d\r\n",channel,IP_buf,port);
			}
			else
			{
				sprintf(Tmp_str,"AT+CIPSTART=%1d,UDP,%s,%d\r\n",channel,IP_buf,port);
			}
			#else
			sprintf(Tmp_str,"AT+CIPSTART=%1d,UDP,%s,%d\r\n",channel,IP_buf,port);
			#endif
		}
		else
		{
			#if SET_MQTT
			if(channel==0)
			{
				sprintf(Tmp_str,"AT+CIPSTART=%1d,TCP,%03d.%03d.%03d.%03d,%d\r\n",channel,IP_buf,port);
			}
			else
			{
				sprintf(Tmp_str,"AT+CIPSTART=%1d,UDP,%03d.%03d.%03d.%03d,%d\r\n",channel,IP_buf,port);
			}
			#else
			sprintf(Tmp_str,"AT+CIPSTART=%1d,UDP,%03d.%03d.%03d.%03d,%d\r\n",channel,IP_buf[0],IP_buf[1],IP_buf[2],IP_buf[3],port);
			#endif

		}
		#else
		//sprintf(Tmp_str,"AT+CIPSTART=%1d,UDP,%03d.%03d.%03d.%03d,%d\r\n",channel,IP_buf[0],IP_buf[1],IP_buf[2],IP_buf[3],port);
		sprintf(Tmp_str,"AT+CIPSTART=%1d,UDP,%s,%d\r\n",channel,IP_buf,port);	
		#endif
		#elif MODE_TYPE==ML307R
		/*
			AT+UDPCLOSE=1
			+UDPCLOSE: 1,OK

			AT+UDPSETUP=1,220.199.66.56,7000
			OK
			+UDPSETUP: 1,OK
		*/
		sprintf(Tmp_str,"MIPCLOSE=%1d\r\n",channel);
		p_nblc_common_func(TRUE, Tmp_str,NULL, 1000, NULL,FALSE);

		//sprintf(Tmp_str,"AT+MIPOPEN=%1d,\"UDP\",\"%03d.%03d.%03d.%03d\",%d,60,0\r\n",channel,IP_buf[0],IP_buf[1],IP_buf[2],IP_buf[3],port);
		sprintf(Tmp_str,"AT+MIPOPEN=%1d,\"UDP\",\"%s\",%d,60,0\r\n",channel,IP_buf,port);
		#elif MODE_TYPE==N716
		/*
			AT+UDPCLOSE=1
			+UDPCLOSE: 1,OK

			AT+UDPSETUP=1,220.199.66.56,7000
			OK
			+UDPSETUP: 1,OK
		*/
		sprintf(Tmp_str,"UDPCLOSE=%1d\r\n",channel);
		p_nblc_common_func(TRUE, Tmp_str,NULL, 1000, NULL,FALSE);

		sprintf(Tmp_str,"AT+UDPSETUP=%1d,%03d.%03d.%03d.%03d,%d\r\n",channel,IP_buf[0],IP_buf[1],IP_buf[2],IP_buf[3],port);
	
		#elif MODE_TYPE==N701
		sprintf(Tmp_str,"SKTDELETE=%1d\r\n",channel);
		p_nblc_common_func(TRUE, Tmp_str,NULL, 1000, NULL,FALSE);
		
		//修改成TCP链接
		//p_nblc_common_func(TRUE, "SKTCREATE=1,2,17","+SKTCREATE", 1000, NULL,FALSE);
		p_nblc_common_func(TRUE, "SKTCREATE=1,1,6","+SKTCREATE", 1000, NULL,FALSE);

		sprintf(Tmp_str,"AT+SKTCONNECT=%1d,%03d.%03d.%03d.%03d,%d\r\n",channel,IP_buf[0],IP_buf[1],IP_buf[2],IP_buf[3],port);
		#else
		//p_nblc_common_func(TRUE, "QICLOSE=1",NULL, 1000, NULL,FALSE);
		sprintf(Tmp_str,"QICLOSE=%1d\r\n",channel);
		p_nblc_common_func(TRUE, Tmp_str,NULL, 1000, NULL,FALSE);
		
		#if SET_DOMAIN
		if((EEPROM_Ptr[DHCPAddr]&BIT_DOMAIN) == BIT_DOMAIN)
		{
			#if SET_MQTT
			if(channel==0)
			{
				sprintf(Tmp_str,"AT+QIOPEN=1,%1d,\"TCP\",\"%s\",%d,0,1\r\n",channel,IP_buf,port);
			}
			else
			{
				sprintf(Tmp_str,"AT+QIOPEN=1,%1d,\"UDP\",\"%s\",%d,0,1\r\n",channel,IP_buf,port);
			}
			#else
			sprintf(Tmp_str,"AT+QIOPEN=1,%1d,\"UDP\",\"%s\",%d,0,1\r\n",channel,IP_buf,port);
			#endif
		}
		else
		{
			#if SET_MQTT
			if(channel==0)
			{
				sprintf(Tmp_str,"AT+QIOPEN=1,%1d,\"TCP\",\"%03d.%03d.%03d.%03d\",%d,0,1\r\n",channel,IP_buf[0],IP_buf[1],IP_buf[2],IP_buf[3],port);
			}
			else
			{
				sprintf(Tmp_str,"AT+QIOPEN=1,%1d,\"UDP\",\"%03d.%03d.%03d.%03d\",%d,0,1\r\n",channel,IP_buf[0],IP_buf[1],IP_buf[2],IP_buf[3],port);
			}		
			#else
			sprintf(Tmp_str,"AT+QIOPEN=1,%1d,\"UDP\",\"%03d.%03d.%03d.%03d\",%d,0,1\r\n",channel,IP_buf[0],IP_buf[1],IP_buf[2],IP_buf[3],port);
			#endif

		}
		#else
			#if SET_MQTT
			sprintf(Tmp_str,"AT+QIOPEN=1,%1d,\"TCP\",\"%03d.%03d.%03d.%03d\",%d,0,1\r\n",channel,IP_buf[0],IP_buf[1],IP_buf[2],IP_buf[3],port);
			#else
			sprintf(Tmp_str,"AT+QIOPEN=1,%1d,\"UDP\",\"%03d.%03d.%03d.%03d\",%d,0,1\r\n",channel,IP_buf[0],IP_buf[1],IP_buf[2],IP_buf[3],port);
			#endif
		#endif

		#endif
		
		#if MODE_TYPE==N701
		service.m_battery_set = 2750;		//电池低压阈值
		
		//p_uart_send_string("AT+SKTCONNECT=1,121.41.87.203,6000\r\n");
		//p_uart_send_string("AT+SKTCONNECT=1,58.60.184.214,6024\r\n");
		if(p_nblc_common_func(TRUE, "SKTCONNECT=1,121.41.87.203,6000",NULL, 5000, NULL,TRUE))
		{
			return 1;
		}
		return 0;
		#else
		p_uart_send_string(Tmp_str);	
		
		
		while ((my_time() - start_time) < wait_ms)
		{
			#if MODE_TYPE==AIR720
			if(strstr(g_c_m26l_rbuf, "CONNECT OK") != NULL)
			{
				return 1;
			}
			if(strstr(g_c_m26l_rbuf, "ALREADY") != NULL)
			{
				return 1;
			}
			#elif MODE_TYPE==ML307R
			sprintf(Tmp_str,"+MIPOPEN: %1d,0\r\n",channel);
			//if(strstr(g_c_m26l_rbuf, "+QIOPEN: 0,0") != NULL)
			if(strstr(g_c_m26l_rbuf, Tmp_str) != NULL)
			//if(strstr(g_c_m26l_rbuf, ",0") != NULL)
			{
				return 1;
			}
			#elif MODE_TYPE==N716
			if(strstr(g_c_m26l_rbuf, ",OK") != NULL)
			{
				return 1;
			}
			#elif MODE_TYPE==N701
//			if(strstr(g_c_m26l_rbuf, "+CSCON:") != NULL)
//			{
//				return 1;
//			}
			
			if(strstr(g_c_m26l_rbuf, ",OK") != NULL)
			{
				return 1;
			}
			#else
			sprintf(Tmp_str,"+QIOPEN: %1d,0\r\n",channel);
			//if(strstr(g_c_m26l_rbuf, "+QIOPEN: 0,0") != NULL)
			if(strstr(g_c_m26l_rbuf, Tmp_str) != NULL)
			//if(strstr(g_c_m26l_rbuf, ",0") != NULL)
			{
				return 1;
			}
			#endif
			Sleep_ms(10);	
		}
		return 0;
		#endif
		
		#else
		u8 i;
		if(channel > 6)
		{//通道支持0-5
			return FLASE;
		}
		p_uart_send_string("AT+CIPSTART=UDP,");
		p_swut_write2buff('0' + (channel % 10));
		for(i=0;i<4;i++)
		{
			p_swut_write2buff('0' + ((IP_buf[0] / 100) % 10));
			p_swut_write2buff('0' + ((IP_buf[0] / 10) % 10));
			p_swut_write2buff('0' + ((IP_buf[0] / 1) % 10));
		}
		p_uart_send_string(",");
		if(((port / 10000) % 10)  > 0)
		{
			p_swut_write2buff('0' + ((port / 10000) % 10));
			p_swut_write2buff('0' + ((port / 1000) % 10));
			p_swut_write2buff('0' + ((port / 100) % 10));
			p_swut_write2buff('0' + ((port / 10) % 10));
			p_swut_write2buff('0' + ((port / 1) % 10));
		}
		else if(((port / 1000) % 10)  > 0)
		{
			p_swut_write2buff('0' + ((port / 1000) % 10));
			p_swut_write2buff('0' + ((port / 100) % 10));
			p_swut_write2buff('0' + ((port / 10) % 10));
			p_swut_write2buff('0' + ((port / 1) % 10));
		}
		else if(((port / 100) % 10)  > 0)
		{
			p_swut_write2buff('0' + ((port / 100) % 10));
			p_swut_write2buff('0' + ((port / 10) % 10));
			p_swut_write2buff('0' + ((port / 1) % 10));
		}
		else if(((port / 10) % 10)  > 0)
		{
			p_swut_write2buff('0' + ((port / 10) % 10));
			p_swut_write2buff('0' + ((port / 1) % 10));
		}
		else
		{
			p_swut_write2buff('0' + ((port / 1) % 10));
		}
		
		//return p_nblc_common_func(FALSE, NULL, "CONNECT OK", 1000, NULL,FALSE);	
		return p_nblc_common_func(FALSE, NULL, "CONNECT OK", 20000, NULL,FALSE);		
			
		#endif
	}
}


/*
判断多链路的状态
*/
u8 p_air720_check_socket_proc(char *line)
{
	/*
	STATE: IP PROCESSING

	C: 0,,"","","","INITIAL"
	C: 1,0,"UDP","121.207.93.172","7301","CONNECTED"
	C: 2,,"","","","INITIAL"
	C: 3,,"","","","INITIAL"
	C: 4,,"","","","INITIAL"
	C: 5,,"","","","INITIAL"
	*/
	char *s;
	s = strstr(line,"STATE");
	if(s != NULL)
	{
		u8 Status[6] = {1,1,1,1,1,1};	//6条链路的状态
		u8 i;
		for(i=0;i<6;i++)
		{
			if(i == 0)
			{
				s = strtok(line,"C:");
			}
			else
			{
				s = strtok(NULL,"C:");
			}
			if((s == NULL) || (strstr(s,"CONNECTED") == NULL))
			{
				Status[i] = 0;
			}
		}
		if(	(Status[0] == 0) ||
			(Status[1] == 0) ||
			(Status[2] == 0) ||
			(Status[3] == 0) ||
			(Status[4] == 0) ||
			(Status[5] == 0)
			)
		{
			return FLASE;
		}
		else
		{
			return TRUE;
		}	
	}
	return FALSE;
}


u8 p_air720_port_send_data(u8 channel,u8 *udp_data, u16 len)
{
	//u8  Tmp_Str[30];
	u32 	start_time = my_time();
	u16 wait_ms = 300;
	u16 i;
	
	#if MODE_TYPE==N701
	
	if(channel==0)
		channel = 1;
	#endif
	
	if(len == 0)
	{
		//LOG("No_Len");
		return	0;
	}
	
	#if MODE_TYPE==AIR720
	p_uart_send_string("AT+CIPSEND=");
	#elif MODE_TYPE==ML307R
	p_uart_send_string("AT+MIPSEND=");
	#elif MODE_TYPE==N716
	/*
		AT+UDPSEND=0,1024,,0
		>
		OK
		+UDPSEND: 0,1024
	*/
	p_uart_send_string("AT+UDPSEND=");
	#elif MODE_TYPE==N701
	p_uart_send_string("AT+SKTSEND=");
	#else
	p_uart_send_string("AT+QISEND=");
	#endif
	p_swut_write2buff('0' + channel);
	p_swut_write2buff(',');
	if(len > MAX_UDP_LEN)
		len = MAX_UDP_LEN;
	if((len / 1000) % 10)
	{
		p_swut_write2buff('0' + ((len / 1000) % 10));
		p_swut_write2buff('0' + ((len / 100) % 10));
		p_swut_write2buff('0' + ((len / 10) % 10));
		p_swut_write2buff('0' + ((len / 1) % 10));
	}
	else if((len / 100) % 10)
	{
		p_swut_write2buff('0' + ((len / 100) % 10));
		p_swut_write2buff('0' + ((len / 10) % 10));
		p_swut_write2buff('0' + ((len / 1) % 10));
	}
	else if((len / 10) % 10)
	{
		p_swut_write2buff('0' + ((len / 10) % 10));
		p_swut_write2buff('0' + ((len / 1) % 10));
	}
	else
	{
		p_swut_write2buff('0' + ((len / 1) % 10));
	}

	#if 0
	//sprintf(Tmp_Str,"AT+CIPSEND=%01d,%d\r\n",channel,len);
	//p_uart_send_string(Tmp_Str);
	while ((my_time() - start_time) < wait_ms)
	{
		Sleep_ms(1);
		if(strstr(g_c_m26l_rbuf,"ERROR") == NULL)
		{
			break;
		}
		if(strstr(g_c_m26l_rbuf,">") != NULL)
		{//收到 > 号表示可以发送数据了
			for (i = 0; i < len; i++)
			{
				p_swut_write2buff(udp_data[i]);
			}
			return p_nblc_common_func(FALSE, NULL, "SEND OK", 1000, NULL,FALSE);	//200410 5000->1000
		}
	}
	return 0;
	#endif

	
	#if MODE_TYPE==N701
	p_swut_write2buff(',');
	#else
	//要不要提高速度进行判断？
	p_uart_send_string("\r\n");	
	
	while ((my_time() - start_time) < wait_ms)
	{
		if(strstr(g_c_m26l_rbuf,">") != NULL)
		{
			break;
		}
		else
		{
			Sleep_ms(10);
		}	
	}
	#endif
	
	//Sleep_ms(300);
	for (i = 0; i < len; i++)		//200416 不进行判断
	{
		#if MODE_TYPE==N701
		p_swut_write2buff(itohex((udp_data[i] >> 4) & 0x0F));
		p_swut_write2buff(itohex((udp_data[i] >> 0) & 0x0F));
		#else
		p_swut_write2buff(udp_data[i]);	
		#endif
	}
	
	#if MODE_TYPE==N701
	return p_nblc_common_func(FALSE, NULL, NULL, 5000, NULL,TRUE);
	#else
	p_uart_send_string("\r\n");

	//打印应答数据
	//user_printf_hex(&udp_data[0], len);

	#if MODE_TYPE==N716
	return p_nblc_common_func(FALSE, NULL, "+UDPSEND:", 1000, NULL,FALSE);
	#elif	MODE_TYPE==ML307R
	return p_nblc_common_func(FALSE, NULL, "+MIPSEND:", 1000, NULL,FALSE);
	#elif	MODE_TYPE==N701
	return p_nblc_common_func(FALSE, NULL, "+CSCON:", 1000, NULL,FALSE);
	#else
	return p_nblc_common_func(FALSE, NULL, "SEND OK", 1000, NULL,FALSE);
	#endif
	
	#endif
}

u8 p_air720_read_data(void)
{
	#if MODE_TYPE==AIR720
	//return p_nblc_common_func(FALSE, NULL, "+RECEIVE", 2, p_air720_proc_read_data,TRUE);
	return p_nblc_common_func(FALSE, NULL, "+RECEIVE", 2, NULL,TRUE);
	#elif	MODE_TYPE==ML307R
	return p_nblc_common_func(FALSE, NULL, "MIPURC", 2, NULL,TRUE);
	#elif	MODE_TYPE==N716
	return p_nblc_common_func(FALSE, NULL, "UDPRECV", 2, NULL,TRUE);
	#elif	MODE_TYPE==N701
	return p_nblc_common_func(FALSE, NULL, "KTRECV", 2, NULL,TRUE);
	#else
	return p_nblc_common_func(FALSE, NULL, "\"recv\"", 2, NULL,TRUE);
	#endif
}
#else

#ifdef	SET_4G
/*
设置单链路状态下接收数据包含IP头	收到数据 +IPD,dat_len:dat
*/
u8 p_air720_set_ciphead(void)
{
	return p_nblc_common_func(TRUE, "CIPHEAD=1", "OK", 1000, NULL,FALSE);
}
#endif


/*
创建单通道的UDP链路
*/
u8 p_air720_start_udp_connect(u8 * IP_buf,u16 port)
{
	if((p_check_air720_Udp_ipAndPort(IP_buf,port) == FALSE) || (g_isNeedSave == 1))
	{//200325 IP必须设置完成之后才初始化
		#ifdef	SET_WIFI
		//PrintString1("IP ERR\n");
		#endif
		return FALSE;
	}
	else
	{
		g_IP_PORT_Change = FLASE;

		#ifdef	SET_WIFI
			#ifdef	SET_SK_ROUTE
			p_nblc_common_func(TRUE, "CIPCLOSE=0", NULL, 1000, NULL,TRUE);
			#else
			p_nblc_common_func(TRUE, "CIPCLOSE", NULL, 1000, NULL,TRUE);
			#endif
		#endif
			
		#ifdef	SET_WIFI
			#ifdef	SET_SK_ROUTE
			p_uart_send_string("AT+CIPSTART=0,\"UDP\",\"");
			#else
			p_uart_send_string("AT+CIPSTART=\"UDP\",\"");
			#endif
		p_uart_send_string(IP_buf);
		p_uart_send_string("\",");
		#else
		p_uart_send_string("AT+CIPSTART=UDP,");
		p_uart_send_string(IP_buf);
		p_uart_send_string(",");
		#endif
			
		#if 1	//创建链路，端口7301，可以直接发送07301，不必进行判断
		p_swut_write2buff('0' + ((port / 10000) % 10));
		p_swut_write2buff('0' + ((port / 1000) % 10));
		p_swut_write2buff('0' + ((port / 100) % 10));
		p_swut_write2buff('0' + ((port / 10) % 10));
		p_swut_write2buff('0' + ((port / 1) % 10));
		#else
		if(((port / 10000) % 10)  > 0)
		{
			p_swut_write2buff('0' + ((port / 10000) % 10));
			p_swut_write2buff('0' + ((port / 1000) % 10));
			p_swut_write2buff('0' + ((port / 100) % 10));
			p_swut_write2buff('0' + ((port / 10) % 10));
			p_swut_write2buff('0' + ((port / 1) % 10));
		}
		else if(((port / 1000) % 10)  > 0)
		{
			p_swut_write2buff('0' + ((port / 1000) % 10));
			p_swut_write2buff('0' + ((port / 100) % 10));
			p_swut_write2buff('0' + ((port / 10) % 10));
			p_swut_write2buff('0' + ((port / 1) % 10));
		}
		else if(((port / 100) % 10)  > 0)
		{
			p_swut_write2buff('0' + ((port / 100) % 10));
			p_swut_write2buff('0' + ((port / 10) % 10));
			p_swut_write2buff('0' + ((port / 1) % 10));
		}
		else if(((port / 10) % 10)  > 0)
		{
			p_swut_write2buff('0' + ((port / 10) % 10));
			p_swut_write2buff('0' + ((port / 1) % 10));
		}
		else
		{
			p_swut_write2buff('0' + ((port / 1) % 10));
		}
		#endif
			
		#ifdef	SET_4G
		return p_nblc_common_func(FALSE, NULL, "CONNECT OK", 20000, NULL,FALSE);
		#endif
			
		#ifdef	SET_WIFI
		return p_nblc_common_func(FALSE, NULL, "CONNECT", 20000, NULL,TRUE);
		#endif
	}
}


/*
单链路状态下发送数据
*/
#ifdef	SET_SK_ROUTE
u8 p_air720_port_send_data(u8 channel,u8 *udp_data, u16 len)
#else
u8 p_air720_port_send_data(u8 *udp_data, u16 len)
#endif
{
	//u8 Tmp_Str[30];
	u16 i = 0;
	//u8 Ready_To_Send = FLASE;
	
	#ifndef	SET_SK_ROUTE
		#ifndef	SET_DIAL
		u32 wait_ms = 1000;
		u32 start_time = my_time();
		#endif
	#endif
		
	p_uart_send_string("AT+CIPSEND=");
	#ifdef	SET_SK_ROUTE
	p_swut_write2buff(channel+'0');
	p_swut_write2buff(',');	
	#endif
	if(len > COM_TX3_Lenth)
		len = COM_TX3_Lenth;
	
	p_swut_write2buff('0' + ((len / 1000) % 10));
	p_swut_write2buff('0' + ((len / 100) % 10));
	p_swut_write2buff('0' + ((len / 10) % 10));
	p_swut_write2buff('0' + ((len / 1) % 10));
	
	if(channel==1)
	{
		p_uart_send_string(",\"");
		p_uart_send_string(&g_bRouteIp[0]);
		p_uart_send_string("\",33333");
	}
	p_uart_send_string("\r\n");
	
	//sprintf(Tmp_Str,"AT+CIPSEND=%d\r\n",len);
	//p_uart_send_string(Tmp_Str);
	
	Sleep_ms(300);
	
	for (i = 0; i < len; i++)		//200416 不进行判断
	{
		p_swut_write2buff(udp_data[i]);
	}
	p_uart_send_string("\r\n");
	
	return p_nblc_common_func(FALSE, NULL, "SEND OK", 1000, NULL,FALSE);	
}

u8 p_air720_read_data(void)
{
	#if 1
	return p_nblc_common_func(FALSE, NULL, "+IPD", 2, NULL,TRUE);		//200330 p_nblc_common_func函数中有解析函数，无需添加回调，否则会执行两次
	#else
	return p_nblc_common_func(FALSE, NULL, "+IPD", 2, p_air720_proc_read_data,TRUE);
	#endif
}
#endif


u8 p_air720_UDP_connect_net(u8 socket)
{

	#if SET_CAT1==0
	p_air720_set_ATE(0);
	#endif
	
	//#if MODE_TYPE==N701
	//is_GetIp = 1;
	//p_air720_set_cgatt();
	//is_GetIp = 0;
	//p_air720_get_local_ip();
	//#else
	//先查询IP是否正常
	is_GetIp = 0;
	p_air720_get_local_ip();
	//#endif

	#if(MultiLink_EN > 0)
	//多端口不能轻易关闭
	#if SET_PROTO_VER==PROTO_VER_JSON
	if(is_GetIp==0)
	#elif SET_PROTO_VER==PROTO_VER_ZH202
	if((is_GetIp==0)||(g_cbNet.IpChangeFlag[0])||(g_cbNet.IpChangeFlag[1]))
	#endif
	{
		#if MODE_TYPE==AIR720
		if(p_air720_set_cipshut() == FLASE)
		{
			return FLASE;
		}
		Sleep_ms(2000);	//强制等待2S确保关闭
		#elif MODE_TYPE==N701
		#else
		p_air720_set_cipshut();
		#endif	
		
		#if MODE_TYPE==AIR720
		//开多链路模式
		if(p_air720_set_cipmux(1) == FALSE)
		{
			return FALSE;
		}
		//设置APN
		if(p_air720_set_cstt() == FALSE)
		{
			return FALSE;
		}
		#endif


		//激活移动场景
		if(p_air720_set_ciicr() == FLASE)
		{
			return FALSE;
		}
		
		//获取本地IP，防止创建链路失败
		if(p_air720_get_local_ip() == FALSE)
		{
			return FALSE;
		}

		
		//关闭以后所有的端口都会异常
		g_run_info.m_nbiot_create_sock_ok = FLASE;
		#if SET_PROTO_VER==PROTO_VER_ZH202
		g_cbNet.IpChangeFlag[0] = 0;
		g_cbNet.IpChangeFlag[1] = 0;
		#endif
	}

	if(p_air720_start_udp_mux_connect(socket,service.Ip_buf,service.port) == FLASE)
	{
		return FALSE;
	}
	return TRUE;
	#else
	if((is_GetIp==0)||(g_cbNet.IpChangeFlag[2])||(g_cbNet.IpChangeFlag[3]))
	{
		//单链路模式
		if(p_air720_set_cipmux(1) == FALSE)
		{
			//return FLASE;
		}
		/*
		//创建链路
		if(p_air720_start_udp_connect(g_cbSta.bIp,g_cbSta.wSocket) == FLASE)
		{
			return FLASE;
		}
		*/
		
		#ifdef	SET_4G
		//设置数据接收头
		if(p_air720_set_ciphead() == FALSE)
		{
			return FLASE;
		}
		#endif
	}

	return p_air720_start_udp_connect(g_cbSta.bIp_Str,g_cbSta.wSocket);	
	#endif
	
}

bit  is_get_card = FLASE;
bit  is_Reg_Net  = FLASE;
bit  is_GetAt = FLASE;
bit  is_GetIp = FLASE;

//char *g_c_m26l_rbuf = (char *)g_uart_ctrl_2.m_rx_buf;
char *g_c_m26l_rbuf = (char *)RX1_Buffer;
st_run_info  g_run_info;

u32  g_nbiot_last_get_csq_time = 0x00;

u8  init_Tmr;
u8  sock_Tmr[2];

u8 itohex(u8 aHex)
{
	if (/* (aHex >= 0) && */(aHex <= 9))
		aHex += 0x30;
	else if ((aHex >= 10) && (aHex <= 15))//A-F
		aHex += 0x37;
	else aHex = 0xff;
	return aHex;
}


/**
* ***********************************************************************
* @brief	从UART发送一条字符串（阻塞）
*
*	@param  char *c: 字符串
*
* @retval void:
*
* @attention
* ***********************************************************************
*/
void p_uart_send_string(const char *c)
{
	/* 发送到字符串结束 */
	while (*c)
	{
		p_swut_write2buff(*c++);
	}

	return;
}

void p_m26l_clear_buffer(u16 bLen)
{
	memset(g_c_m26l_rbuf, 0x00, bLen);
	if(g_u16_m26l_rindex>bLen)
	{
		memcpy(&g_c_m26l_rbuf[0],&g_c_m26l_rbuf[bLen],g_u16_m26l_rindex-bLen);
		g_u16_m26l_rindex = g_u16_m26l_rindex-bLen;
	}
	else
	{
		g_u16_m26l_rindex = 0;
	}	
}

void p_m26l_clear_recv_buffer(void)
{
	g_u16_m26l_rindex = 0x00;
	memset(g_c_m26l_rbuf, 0x00, UART_RX_LEN);
	//g_b_m26l_r_ok = FALSE;
}


void p_sn12_gpio_init(void)
{

}

void p_sn12_boot(void)
{
	PrintString1("下电上电\r\n");
	#if MODE_TYPE==AIR720
	// isC_Lte = 	LTE_POW_OFF;
	// isLte_Onff = NB_ON;
	// Sleep_ms(500);
	// Sleep_ms(500);
	// isC_Lte = LTE_POW_ON;
	// isLte_Onff = NB_ON;
	// Sleep_ms(1000);
	// Sleep_ms(1000);
	// isC_Lte = LTE_POW_ON;
	// isLte_Onff = NB_OFF;
	// Sleep_ms(1500);
	// isLte_Onff = NB_ON;
	// Sleep_ms(1000);
	SET_LTE_RST_N(0);
	SET_LTE_PWR_C(1);
	Sleep_ms(500);
	SET_LTE_PWR_C(0);
	SET_LTE_RST_N(0);
	Sleep_ms(1000);
	Sleep_ms(1000);
	SET_LTE_PWR_C(0);
	SET_LTE_RST_N(1);
	Sleep_ms(500);
	#elif MODE_TYPE==ML307R
	
	#if SET_TEST
	#else
	SET_LTE_RST_N(0);
	SET_LTE_PWR_C(1);
	Sleep_ms(200);
	#endif
	SET_LTE_RST_N(1);
	SET_LTE_PWR_C(0);
	Sleep_ms(500);
	
	
	
	#elif MODE_TYPE==N716
	SET_LTE_PWR_C(0);
	Sleep_ms(1000);
	SET_LTE_PWR_C(1);
	Sleep_ms(500);
	SET_LTE_PWR_C(1);
	Sleep_ms(500);
	//复位模块
	SET_LTE_RST_N(0);
	Sleep_ms(80);
	SET_LTE_RST_N(1);
	Sleep_ms(500);

	#elif  MODE_TYPE==N701
	SET_LTE_PWR_C(1);
	SET_LTE_RST_N(0);
	Sleep_ms(500);
	SET_LTE_RST_N(1);
	SET_LTE_PWR_C(0);	//上电等电源稳定
	//Sleep_ms(5000);								//>10s
	Sleep_ms(1000);
	#else
	static u8 s_isPowOn = 0;
	if(s_isPowOn)
	{
		isC_Lte = LTE_POW_OFF;
		pHC595_Pro();		
		Sleep_ms(4000);	//4s
	}
	else
	{
		s_isPowOn = 1;
	}
	isC_Lte = LTE_POW_ON;
	pHC595_Pro();
	Sleep_ms(2000);
	#endif
}

u8 p_nblc_communic_test(void)
{
	p_uart_send_string("AT");
	return p_nblc_common_func(FALSE, NULL,	NULL,	1000, NULL,FALSE);
}


u8 IsNeedConnect(u8 socket)
{
	if(isVailIpInt(service.Ip_buf))
	{	
		//230316:测试，只一个端口
		if(socket==0)
		{
			return 1;
		}
		else
		{
			return 0;
		}
		#if 0
		return 1;
		#endif
	}
	else 
	{
		return 0;
	}
	
}


void NB_pro(void)
{
	//状态机
	#if SET_BLE_LEARN
	if(TRUE==g_run_info.m_nbiot_init_ok)
	{
		g_run_info.m_nbiot_init_sta = STA_4G_INT_0;
	}

	if(g_run_info.m_nbiot_init_sta==STA_4G_INT_0)
	{
		/* 初始化NBIOT */
		if (/*(g_run_info.m_nbiot_canbe_init == TRUE) &&*/
			(g_run_info.m_nbiot_init_ok == FALSE) 

				|| (g_run_info.m_nbiot_err_cnt >= 10)
				)
		{
			is_get_card = FLASE; 
			is_Reg_Net = FLASE;
			is_GetAt = FLASE;
		
			g_run_info.m_nbiot_init_ok = FALSE;
			g_run_info.m_nbiot_err_cnt = 0x00;
			g_run_info.m_nbiot_create_sock_ok = FALSE;
			
			AIR720_printf("init_Net");

			g_run_info.m_nbiot_init_sta = STA_4G_INT_BOOT;
	
			//g_run_info.m_nbiot_init_ok = p_sn12_init();
		}
	}
	else
	{
		p_sn12_init();
		
		if(g_device_info.test_mode)
		{
			//230617:开机过程进入标定则直接退出
			if(device_info.state.F_SM_BD)
			{
				g_cbSta.bNetTimeOut1s = 0;
				g_cbSta.isBd = 1;
				DEBUG_LOG("标定状态模块关机\n");
				
				//230704:防拆状态直接重置,不需去抖
				if(bFcClose>0)
				{
					bFcClose = 0xffff;
					g_cbSta.isFcOpen = FC_CLOSE;
				}	
				else if(bFcOpen>0)
				{
					bFcOpen = 0xffff;
					g_cbSta.isFcOpen = FC_OPEN;
				}
				
				g_run_info.m_nbiot_init_ok = FALSE;
				#if SET_BLE_LEARN
				g_run_info.m_nbiot_init_sta = STA_4G_INT_0;
				#endif
			}
		}
		
	}
	#else
	/* 初始化NBIOT */
	if (((g_run_info.m_nbiot_init_ok == FALSE) 
			|| (g_run_info.m_nbiot_err_cnt >= 10)		//230315:控电端口异常可重启模块
			)
			#ifdef	SET_WIFI
			|| (g_isSetRoute)
			#endif
			
//			#ifdef SET_4G
//			&& (init_Tmr == 0)
//			#endif
			)
	{
		is_get_card = FLASE; 
		is_Reg_Net = FLASE;
		is_GetAt = FLASE;
		
		g_run_info.m_nbiot_init_ok = FALSE;
		g_run_info.m_nbiot_err_cnt = 0x00;
		g_run_info.m_nbiot_create_sock_ok = FALSE;
		
//		#ifdef SET_4G
//		init_Tmr = 60;		//4G增加最短的重启时间(120S)，防止频繁操作重启，模块容易升级失败
//		#endif
			
		
	
		AIR720_printf("init_Net");
		g_run_info.m_nbiot_init_ok = p_sn12_init();

	}
	#endif
	
	
	if ((g_run_info.m_nbiot_init_ok == TRUE) &&
	((g_run_info.m_nbiot_create_sock_ok == FALSE) 
	) && IsNeedConnect(0) && (sock_Tmr[0] == 0))
	{
			sock_Tmr[0] = 10;

			if ((g_run_info.m_nbiot_create_sock_ok = p_nblc_UDP_connect_net(0)) == FALSE)
			{
				LOG("SOCK0 OPEN Fail\r\n");
				g_run_info.m_nbiot_err_cnt += 5;        			// 等待时间长，重发2次即可  
				
			}else{
				LOG("SOCK0 OPEN OK\r\n");
				#if SET_PROTO_VER==PROTO_VER_ZH202
				g_cbNet.UDPCh1Timer120s = 1;			   //端口打开120s没有接收到数据则异常	
				//230316:是否可以不心跳，直接发送数据？
				//立即心跳
				g_cbNet.hreadbeat_timer1 = NET_HEARTBEAT_TIME;	
				#endif
				
			}
	}		
		

	/* NBIOT正常处理流程 */
	if ((g_run_info.m_nbiot_init_ok == TRUE) &&
		(g_run_info.m_nbiot_create_sock_ok == TRUE) )
	{
		//读取数据
		//if((my_time()-g_uart_ctrl_2.m_last_rx_time>20)&&(g_u16_m26l_rindex>12))
		if((COM1.RX_TimeOut==0)&&(g_u16_m26l_rindex>12))
		{
			p_nblc_read_data();
		}
				
		//正在发送不能发数据
		if(FLASE==g_run_info.m_flg_sending)
		{
			if (g_run_info.m_last_nbiot_resend_count > 3)
			{
				//PrintString1("\r\n\r\nReSnd_Over\r\n\r\n");
				g_run_info.m_last_nbiot_resend_count = 0;
				g_run_info.m_flg_retry = 0;	//重发次数则不再重发
				
				g_cbSta.isNbNetOk = 0;							
			}
			
			NB_TxPro();
		}
		#if SET_PROTO_VER==PROTO_VER_ZH202
		//120s无数据重连端口
		if(g_cbNet.UDPCh1Timer120s >= 120)
		{
			//g_cbNet.IpChangeFlag[0] = 0;
			g_cbNet.server1Status = 0;//服务1 链路不正常
			g_cbNet.UDPCh1Timer120s = 0;
			g_cbNet.reconn_count++;
			if(g_cbNet.reconn_count>3)
			{
				#if SET_BLE_LEARN
				g_run_info.m_nbiot_init_sta = STA_4G_INT_0;
				#endif
				g_run_info.m_nbiot_init_ok = FALSE;
			}
			else
			{
				g_run_info.m_nbiot_create_sock_ok = FLASE;
			}		      
		}
		#endif
		
		
		
		//5s重发
		if (TRUE == g_run_info.m_flg_sending && ((my_time() - g_run_info.m_last_nbiot_send_time) > (5*1000)))
		{
			g_run_info.m_flg_retry = TRUE;
			g_run_info.m_flg_sending = FLASE;
			
			g_run_info.m_last_nbiot_resend_count++;
			
			g_run_info.m_nbiot_err_cnt++;	
			PrintString1("Not_Reply");
		}		
	}
}
#endif



