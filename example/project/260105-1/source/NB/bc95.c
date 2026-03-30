#include	"bc95.h"

#if SET_NB

#define 	C_SN12_WAIT_AT_SEC			20
#define 	C_SN12_WAIT_SIM_SEC     	10   	//查卡30秒
#define 	C_SN12_WAIT_CS_PS_SEC   	60   	//联网120秒  -- 修改成60s入网
#define 	C_SN12_REBOOT_PIN_SEC		3
#define 	C_M26_MAX_PROC_LINE			20

#define 	g_m26l_active_receive_pro(r)


st_run_info  g_run_info;

char *g_c_m26l_rbuf = (char *)RX1_Buffer;
u8 p_nblc_proc_read_data(char *line);
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

void p_m26l_format_data(u16 max_len)
{
	u16 i;
	for (i = 0; i < max_len; i++)
	{
		if ((g_c_m26l_rbuf[i] == 0x0d) ||
			(g_c_m26l_rbuf[i] == 0x0a))
		{
			g_c_m26l_rbuf[i] = 0x00;
		}
	}

	return;
}

void p_m26l_clear_recv_buffer(void)
{
	g_u16_m26l_rindex = 0x00;
	#if 	SET_VER!=0
	memset(g_c_m26l_rbuf, 0x00, COM_RX2_Lenth);
	#else
	memset(g_c_m26l_rbuf, 0x00, COM_RX1_Lenth);
	#endif
	g_b_m26l_r_ok = FALSE;
}


void p_m26l_clear_buffer(u16 bLen)
{
	memset(g_c_m26l_rbuf, 0x00, bLen);
	if(g_u16_m26l_rindex>bLen)
	{
		memcpy(&g_c_m26l_rbuf[0],&g_c_m26l_rbuf[bLen],g_u16_m26l_rindex-bLen);
		g_u16_m26l_rindex = g_u16_m26l_rindex-bLen;
		LOG("remind:%d\r\n",g_u16_m26l_rindex);
	}
	else
	{
		g_u16_m26l_rindex = 0;
	}	
}

// 读取一行参数
char *p_m26l_get_line(char *line, u32 max_len)
{
	u32 i;
	if (line == NULL)
	{
		return g_c_m26l_rbuf;
	}
	else if ((line < g_c_m26l_rbuf) || (line >= &g_c_m26l_rbuf[max_len]))
	{
		return NULL;
	}

	for (i = (line - g_c_m26l_rbuf); i < max_len; i++)
	{
		if (g_c_m26l_rbuf[i] != 0x00)
		{
			continue;
		}

		/* g_m261_rbuffer[i] == 0x00 */
		if ((g_c_m26l_rbuf[i + 1] != 0x00) && ((i + 1) < max_len))
		{
			return &g_c_m26l_rbuf[i + 1];
		}
	}

	/*返回NULL，退出getline流程*/
	return NULL;
}

void p_sn12_gpio_init(void)
{	

}

void p_sn12_boot(void)
{
	PrintString1("下电上电\r\n");
	#if	NB_MODULE==NB_MN316	
	//POW_EN 1, NB_ON 1 下电
	SET_ME3616_POW_EN(OFF_POW_NB);
	SET_ME3616_NB_ON(ON_RST_NB);
	Sleep_ms(1000);								//下电时间
	SET_ME3616_NB_ON(OFF_RST_NB);
	SET_ME3616_POW_EN(ON_POW_NB);				//上电等电源稳定
	Sleep_ms(1000);
	#elif	(NB_MODULE==NB_N306) 
	SET_ME3616_POW_EN(OFF_POW_NB);
	SET_ME3616_NB_ON(OFF_RST_NB);
	//230608:下电0.5s足够
	//Sleep_ms(1000);
	Sleep_ms(500);
	SET_ME3616_NB_ON(ON_RST_NB);
	SET_ME3616_POW_EN(ON_POW_NB);	//上电等电源稳定
	Sleep_ms(1000);

	#elif  (NB_MODULE==NB_N701)
	SET_ME3616_POW_EN(OFF_POW_NB);
	SET_ME3616_NB_ON(OFF_RST_NB);
	Sleep_ms(500);
	SET_ME3616_NB_ON(ON_RST_NB);
	SET_ME3616_POW_EN(ON_POW_NB);	//上电等电源稳定
	//Sleep_ms(5000);								//>10s
	Sleep_ms(1000);
	#endif

}

/*四、SN12初始化、联网指令：*/
u8 p_nblc_common_func(u8 send_string, char *at_cmd, char *match_cmd, u16 wait_ms, u8 (*proc_cb)(char *),u8 iWait_OK)
{
	int     data_len = 0x00;
	char    *line = NULL;
	u8	    ret_val = FALSE;
	u8      max_line = 0x00;
    u32 	start_time = my_time();
	
	#if (NB_COAP_EN == 1)  
		#if  NB_MODULE==NB_MN316
		//增加判断连接状态超时：3分钟
		if (strstr(g_c_m26l_rbuf, "+MCWEVENT:3") != NULL)
		{
			g_run_info.m_last_nbiot_connect_time = my_time();
		}
	
		//if (strstr(g_c_m26l_rbuf, "+MCWREAD:") == NULL)
		if (strstr(g_c_m26l_rbuf, "READ:") == NULL)
		#elif   NB_MODULE==NB_BC20
		if (strstr(g_c_m26l_rbuf, "QLWDATARECV") == NULL)
		#elif  (NB_MODULE==NB_N306) || (NB_MODULE==NB_N701)
		if ((strstr(g_c_m26l_rbuf, "CTM2MRECV") == NULL)&&(strstr(g_c_m26l_rbuf, "NNMI") == NULL))		
		#else
		if (strstr(g_c_m26l_rbuf, "M2MCLIRECV") == NULL)
		#endif
	{
		
		if(g_u16_m26l_rindex>0)
		{
			p_m26l_clear_recv_buffer();
		}
	}
	#endif
		
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
		if (strstr(g_c_m26l_rbuf, "ERROR") != NULL){            // ERROR处理
			break;
		}
            if ((match_cmd == NULL) || (iWait_OK == TRUE)){             // 2019.01.12：增加其他字符的判断需求
                if (strstr(g_c_m26l_rbuf, "OK\r\n") != NULL)            // OK 处理
                {
                        break;                          
                } 
            }else if (strstr(g_c_m26l_rbuf, match_cmd) != NULL){        // 匹配字符串处理     
                if(iWait_OK == FALSE){
                        break;    
                }             
            }
		Sleep_ms(1);
	}
	
	#if 1
		if(g_u16_m26l_rindex>0)
		{
			//230627:非\r\n结束就多等待一会
			if ((g_c_m26l_rbuf[g_u16_m26l_rindex-1] == 0x0d) ||
				(g_c_m26l_rbuf[g_u16_m26l_rindex-1] == 0x0a))
			{
			
			}
			else
			{
				while((my_time() - start_time) < 1000)
				{
					Sleep_ms(1);
					if((COM1.RX_TimeOut == 0)|| (g_c_m26l_rbuf[g_u16_m26l_rindex-1] == 0x0d) || (g_c_m26l_rbuf[g_u16_m26l_rindex-1] == 0x0a))
					{
						break;
					}
				}
			}
		}
		
	#endif
	
	g_b_m26l_r_ok = TRUE;
	/*0x0d替换为0x00 ：拆分为行*/
	p_m26l_format_data(g_u16_m26l_rindex);                         
  data_len = g_u16_m26l_rindex;       
        
	/* 按行处理数据 */
	while (((line = p_m26l_get_line(line, data_len)) != NULL) && ((++max_line) < C_M26_MAX_PROC_LINE))
	{
		if (strlen(line) > 0)
		{
			//打印行
			#if SET_NB_LOG
			//PrintString1("line:");
//			PrintString1(line);
//			PrintString1("\r\n");
			#endif
			
			#if	NB_MODULE==NB_BC20
			if (strstr(line, "OK")&&(match_cmd == NULL))
			{	
				ret_val = TRUE;
			}
			#else
			//空口回传标识
			if (strstr(line, "+MCWSTR:") != NULL)	
			{
				if (strstr(line, ",0") != NULL)
				{
					g_run_info.m_flg_str = 1;
				}
			}
			
			
			//消息ID	
			if (strstr(line, "+MCWSEND:") != NULL)
			{
				u8 *tmp;
				tmp = strstr(line, ":");
				g_run_info.mMessageId = atoi(&tmp[1]);
			}
			
			if (strstr(line, "OK"))
			{
				if (match_cmd == NULL)
				{
					ret_val = TRUE;
				}
			}
			#endif
			/* 垃圾模块数据会自动上报 其他命令也要检查数据 */
            #if (NB_COAP_EN == 1)  
				#if	NB_MODULE==NB_MN316
				//if (strstr(line, "+MCWREAD:") != NULL)
				if (strstr(line, "READ:") != NULL)
				#elif NB_MODULE==NB_BC20
				else if (strstr(line, "QLWDATARECV") != NULL)
				#elif  (NB_MODULE==NB_N306) || (NB_MODULE==NB_N701)
				else if ((strstr(line, "CTM2MRECV") != NULL)||(strstr(line, "NNMI") != NULL))
				#else
				else if (strstr(line, "M2MCLIRECV") != NULL)    // 内含接收数据处理；
				#endif
            #else
			else if (strstr(line,"ESONMI") != NULL)
            #endif
			{
				u8 get_data = p_nblc_proc_read_data(line);
				#if (NB_COAP_EN == 1)  
					#if	NB_MODULE==NB_MN316
					//ret_val = strcmp(match_cmd, "+MCWREAD:") == 0 ? get_data : ret_val;
					ret_val = strcmp(match_cmd, "READ:") == 0 ? get_data : ret_val;
					#elif NB_MODULE==NB_BC20
					ret_val = strcmp(match_cmd, "QLWDATARECV") == 0 ? get_data : ret_val;
					#elif  (NB_MODULE==NB_N306) || (NB_MODULE==NB_N701)
					ret_val = ((strcmp(match_cmd, "NNMI") == 0) || (strcmp(match_cmd, "CTM2MRECV") == 0))? get_data : ret_val;						
					#else
					ret_val = strcmp(match_cmd, "M2MCLIRECV") == 0 ? get_data : ret_val;
					#endif
				#else        
				ret_val = strcmp(match_cmd, "ESONMI") == 0 ? get_data : ret_val;
                #endif  
			}

			else if ((match_cmd != NULL) && (strstr(line, match_cmd) != NULL))
			{
				ret_val = TRUE;
			}
			else if (proc_cb != NULL)
			{
				//回调处理到一次OK就可以了
				#if NB_MODULE==NB_MN316
				if(ret_val==TRUE)
				{
					proc_cb(line);
				}
				else
				{
					ret_val = proc_cb(line);
				}
				#else
				/* 剩下的交由回调处理 */
				ret_val = proc_cb(line);
				#endif
			}
			else
			{
				/* 交由主动上传数据函数处理 */
				g_m26l_active_receive_pro(line);
			}
		}
	}

	/* 清理接收缓冲区 */
	//p_m26l_clear_recv_buffer();
	//230628:只清除解析到的接收缓冲
	p_m26l_clear_buffer(data_len);	
	return ret_val;
}

u8 p_nblc_communic_test(void)
{
	p_uart_send_string("AT");
	return p_nblc_common_func(FALSE, NULL,		NULL,			1000, NULL,FALSE);
}


u8 p_nblc_proc_cimi(char *line)
{
	do
	{
		if ('0' <= *line && *line <= '9')
		{
			memcpy(g_run_info.m_imsi, line, sizeof(g_run_info.m_imsi));
			g_run_info.m_imsi[15] = '\0';
			return TRUE;
		}
	}
	while(*line++ != 0x00);

	return FALSE;
}

/* 查SIM卡 */
u8 p_nblc_get_cimi(void)
{
	return p_nblc_common_func(TRUE, "CIMI",NULL,1000, p_nblc_proc_cimi,FALSE);
}

//
//u8 test_imei[15] = "861541066360398";
u8 p_nblc_proc_imei(char *line)
{
	do
	{
		if ('0' <= *line && *line <= '9')
		{
			memcpy(g_run_info.m_imei, line, sizeof(g_run_info.m_imei));
			
			//测试
			//memcpy(g_run_info.m_imei, test_imei, sizeof(g_run_info.m_imei));
			
			g_run_info.m_imei[15] = '\0';
			return TRUE;
		}
	}
	while(*line++ != 0x00);

	return FALSE;
}

u8 p_nblc_get_imei(void)
{
	#if (NB_MODULE==NB_MN316) ||(NB_MODULE==NB_N306) 
	return p_nblc_common_func(TRUE, "CGSN=1",	NULL,1000, p_nblc_proc_imei,FALSE);
	#elif NB_MODULE==NB_N701
	return p_nblc_common_func(TRUE, "CGSN",	NULL,1000, p_nblc_proc_imei,FALSE);
	#else
	return p_nblc_common_func(TRUE, "GSN",	NULL,1000, p_nblc_proc_imei,FALSE);
	#endif
}

u8 p_nblc_proc_iccid(char *line)
{
	do
	{
		if ('0' <= *line && *line <= '9')
		{
			memcpy(g_run_info.m_iccid, line, sizeof(g_run_info.m_iccid));
			g_run_info.m_iccid[20] = '\0';
			return TRUE;
		}
	}
	while(*line++ != 0x00);

	return FALSE;
}

u8 p_nblc_get_iccid(void)
{
	#if	NB_MODULE==NB_MN316
	p_uart_send_string("AT+NCCID");
	#elif  NB_MODULE==NB_N306
	if(g_run_info.m_flg_n306)
	{
		p_uart_send_string("AT+ECICCID");
	}
	else
	{
		p_uart_send_string("AT+QCCID");
	}
	#elif NB_MODULE==NB_N701
	p_uart_send_string("AT+ICCID");
	#else
	p_uart_send_string("AT*MICCID");
	#endif
	return p_nblc_common_func(FALSE, NULL,	NULL,1000, p_nblc_proc_iccid,FALSE);
}

#if 0
u8 p_nblc_set_cereg(void)
{
	return p_nblc_common_func(TRUE, "CEREG=4",NULL,1000, NULL,FALSE);
}
#endif


#if	(NB_COAP_EN ==0)
u8 p_nblc_set_cgatt(void)
{
	return p_nblc_common_func(TRUE, "CGATT=1",NULL,1000, NULL,FALSE);
}
#endif

u8 p_nblc_get_cereg(void)
{
	#if   NB_MODULE==NB_MN316
	if(p_nblc_common_func(TRUE, "CGATT?","+CGATT:1",1000, NULL,TRUE))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
	#elif (NB_MODULE==NB_BC260) ||(NB_MODULE==NB_N306) ||(NB_MODULE==NB_N701)
	if(p_nblc_common_func(TRUE, "CGATT?","+CGATT: 1",1000, NULL,TRUE))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}	
	#else
	/* 本地 / 漫游 */
	//if (TRUE == p_nblc_common_func(TRUE, "CEREG=0", NULL, 1000, NULL,FALSE))
	if (TRUE == p_nblc_common_func(TRUE, "CEREG=4", NULL, 1000, NULL,TRUE))
	{
		// if (p_nblc_common_func(TRUE, "CEREG?", "+CEREG: 0,1", 1000, NULL,FALSE) == FALSE)
			// return p_nblc_common_func(TRUE, "CEREG?", "+CEREG: 0,5", 1000, NULL,FALSE);
		if (p_nblc_common_func(TRUE, "CEREG?", "+CEREG: 4,1", 1000, NULL,FALSE) == FALSE)
			return p_nblc_common_func(TRUE, "CEREG?", "+CEREG: 4,5", 1000, NULL,FALSE);
		else
			return TRUE;
	}
	return FALSE;
	#endif
}

u8 p_nblc_proc_csq(char *line)
{
//	#if NB_MODULE==NB_N701
//	u8 temp;
//	char *s;
//	s = strstr(line,"+CSQ:");
//	if(s != NULL)
//	{
//		//转换成百分比
//		temp = atoi(&s[6]);
//		//信号正常值是0-31	99表示未知或不可测
//		if((temp >= 1) && (temp <= 31))
//		{
//			g_run_info.m_csq = (u16)temp*100/31;
//			return TRUE;
//		}
//	}
//	g_run_info.m_csq = 0xff;
//	return FLASE;
//	#else
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
//	#endif
}

u8 p_nblc_get_csq(void)
{
//	#if NB_MODULE==NB_N701
//	return p_nblc_common_func(TRUE, "CSQ", NULL, 1000, p_nblc_proc_csq,FALSE);
//	#else
	return p_nblc_common_func(TRUE, "CESQ", NULL, 1000, p_nblc_proc_csq,FALSE);
//	#endif
}


u8 p_nblc_set_npsmr(void)
{
	p_nblc_common_func(TRUE, "CFUN=1", NULL, 1000, NULL,FALSE);
	return TRUE;
}


#if (NB_COAP_EN ==0)
u8 p_nblc_proc_socket(char *line)
{
	do
	{
		if ('0' <= *line && *line <= '9')
		{
			g_run_info.m_socket_id = *line;
			return TRUE;
		}
	}
	while (*line++ != 0x00);

	return FALSE;
}
#endif

void pDelConnect(void)
{
	#if NB_MODULE==NB_MN316
	p_nblc_common_func(TRUE, "MCWCLOSE", NULL, 2000, NULL,FALSE);
	p_nblc_common_func(FALSE, NULL, "+MCWEVENT:5", 5000, NULL,FALSE);
	p_nblc_common_func(TRUE, "MCWDELETE", NULL, 2000, NULL,FALSE);
	p_nblc_common_func(FALSE, NULL, "+MCWEVENT:5", 5000, NULL,FALSE);
	// if(TRUE==p_nblc_common_func(FALSE, NULL, "+MCWEVENT:5", 20000, NULL,FALSE))
	// {
		// p_nblc_common_func(TRUE, "MCWDELETE", NULL, 2000, NULL,FALSE);
	// }
	//delay_ms(1000);
	#elif NB_MODULE==NB_BC260
	p_nblc_common_func(TRUE, "NCDPCLOSE", NULL, 2000, NULL,FALSE);	
	#elif (NB_MODULE==NB_N306) || (NB_MODULE==NB_N701)
	if(g_run_info.m_flg_n306)
	{
		p_nblc_common_func(TRUE, "CTM2MDEREG", NULL, 2000, NULL,FALSE);
	}
	else
	{
		p_nblc_common_func(TRUE, "NCDPCLOSE", NULL, 2000, NULL,FALSE);	
	}	
	#endif
	PrintString1("关闭端口");
	
	g_run_info.m_nbiot_create_sock_ok = FALSE;

	g_run_info.m_nbiot_err_cnt += 5;
}


u8 NB_getNMstatus(void)
{
	u32 	 start_time;
	 
	start_time = my_time();
	p_uart_send_string("AT+NMSTATUS?\r\n");
	while ((my_time() - start_time) < 1000)
	{
		if(strstr(g_c_m26l_rbuf, "REGISTERED"))
		{
			return TRUE;
		}
		else if(strstr(g_c_m26l_rbuf, "REGISTERING"))
		{
			if(p_nblc_common_func(FALSE, NULL, "+QLWEVTIND: 3", 10000, NULL,FALSE)==FALSE)
			{
				return FALSE;
			}
			return TRUE;
		}
	}
	
	return FALSE;
	
}

u8 p_nblc_UDP_connect_net(void)
{	
	u8 i;	
#if (NB_COAP_EN == 1)   
//发送：AT+M2MCLINEW=180.101.147.115,5683,”000000000000000”,43200  
//返回：OK
//      +M2MCLI:register success
//      +M2MCLI:observe success 
		#if   NB_MODULE==NB_MN316
		u8 isFail;
		isFail = 0;

		//AEP平台
		p_nblc_common_func(TRUE,"MCWCREATE=221.229.214.202,5683",NULL,1000,NULL,TRUE);
		//p_nblc_common_func(TRUE,"MCWCREATE=117.60.157.137,5683",NULL,1000,NULL,TRUE);
			

		//配置数据格式：hex字符串格式
		p_nblc_common_func(TRUE,"MCWCFGEX=0,0",NULL,1000,NULL,TRUE);
			
		//注册设备到平台 CTWing,保活时间86400秒
		p_uart_send_string("AT+MCWOPEN=1,86400");
		//p_uart_send_string("AT+MCWOPEN=1,300");
		//+MCWEVENT:1 注册成功
		//+MCWEVENT:5 订阅成功
		if(p_nblc_common_func(FALSE, NULL, "+MCWEVENT:1", 20000, NULL,FALSE)==FALSE)
		{
			//有可能未删除成功
			isFail = 1;
			// pDelConnect();
			// return FALSE;
			PrintString1("注册失败");
			
		}
		if(p_nblc_common_func(FALSE, NULL, "+MCWEVENT:7", 10000, NULL,FALSE)==FALSE)
		{
			//有可能未删除成功
			// pDelConnect();
			// return FALSE;
			isFail = 1;
			PrintString1("订阅失败");
		}
		if(isFail)
		{
			//手动平台更新状态成功
			p_uart_send_string("AT+MCWUPDATE");		
			//以更新CT状态为准
			if(p_nblc_common_func(FALSE, NULL, "+MCWEVENT:3", 20000, NULL,FALSE)==FALSE)
			{
				pDelConnect();
				return FALSE;
			}
		}
		
		g_run_info.m_last_nbiot_connect_time = my_time();
		return TRUE;
		#elif	 (NB_MODULE==NB_N306) || (NB_MODULE==NB_N701) 
		u8 isFail;
		isFail = 0;	
	
		#if NB_MODULE==NB_N306
		//查询pdp是否激活？
		if(p_nblc_common_func(TRUE,"CGPADDR","+CGPADDR",5000,NULL,TRUE)==FLASE)
		{
			return FALSE;
		}
		#else
		if(p_nblc_common_func(TRUE,"CGPADDR=1","+CGPADDR",5000,NULL,TRUE)==FLASE)
		{
			return FALSE;
		}
		#endif
		
		if(g_run_info.m_flg_n306)
		{
			#if SET_CQ_AEP
			p_nblc_common_func(TRUE,"CTM2MSETPM=\"chongqing-lwm2m.ctwing.cn\",5683,86400",NULL,1000,NULL,TRUE);
			#else
			p_nblc_common_func(TRUE,"CTM2MSETPM=\"221.229.214.202\",5683,86400",NULL,1000,NULL,TRUE);
			#endif
			p_nblc_common_func(TRUE,"CTM2MREG",NULL,1000,NULL,TRUE);
			//模块已连接
			if(p_nblc_common_func(FALSE, NULL, "reg", 5000, NULL,FALSE)==FALSE)
			{
				//有可能未删除成功
				isFail = 1;
				// pDelConnect();
				// return FALSE;
				PrintString1("注册失败");
				
			}
			//已订阅
			if(p_nblc_common_func(FALSE, NULL, "obsrv", 5000, NULL,FALSE)==FALSE)
			{
				//有可能未删除成功
				// pDelConnect();
				// return FALSE;
				isFail = 1;
				PrintString1("订阅失败");
			}
			if(isFail)
			{
				//手动平台更新状态成功
				p_uart_send_string("AT+CTM2MUPDATE");		
				//以更新CT状态为准
				if(p_nblc_common_func(FALSE, NULL, "update", 10000, NULL,FALSE)==FALSE)
				{				
					pDelConnect();
					
					return FALSE;
				}
			}
			else
			{
				PrintString1("平台成功\r\n");
			}

			#if  NB_MODULE==NB_N701
			g_run_info.m_last_nbiot_connect_time = my_time();
			#endif
		}
		else
		{
			p_nblc_common_func(TRUE,"NNMI=1",NULL,1000,NULL,TRUE);
			
			//p_nblc_common_func(TRUE,"NCDPOPEN=\"221.229.214.202\",5683",NULL,1000,NULL,TRUE);
			//p_nblc_common_func(TRUE,"QCFG=\"LWM2M/Lifetime\",864000",NULL,1000,NULL,TRUE);
			//p_nblc_common_func(TRUE,"NCDP=\"221.229.214.202\",5683",NULL,1000,NULL,TRUE);
			
			//p_nblc_common_func(TRUE,"NCFG=0,86400",NULL,1000,NULL,TRUE);
			p_nblc_common_func(TRUE,"NCFG=0,900",NULL,1000,NULL,TRUE);
			#if SET_CQ_AEP
			//if(p_nblc_common_func(TRUE,"NCDPOPEN=\"219.152.63.138\",5683",NULL,1000,NULL,TRUE))
			if(p_nblc_common_func(TRUE,"NCDPOPEN=\"chongqing-lwm2m.ctwing.cn\",5683",NULL,1000,NULL,TRUE))
			{
				
			}
			else
			{
				isFail = 1;
			}
			#else
			p_nblc_common_func(TRUE,"NCDPOPEN=\"221.229.214.202\",5683",NULL,1000,NULL,TRUE);
			#endif
			#if 0
			//模块已连接
			if(p_nblc_common_func(FALSE, NULL, "+QLWEVTIND: 0", 5000, NULL,FALSE)==FALSE)
			{
				//有可能未删除成功
				isFail = 1;
				// pDelConnect();
				// return FALSE;
				PrintString1("注册失败");
				
			}
			//已订阅
			if(p_nblc_common_func(FALSE, NULL, "+QLWEVTIND: 3", 5000, NULL,FALSE)==FALSE)
			{
				//有可能未删除成功
				// pDelConnect();
				// return FALSE;
				isFail = 1;
				PrintString1("订阅失败");
			}
			#endif
			
			#if SET_CQ_AEP
			if(isFail==0)
			{
				//已订阅
				if(p_nblc_common_func(FALSE, NULL, "+QLWEVTIND: 3", 10000, NULL,FALSE)==FALSE)
				{
					//有可能未删除成功
					// pDelConnect();
					// return FALSE;
					isFail = 1;
					PrintString1("订阅失败");
				}
			}
			#else
			//已订阅
			if(p_nblc_common_func(FALSE, NULL, "+QLWEVTIND: 3", 10000, NULL,FALSE)==FALSE)
			{
				//有可能未删除成功
				// pDelConnect();
				// return FALSE;
				isFail = 1;
				PrintString1("订阅失败");
			}
			#endif
			
			if(isFail)
			{
				#if 0
				//手动平台更新状态成功
				p_uart_send_string("AT+NMSTATUS?");		
				//以更新CT状态为准
				//if(p_nblc_common_func(FALSE, NULL, "OBSERVED", 10000, NULL,FALSE)==FALSE)
				if(p_nblc_common_func(FALSE, NULL, "OBSERVED", 1000, NULL,FALSE)==FALSE)
				{				
					pDelConnect();
					
					return FALSE;
				}
				#endif
				if(NB_getNMstatus()==FALSE)
				{
					pDelConnect();
					
					return FALSE;
				}
				#if SET_CQ_AEP
				else
				{
					PrintString1("已注册\r\n");
				}
				#endif
			}
			else
			{
				
				//p_nblc_common_func(TRUE,"NCFG=0",NULL,1000,NULL,TRUE);
				
				PrintString1("平台成功\r\n");
			}
		}
		
		return TRUE;
		#elif   NB_MODULE==NB_BC20
		if(p_nblc_common_func(TRUE, "CGPADDR=1", "+CGPADDR", 3000, NULL,FALSE)==FALSE)
		{
			// p_nblc_common_func(TRUE, "QLWCLOSE", NULL, 5000, NULL,FALSE);
			// p_nblc_common_func(TRUE, "QLWDEL", NULL, 2000, NULL,FALSE);
			return FALSE;
		}	
		
		p_nblc_common_func(TRUE,"QLWSERV=\"117.60.157.137\",5683",NULL,1000,NULL,TRUE);
		
		p_uart_send_string("AT+QLWCONF=\""); 
		for(i=0; i<15; i++)
          p_swut_write2buff(g_run_info.m_imei[i]);
        p_uart_send_string("\"");
		p_nblc_common_func(FALSE, NULL, "OK", 1000, NULL,FALSE);
		
		p_nblc_common_func(TRUE,"QLWADDOBJ=19,0,1,\"0\"",NULL,1000,NULL,TRUE);
		p_nblc_common_func(TRUE,"QLWADDOBJ=19,1,1,\"0\"",NULL,1000,NULL,TRUE);
		
		
		//if(p_nblc_common_func(TRUE,"QLWOPEN=0","CONNECT OK",20000,NULL,FALSE)==FALSE)
		p_uart_send_string("AT+QLWOPEN=0");
		if(p_nblc_common_func(FALSE, NULL, "CONNECT OK", 20000, NULL,FALSE)==FALSE)
		{
			p_nblc_common_func(TRUE, "QLWCLOSE", NULL, 5000, NULL,FALSE);
			p_nblc_common_func(TRUE, "QLWDEL", NULL, 2000, NULL,FALSE);
			return FALSE;
		}	
		
		//接收订阅请求
		if(p_nblc_common_func(FALSE, NULL, "+QLWOBSERVE:", 10000, NULL,FALSE)==FALSE)
		{			
			p_nblc_common_func(TRUE, "QLWCLOSE", NULL, 5000, NULL,FALSE);
			p_nblc_common_func(TRUE, "QLWDEL", NULL, 2000, NULL,FALSE);
			return FALSE;
		}
		
		//更新请求
		p_uart_send_string("AT+QLWUPDATE");
		if(p_nblc_common_func(FALSE, NULL, "UPDATE OK", 10000, NULL,FALSE)==FALSE)
		{			
			p_nblc_common_func(TRUE, "QLWCLOSE", "CLOSE OK", 5000, NULL,FALSE);
			p_nblc_common_func(TRUE, "QLWDEL", NULL, 2000, NULL,FALSE);
			return FALSE;
		}
		return TRUE;
		#else
		p_uart_send_string("AT+M2MCLINEW=117.60.157.137,5683,\"");              // 商用平台        
        //p_uart_send_string("AT+M2MCLINEW=180.101.147.115,5683,\"");             	// 测试平台      
			
		for(i=0; i<15; i++)
          p_swut_write2buff(g_run_info.m_imei[i]);
        p_uart_send_string("\"");
        p_uart_send_string(",");
        
		#ifdef	SET_PSM
		//p_uart_send_string("2592000,\"\",,86400"); 
		p_uart_send_string("2592000"); 
		#else
        p_uart_send_string("300");                 // 有效时间，5分钟
		#endif
//        if(p_nblc_common_func(FALSE, NULL, NULL, 1000, NULL,FALSE)){                    
          if(p_nblc_common_func(FALSE, NULL, "+M2MCLI:observe success", 20000, NULL,FALSE)){    // 不需要再等待OK
       //      p_nblc_communic_test();                            // 测试:OK  
            return TRUE;
          }   
//        }
        /* 连接失败 断开套接字 */
        p_nblc_common_func(TRUE, "M2MCLIDEL", "+M2MCLI:deregister success", 2000, NULL,FALSE);// 不需要再等待OK
        return FALSE;
		#endif
#else	
	/* 创建 udp socket */
	p_nblc_common_func(TRUE, "ESOC=1,2,1", NULL, 1000, p_nblc_proc_socket,FALSE);
	
	/* 连接 */
	if (p_nblc_common_func(TRUE, "ESOCON=0,"SERVER_PORT",\""SERVER_IP"\"", NULL, 1000, NULL,FALSE))
	{
		return TRUE;
	}

	/* 连接失败 断开套接字 */
	p_nblc_common_func(TRUE, "ESODIS=0", NULL, 1000, NULL,FALSE);
		
	return FALSE;
#endif
}


u8 p_nblc_port_send_data(u8 *udp_data, u16 len)
{
	u16 i;
	
#if (NB_COAP_EN == 1)   
		#if	 NB_MODULE==NB_MN316
		static u8 bStr;
		bStr++;
		if(bStr==0)
		{
			bStr++;
		}

		//+MLWCFG配置数据格式为TXT，CON消息，无RAI，无空口回传
		p_uart_send_string("AT+MCWSEND=");
		if(len>99)
		{
			p_swut_write2buff(len/100+'0');
			p_swut_write2buff(len/10%10+'0');
			p_swut_write2buff(len%10+'0');
		}
		else if(len>9)
		{
			p_swut_write2buff(len/10+'0');
			p_swut_write2buff(len%10+'0');
		}
		else
		{
			p_swut_write2buff(len+'0');
		}
		p_swut_write2buff(',');

		if (udp_data != NULL)
		{
			for (i = 0; i < len; i++)
			{
				p_swut_write2buff(itohex((udp_data[i] >> 4) & 0x0F));
				p_swut_write2buff(itohex((udp_data[i] >> 0) & 0x0F));
			}
		}
		p_uart_send_string(",1");
	
		//加一下空口标识
		p_swut_write2buff(',');
		if(bStr>99)
		{
			p_swut_write2buff(bStr/100+'0');
			p_swut_write2buff(bStr/10%10+'0');
			p_swut_write2buff(bStr%10+'0');
		}
		else if(bStr>9)
		{
			p_swut_write2buff(bStr/10+'0');
			p_swut_write2buff(bStr%10+'0');
		}
		else
		{
			p_swut_write2buff(bStr+'0');
		}
		#elif	(NB_MODULE==NB_N306) || (NB_MODULE==NB_N701) 
		if(g_run_info.m_flg_n306)
		{
			p_uart_send_string("AT+CTM2MSEND=");
			if (udp_data != NULL)
			{
				for (i = 0; i < len; i++)
				{
					p_swut_write2buff(itohex((udp_data[i] >> 4) & 0x0F));
					p_swut_write2buff(itohex((udp_data[i] >> 0) & 0x0F));
				}
			}
		}
		else
		{
			p_uart_send_string("AT+NMGS=");
			if(len>99)
			{
				p_swut_write2buff(len/100+'0');
				p_swut_write2buff(len/10%10+'0');
				p_swut_write2buff(len%10+'0');
			}
			else if(len>9)
			{
				p_swut_write2buff(len/10+'0');
				p_swut_write2buff(len%10+'0');
			}
			else
			{
				p_swut_write2buff(len+'0');
			}
			p_uart_send_string(",\"");
			
			if (udp_data != NULL)
			{
				for (i = 0; i < len; i++)
				{
					p_swut_write2buff(itohex((udp_data[i] >> 4) & 0x0F));
					p_swut_write2buff(itohex((udp_data[i] >> 0) & 0x0F));
				}
			}
			
			p_uart_send_string("\",100");
		}	
		#else			
		#if   NB_MODULE==NB_BC20	
		//RRC连接释放
		p_nblc_common_func(TRUE, "QNBIOTRAI=2", NULL, 1000, NULL,FALSE);
		
		p_uart_send_string("AT+QLWDATASEND=19,0,0,");
		//p_swut_write2buff(len/100+'0');
		p_swut_write2buff(len/10%10+'0');
		p_swut_write2buff(len%10+'0');
		p_swut_write2buff(',');
		#else
// AT+M2MCLISEND=0100
        p_uart_send_string("AT+M2MCLISEND=");
		#endif
        if (udp_data != NULL)
		{
			for (i = 0; i < len; i++)
			{
				p_swut_write2buff(itohex((udp_data[i] >> 4) & 0x0F));
				p_swut_write2buff(itohex((udp_data[i] >> 0) & 0x0F));
			}
		}		
		#endif
		
        /* 重设需要处理时间 */
		g_run_info.m_nb_stat.m_nbiot_send_cnt++;
		g_run_info.m_nbiot_start_proc_time = my_time();
		
		#if	NB_MODULE==NB_MN316
		//220415: 有发现Message Id一致的问题
		//CON消息模式，接收到平台回复的ACK包 
		//需要空口回传标识
		return p_nblc_common_func(FALSE, NULL, "+MCWEVENT:9", 10000, NULL,FALSE);
		
		#elif  (NB_MODULE==NB_N306) || (NB_MODULE==NB_N701) 
		if(g_run_info.m_flg_n306)
		{
			//CON数据发送成功
			return p_nblc_common_func(FALSE, NULL, "send", 10000, NULL,FALSE);
		}
		else
		{
			//CON数据发送成功
			return p_nblc_common_func(FALSE, NULL, "+QLWEVTIND: 4", 10000, NULL,FALSE);
		}	
		#elif	NB_MODULE==NB_BC20
		p_uart_send_string(",0x0100");
		 return p_nblc_common_func(FALSE, NULL, "SEND OK", 10000, NULL,FALSE);
		#else
 //       p_nblc_common_func(FALSE, NULL, NULL, 1000, NULL,FALSE);
        return p_nblc_common_func(FALSE, NULL, "+M2MCLI:notify success", 5000, NULL,FALSE);
		#endif
		
#else  
	/* 发送命令 */
	//"AT+NSOST=0,121.41.87.203,7005,5,400241C7B1"
	char tmp[20] = { 0x00 };

	p_uart_send_string("AT+ESOSEND=0,");
	if (len < 10)
	{
		tmp[0] = len + '0';
	}
	else if (len < 100)
	{
		tmp[0] = (len / 10) + '0';
		tmp[1] = (len % 10) + '0';
	}
	else
	{
		tmp[0] = (len / 100) + '0';
		tmp[1] = ((len % 100) / 10) + '0';
		tmp[2] = (len % 10) + '0';
	}

	p_uart_send_string(tmp);
	p_uart_send_string(",");


	if (udp_data != NULL)
	{
		for (u16 i = 0; i < len; i++)
		{
			p_swut_write2buff(itohex((udp_data[i] >> 4) & 0x0F));
			p_swut_write2buff(itohex((udp_data[i] >> 0) & 0x0F));
		}
	}
	/* 重设需要处理时间 */
	g_run_info.m_nb_stat.m_nbiot_send_cnt++;
	g_run_info.m_nbiot_start_proc_time = my_time();
	return p_nblc_common_func(FALSE, NULL, NULL, 5000, NULL,FALSE);
#endif
}


u8 p_nblc_proc_read_data(char *line)
{
	u16  i;
#if (NB_COAP_EN == 1)  
	#if   NB_MODULE==NB_MN316
	//自动接收模式
	//if (strstr(line, "+MCWREAD:") == NULL)
	if (strstr(line, "READ:") == NULL)
	#elif   (NB_MODULE==NB_N306) || (NB_MODULE==NB_N701) 
	if ((strstr(line, "CTM2MRECV") == NULL)&&(strstr(line, "NNMI") == NULL))
	#elif   NB_MODULE==NB_BC20
	if (strstr(line, "+QLWDATARECV:") == NULL)
	#else
    if (strstr(line, "+M2MCLIRECV:") == NULL)      
	#endif
	{               
		return FALSE;
	}     
#else  
	if (strstr(line, "0,") == NULL)
	{
		return FALSE;
	}
#endif  
	do
	{
		char *s = NULL;
		u16 recv_len = 0x00;
		u8 *b = g_c_m26l_rbuf;
		
		
        #if (NB_COAP_EN == 1)  
		// +M2MCLIRECV:3132333435
		// +M2MCLIRECV:B134560044003E0182065C384C510014D6
        /*     for(recv_len = 0; ; recv_len++){ 
                  if(line[recv_len] == '\r')
                    recv_len -= 12;
                    break;
                }*/
				#if	NB_MODULE==NB_MN316
				if ( (s = strtok(line, ":")) == NULL) { break; } 
				if ( (s = strtok(NULL, ",")) == NULL) { break; } // length
				if ((recv_len = atoi(s)) >= 512)
				{
					recv_len = 512;
				}
				//printf("RCV LEN:%d\r\n",recv_len);
				//if ( (s = strtok(NULL, ",")) == NULL) { break; } // socket
				if ( (s = strtok(NULL, ",")) == NULL) { break; } // data
				#elif  (NB_MODULE==NB_N306) || (NB_MODULE==NB_N701) 
				if(g_run_info.m_flg_n306)
				{
					if ( (s = strtok(line, ":")) == NULL) { break; } 
					if ( (s = strtok(NULL, ":")) == NULL) { break; } 
					s++;		//一个空格
					recv_len = strlen(s);
				}
				else
				{					
					if ( (s = strtok(line, ":")) == NULL) { break; } 
					
					
					if ( (s = strtok(NULL, ",")) == NULL) { break; } // length
					if ((recv_len = atoi(s)) >= 512)
					{
						recv_len = 512;
					}
					
					//printf("RCV LEN:%d\r\n",recv_len);
					//if ( (s = strtok(NULL, ",")) == NULL) { break; } // socket
					if ( (s = strtok(NULL, ",")) == NULL) { break; } // data	
					
				}
				#elif   NB_MODULE==NB_BC20
				s = strstr(line, ",");
				s = strstr(&s[1], ",");
				s = strstr(&s[1], ",");
				if(s!=NULL)
				{
					recv_len = atoi(&s[1]);
					s = strstr(&s[1], ",");
					s++;
				}
				else
				{
					break;
				}
				#else
                if ( (s = strtok(line, ":")) == NULL) { break; } // data
		if ( (s = strtok(NULL, ":")) == NULL) { break; } // data
                
                // recv_len = (s[18]-0x30)<<4;                       // 有效数据长度
                // recv_len |= (s[19]-0x30);
                // recv_len = recv_len + 11;
				recv_len = 512;	//按最长了算？
				#endif
                
        #else
                // +ESONMI=0,17,b13347003000420181065b45b5c5003ce6
		if ( (s = strtok(line, ",")) == NULL) { break; } // socket
		if ( (s = strtok(NULL, ",")) == NULL) { break; } // length
		if ((recv_len = atoi(s)) >= 512)
		{
			recv_len = 512;
		}

		if ( (s = strtok(NULL, ",")) == NULL) { break; } // data
        #endif        
		//u8 *b = g_c_m26l_rbuf;

		for (i = 0; i < recv_len; i++)
		{
			b[i]  = hextoi(s[i * 2]) << 4;
			b[i] |= hextoi(s[i * 2 + 1]);
         //               p_nblc_communic_test();                  // 长度测试OK
		}

		/* 服务器协议处理 */
		g_run_info.m_nb_stat.m_nbiot_recv_cnt++;
		if(p_prto_dispose_data(b, recv_len) == TRUE){
         //           p_nblc_communic_test();                       // 测试OK
                }

		/* 重设需要处理时间 */
		g_run_info.m_nbiot_start_proc_time = my_time();

                
		return TRUE;
	} while (0);

	return FALSE;
}


u8 p_nblc_read_data(void)
{
	#if (NB_COAP_EN == 1)   
		#if NB_MODULE==NB_MN316
		//return p_nblc_common_func(FALSE, NULL, "MCWREAD", 2, p_nblc_proc_read_data,TRUE);
		return p_nblc_common_func(FALSE, NULL, "READ", 2, p_nblc_proc_read_data,TRUE);
		#elif (NB_MODULE==NB_N306) || (NB_MODULE==NB_N701) 
		if(g_run_info.m_flg_n306)
		{
			return p_nblc_common_func(FALSE, NULL, "CTM2MRECV", 2, p_nblc_proc_read_data,TRUE);
		}
		else
		{
			return p_nblc_common_func(FALSE, NULL, "NNMI", 2, p_nblc_proc_read_data,TRUE);
		}
		#elif NB_MODULE==NB_BC20
		return p_nblc_common_func(FALSE, NULL, "QLWDATARECV", 2, p_nblc_proc_read_data,TRUE);
		#else
		return p_nblc_common_func(FALSE, NULL, "M2MCLIRECV", 2, p_nblc_proc_read_data,TRUE);
		#endif
	#else
	return p_nblc_common_func(FALSE, NULL, "ESONMI", 1000, p_nblc_proc_read_data,TRUE);
	#endif

}


u8 itohex(u8 aHex)
{
	if (/* (aHex >= 0) && */(aHex <= 9))
		aHex += 0x30;
	else if ((aHex >= 10) && (aHex <= 15))//A-F
		aHex += 0x37;
	else aHex = 0xff;
	return aHex;
}

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


u8 p_nblc_proc_vbatt(char *line)
{	
	char *s = NULL;
	#if  	(NB_MODULE==NB_BC260)||(NB_MODULE==NB_BC20)
	if (strstr(line, "CBC:"))	
	#elif (NB_MODULE==NB_N306) || (NB_MODULE==NB_N701) 
	if (strstr(line, "VBAT,"))
	#else
	if (strstr(line, "VBAT:"))
	#endif
	{
		//第2个逗号后面
		#if  	(NB_MODULE==NB_BC260)||(NB_MODULE==NB_BC20)
		s = strstr(line,":");
		if(s != NULL)
		{
			#if NB_MODULE==NB_BC260
			g_cbSta.bBatVal = atoi(&s[2]);
			#else
			g_cbSta.bBatVal = atoi(&s[1]);
			#endif
			// PrintString1("BAT:");
			// TX1_write2buff(g_cbSta.bBatVal/1000+'0');
			// TX1_write2buff(g_cbSta.bBatVal/100%10+'0');
			// TX1_write2buff(g_cbSta.bBatVal/10%10+'0');
			// TX1_write2buff(g_cbSta.bBatVal%10+'0');
		}
		#elif	(NB_MODULE==NB_N306) || (NB_MODULE==NB_N701) 
		s = strstr(line,",");
		if(s != NULL)
		{
			//g_cbSta.bBatVal = 3000;
			if((s[5]>='0')&&(s[5]<='9'))
			{
				g_cbSta.bBatVal = (s[2]-'0')*1000+(s[3]-'0')*100+(s[4]-'0')*10+(s[5]-'0');
			}
			else
			{
				g_cbSta.bBatVal = 0;
			}
		}	
		#else
		g_cbSta.bBatVal = atoi(&line[6]);
			#if NB_MODULE!=NB_MN316
			g_cbSta.bBatVal += 30;
			#endif
		#endif
		return TRUE;
	}
	return FALSE;
}


#if 0
void NB_ReadVbatt(void)
{
	#if   NB_MODULE==NB_MN316
		p_nblc_common_func(TRUE, "VBAT", NULL, 1000, p_nblc_proc_vbatt,FALSE);
	#elif	SET_VER==3
		p_nblc_common_func(TRUE, "CBC", NULL, 1000, p_nblc_proc_vbatt,FALSE);
	#else
		//读取电压
		#if	SET_ADC_BAT==1
		p_nblc_common_func(TRUE, "ZADC?", NULL, 1000, p_nblc_proc_adc,FALSE);
		#else
		p_nblc_common_func(TRUE, "VBAT?", NULL, 1000, p_nblc_proc_vbatt,FALSE);
		//g_cbSta.bBatVal =3000;
		#endif
	#endif
	
	#if  SET_SZKW
	if(g_cbSta.bBatVal>=3000)
	{
		g_cbSta.bBatPercent = 100;
	}
	else if(g_cbSta.bBatVal>2600)
	{
		//g_cbSta.bBatPercent = 10;
		g_cbSta.bBatPercent = (g_cbSta.bBatVal-2600)*90/400+10;
	}
	else if(g_cbSta.bBatVal>2500)
	{
		g_cbSta.bBatPercent = 5;
	}
	else if(g_cbSta.bBatVal<2500)
	{
		g_cbSta.bBatPercent = 0;
	}
	#else
	if(g_cbSta.bBatVal<2500)
	{
		LOG("电池低压\r\n");
		g_cbSta.isBattLow = 1;
	}
	else
	{
		g_cbSta.isBattLow = 0;
	}
	if(g_cbSta.isBattLow && (g_cbSta.isBattLowSnd==0))
	{
		g_cbSta.isBattLowSnd = 1;
		LOG("电池低压上报\r\n");
		//低压事件，开机只发一次
		Event_AddEvent(EVENT_LOWBATT);
	}
	#endif
}
#endif

#ifdef	SET_PSM
void NB_PsmSleep(void)
{
	//static u8 flagPowerOn =0;
	
	//u8 bTry;
	g_run_info.m_flg_psm = FALSE;
	//for(bTry = 0;bTry<2;bTry++)
	{
		#if (NB_MODULE==NB_N306) || (NB_MODULE==NB_N701) 
		p_nblc_common_func(TRUE, "ECPURC=\"HIBNATE\",1", NULL, 1000, NULL,FALSE);
		p_nblc_common_func(TRUE, "ECCFG=\"T3324MaxValueS\",2", NULL, 1000, NULL,FALSE);
		p_nblc_common_func(TRUE, "CPSMS=1", NULL, 1000, NULL,FALSE);	
		#else	
		//进入PSM模式
	    p_nblc_common_func(TRUE, "CPSMS=1,,,\"00011000\",\"00000001\"", NULL, 1000, NULL,FALSE);	
		#endif	
		
		#if NB_MODULE==NB_MN316
		p_nblc_common_func(TRUE, "WORKLOCK=0", NULL, 1000, NULL,FALSE);	//释放省电锁
		//开机第一次进入PSM的时间慢一点，之后都快速释放
		// if(flagPowerOn==0)
		// {
			// flagPowerOn = 1;
		// }
		// else
		{
			p_nblc_common_func(TRUE, "XYRAI", NULL, 1000, NULL,FALSE);
		}
		#elif  (NB_MODULE==NB_N306) || (NB_MODULE==NB_N701) 
		p_nblc_common_func(TRUE, "ECNBIOTRAI=1", NULL, 1000, NULL,FALSE);	
		//p_nblc_common_func(TRUE, "NYGOSLEEP", NULL, 1000, NULL,FALSE);	
		p_nblc_common_func(TRUE, "ECPMUCFG=1,4", NULL, 1000, NULL,FALSE);
		#elif  NB_MODULE==NB_BC20
		p_uart_send_string("AT+QNBIOTEVENT=1,1");
		p_nblc_common_func(FALSE, NULL, NULL, 1000, NULL,FALSE);
		p_nblc_common_func(TRUE, "QSCLK=1", NULL, 1000, NULL,FALSE);
		p_nblc_common_func(TRUE, "QRELLOCK", NULL, 1000, NULL,FALSE);
		#else
		/* 打开PSM主动上报 */
		p_uart_send_string("AT*MNBIOTEVENT=1,1");
		p_nblc_common_func(FALSE, NULL, NULL, 1000, NULL,FALSE);
		
		//主动释放 RRC 连接
		p_uart_send_string("AT*RRCRELEASE");
		p_nblc_common_func(FALSE, NULL, NULL, 1000, NULL,FALSE);
		//ZSLR
		p_nblc_common_func(TRUE, "ZSLR", NULL, 1000, NULL,FALSE);
		#endif
		//等待过程又有
		//等待
		// if(TRUE == p_nblc_common_func(FALSE, NULL, "ENTER PSM", 30000, NULL,FALSE))
		// {
			// g_run_info.m_flg_psm = TRUE;
			// break;
		// }
		#if  NB_MODULE==NB_BC20
		g_bWaitTimeOut1s = 40;
		#else
		//等待进入PSM模式
		g_bWaitTimeOut1s = 10;
		#endif
		while(g_bWaitTimeOut1s!=0)
		{
			#if	SET_EX_WAKE==1
			//等待期间又产生事件
			if(g_cbSta.bDevTimeOut1s!=0)
			#else
			if(!Event_IsEmpty())
			#endif
			{
				g_device_info.isNbIdle = 0;
				//g_isIdle = 0;
				#if  NB_MODULE==NB_BC20
				p_nblc_common_func(TRUE, "QSCLK=0", NULL, 1000, NULL,FALSE);
				#elif (NB_MODULE==NB_N306) || (NB_MODULE==NB_N701) 
				p_nblc_common_func(TRUE, "ECPMUCFG=1,1", NULL, 1000, NULL,FALSE);	
				
				#else
				if(FLASE==p_nblc_common_func(TRUE, "CPSMS=0", NULL, 1000, NULL,FALSE))
				{
					g_run_info.m_flg_psm = TRUE;
					Sleep_ms(20);
					g_u16_m26l_rindex = 0;
				}
				#endif
				break;
			}
			else
			{
				#if NB_MODULE==NB_MN316
				if(TRUE == p_nblc_common_func(FALSE, NULL, "POWERDOWN:", 1000, NULL,FALSE))	
				#elif (NB_MODULE==NB_N306) || (NB_MODULE==NB_N701) 				
				//if(TRUE == p_nblc_common_func(FALSE, NULL, "HIB Enter", 1000, NULL,FALSE))
				//+QNBIOTEVENT: "ENTER DEEPSLEEP"
				//if(strstr(g_c_m26l_rbuf, "HIB Enter") != NULL)
				if(strstr(g_c_m26l_rbuf, "ENTER") != NULL)
				#else
				if(TRUE == p_nblc_common_func(FALSE, NULL, "ENTER PSM", 1000, NULL,FALSE))
				#endif
				{
					Sleep_ms(20);
					g_u16_m26l_rindex = 0;
					g_run_info.m_flg_psm = TRUE;

					break;
				}
			}		
		}	
	}
}

#if SET_USE_PSM
u8 NB_PsmWake(void)
{	
	LOG("唤醒模块\r\n");
	#if NB_MODULE==NB_MN316
	SET_ME3616_NB_ON(ON_RST_NB);
	//复位：高电平脉冲大于6s
	//唤醒：高电平脉冲大于100us，小于5s
	Sleep_ms(100);
	//ME3616_NB_ON  = OFF_WAKE_NB;	
	SET_ME3616_NB_ON(OFF_RST_NB);
	Sleep_ms(50);	//释放唤醒信号40ms以上再进行AT命令发送操作
	#elif (NB_MODULE==NB_N306) || (NB_MODULE==NB_N701) 
	SET_MN306_WAKE(ON_WAKE_NB);
	//唤醒：低电平脉冲大于100ms
	Sleep_ms(200);
	SET_MN306_WAKE(OFF_WAKE_NB);
	Sleep_ms(50);	//释放唤醒信号40ms以上再进行AT命令发送操作
	#endif
	g_bWaitTimeOut1s = 3;
	while(g_bWaitTimeOut1s!=0)
	{
		if (p_nblc_communic_test() == TRUE)
		{
			return TRUE;
		}
	}
	return FALSE;
	// /* 核实模块注册上了没 */
	// if (TRUE == p_nblc_get_cereg())
	// {
		// return TRUE;
	// }	
	// else
	// {
		// return FALSE;
	// }
}
#endif


#endif

#if NB_MODULE==NB_N306
void NB_GetN306Ver(void)
{
	u32 	 start_time = my_time();
	p_uart_send_string("AT+CGMR\r\n");
	while ((my_time() - start_time) < 1000)
	{
		if(strstr(g_c_m26l_rbuf, "N306"))
		{
			g_run_info.m_flg_n306 = 1;
		}
		else if(strstr(g_c_m26l_rbuf, "BC2"))
		{
			g_run_info.m_flg_n306 = 0;
		}
		
		if(strstr(g_c_m26l_rbuf, "OK"))
		{
			break;
		}	
	}
}
#endif

u8 p_me36_init(void)
{
	u8 sim_status = FALSE;
	u8 ca_status = FALSE;
	u8 at_ok_times = 0x00;
	u32 timout;
	//u8  i;
	u8 bTry;
	
	p_sn12_gpio_init();
	p_sn12_boot();

	//230104:测试模式
	if(g_device_info.test_mode)
	{
		#if (NB_MODULE==NB_N701)
		for (timout = my_time() + 15000; my_time() < timout; )
		#else
		/* 等待启动完成 */
		for (timout = my_time() + 5000; my_time() < timout; )
		#endif
		{
			//230617:开机过程进入标定则直接退出
			if(device_info.state.F_SM_BD)
			{
				g_cbSta.bNetTimeOut1s = 0;
				g_cbSta.isBd = 1;
				
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
				
				LOG("开机标定退出\n");
				return 0;
			}

			if (p_nblc_communic_test() == TRUE)
			{		
				if(g_cbSta.isBattRead==0)
				{
					g_cbSta.isBattRead = 1;
					
					//NB_ReadVbatt();		
				}

				//改成2次 略微提升速度
				//if (++at_ok_times >= 2)
				if(++at_ok_times)
				{
					#if 0
					/*230608:锁一下物理小区ID*/
					if(g_run_info.cellid!=0)
					{
						char  Tmp_str[50];
						//p_nblc_common_func(TRUE, "CFUN=0", NULL, 1000, NULL,FALSE);

						//p_nblc_common_func(TRUE, "ECFREQ=?", NULL, 1000, NULL,FALSE);

						sprintf(Tmp_str,"AT+ECFREQ=2,%ld,%d",g_run_info.earfcn,g_run_info.cellid);
						p_uart_send_string(Tmp_str);
						p_nblc_common_func(FALSE, NULL, "OK", 1000, NULL,FALSE);

						p_nblc_common_func(TRUE, "ECFREQ?", NULL, 1000, NULL,FALSE);
					}		
					#endif

					timout = 0x00;
				}			
			}
	
			Sleep_ms(500);
		}
		
		if(at_ok_times==0)
		{
			g_cbSta.bNetTimeOut1s = 0;
			LOG("开机超时退出\n");
		}
		else
		{
			at_ok_times = 2;
			
		}
	}
	else
	{
		/* 等待启动完成 */
		for (timout = my_time() + 20000; my_time() < timout; )
		{
			if (p_nblc_communic_test() == TRUE)
			{		
				if(g_cbSta.isBattRead==0)
				{
					g_cbSta.isBattRead = 1;
					
					//NB_ReadVbatt();				
				}

				//改成2次 略微提升速度
				if (++at_ok_times >= 2)
				{
					timout = 0x00;
				}
				
			}
	
			Sleep_ms(500);
		}
	}
		
	

	//下次可再读取
	//g_cbSta.isBattRead = 0;
	
	if (at_ok_times >= 2)
	{
		#if 0	
		//测试
		p_uart_send_string("ATE1\r\n");
		#else
		//关闭回显
		p_uart_send_string("ATE0\r\n");
		#endif
		Sleep_ms(100);

		g_run_info.m_flg_psm = FLASE;
	
		/* 开启全功能 */
		p_nblc_set_npsmr();
                
		#if (NB_COAP_EN == 1)  
			#if  NB_MODULE==NB_BC20
			p_nblc_common_func(TRUE, "QLWCFG=\"dataformat\",1,1", NULL, 1000, NULL,FALSE);	//文本格式
			p_nblc_common_func(TRUE, "QLWCFG=\"lifetime\",0", NULL, 1000, NULL,FALSE);
			#elif NB_MODULE==NB_ME3616
			p_nblc_common_func(TRUE, "M2MCLICFG=0,0", NULL, 1000, NULL,FALSE);    // 设置数据上报和接收为Hex数据
			#endif
		#endif
		
			
		/* 查卡30s,超时返回 */
		//for (i = 0; i < C_SN12_WAIT_SIM_SEC; i++)
		for (timout = my_time() + 20000; my_time() < timout; )	
		{	
			//230617:开机过程进入标定则直接退出
			if(device_info.state.F_SM_BD && g_device_info.test_mode)
			{
				g_cbSta.bNetTimeOut1s = 0;
				g_cbSta.isBd = 1;
				
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
				return 0;
			}

			//p_uart_send_string("ATE0\r\n");
			Sleep_ms(1000);

			/* 顺便给stm32干活 */
			//p_nblc_proc_at_cmd();

			/* 读取软件版本号 */
			#if NB_MODULE==NB_N306
			NB_GetN306Ver();
			#elif	NB_MODULE==NB_N701 
			g_run_info.m_flg_n306 = 1;
			#else
			p_nblc_common_func(TRUE, "CGMR", NULL, 1000, NULL,FALSE);
			#endif
			
			/* 查询设备标识 */
			if (p_nblc_get_cimi() == FALSE)
			{
				continue;
			}

			/* 读取SIM卡ID */
			if (p_nblc_get_iccid() == FALSE)
			{
				continue;
			}

			/* 读取imei */
			if (p_nblc_get_imei() == FALSE)
			{
				continue;
			}


			if (FALSE == p_nblc_common_func(TRUE, "CEREG=4",NULL,1000, NULL,FALSE))
			{
				continue;
			}
			
			#if	NB_MODULE!=NB_MN316 
				
			#ifdef	SET_PSM				
			//默认ZSLR=0
			//指令中包含重启生效指令，故需要重启模组。重启模组后，等待 T3324 超时，模组进入 PSM	
			// if (FALSE == p_nblc_common_func(TRUE, "ZSLR=0", NULL, 1000, NULL,FALSE))
			// {
				// continue;
			// }		
			/* 打开节能模式 */
			//测试10s
			//if (FALSE == p_nblc_common_func(TRUE, "CPSMS=1,,,\"01100101\",\"00000001\"", NULL, 1000, NULL,FALSE))
//				if (FALSE == p_nblc_common_func(TRUE, "CPSMS=1,,,\"00101111\",\"00000001\"", NULL, 1000, NULL,FALSE))  //15小时
//				{
//					continue;
//				}		

				#if	NB_MODULE==NB_BC20
				p_nblc_common_func(TRUE, "QSCLK=0",NULL,1000, NULL,TRUE);
				p_nblc_common_func(TRUE, "QNBIOTEVENT=1,1",NULL,1000, NULL,TRUE);
				#elif		(NB_MODULE==NB_N306) || (NB_MODULE==NB_N701) 
				p_nblc_common_func(TRUE, "ECPMUCFG=1,1",NULL,1000, NULL,TRUE);	
				//p_nblc_common_func(TRUE, "NYSM",NULL,1000, NULL,TRUE);
				#else
				/* 打开PSM主动上报 */
				p_uart_send_string("AT*MNBIOTEVENT=1,1");
				if (FALSE == p_nblc_common_func(FALSE, NULL, NULL, 1000, NULL,FALSE))
				{
					continue;
				}	
				#endif

			#else
			/* 打开节能模式 */
			if (FALSE == p_nblc_common_func(TRUE, "CPSMS=1", NULL, 1000, NULL,FALSE))
			{
				continue;
			}
			#endif
			
			#endif
				
			
			/* 查询卡准备好了没 */
			if (FALSE == p_nblc_common_func(TRUE, "CPIN?", "READY", 1000, NULL,TRUE))
			{
				continue;
			}
			
			
			/* 配置APN */
			//if (FALSE == p_nblc_common_func(TRUE, "EGACT=1,1,\"ctnet\",\"card\",\"card\"", NULL, 1000, NULL,TRUE))
			//{
				//continue;
				//自动拨号,可以不需配置
			//}
                        
       		p_nblc_get_csq();

					sim_status = TRUE;
                        
            break;
		}
		
		#if	NB_MODULE==NB_N701
		if((sim_status == FALSE)&& g_device_info.test_mode)
		#else
		//没有卡不再重启，直接关机
		if(sim_status == FALSE)
		#endif
		{
			g_cbSta.bNetTimeOut1s = 0;
		}
	}

	/* 有卡 */
	//修改成不需要更改扰码算法，1分钟注册不上就重启模块！！
	if (sim_status == TRUE)
	{
		/* APN设置 */
		//p_nblc_common_func(TRUE, "CGDCONT=1,\"IP\",\"CTNB\"", NULL, 1000, NULL,FALSE);
		//p_nblc_common_func(TRUE, "CGDCONT=1,\"IP\",\"SXZCAT1\"", NULL, 1000, NULL,FALSE);
			
		//for (i = 0; i < C_SN12_WAIT_CS_PS_SEC; i++)
		#if 	NB_MODULE==NB_MN316
		for(bTry=0;bTry<2;bTry++)
		{
			p_nblc_common_func(TRUE, "CFUN=1", NULL, 1000, NULL,TRUE);
			g_bWaitTimeOut1s = 120;
		#else	
			//g_bWaitTimeOut1s = 120;
			//230710:改成100秒
			g_bWaitTimeOut1s = 100;
		#endif
			while(g_bWaitTimeOut1s!=0)
			{
				//230617:开机过程进入标定则直接退出
				if(device_info.state.F_SM_BD && g_device_info.test_mode)
				{
					g_cbSta.bNetTimeOut1s = 0;
					g_cbSta.isBd = 1;
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
					return 0;
				}

				#if NB_MODULE==NB_N306
				//20s还没注网则清除锁定的频点
				if(g_bWaitTimeOut1s==100)
				{
					g_bWaitTimeOut1s = 99;
					p_nblc_common_func(TRUE, "ECBAND=0,5,8", NULL, 1000, NULL,FALSE);
				}
				#endif

				/* 核实模块注册上了没 */
				if (TRUE == p_nblc_get_cereg())
				{
					ca_status = TRUE;
					
					#if	NB_MODULE==NB_MN316
					//NV值默认标准深睡模式不需要设置
					p_nblc_common_func(TRUE, "NV=GET,IPALIVE", NULL, 1000, NULL,FALSE);
					p_nblc_common_func(TRUE, "NV=GET,RAI", NULL, 1000, NULL,FALSE);
					p_nblc_common_func(TRUE, "NV=GET,OFFTIME", NULL, 1000, NULL,FALSE);
					p_nblc_common_func(TRUE, "NV=GET,DOWNDATA", NULL, 1000, NULL,FALSE);
					//p_nblc_common_func(TRUE, "NRB", NULL, 1000, NULL,FALSE));
					#endif
					//关闭PSM
					p_nblc_common_func(TRUE, "CPSMS=0", NULL, 1000, NULL,FALSE);
					
					
					//测试
					//p_nblc_common_func(TRUE, "CEDRXS?", NULL, 1000, NULL,FALSE);
					#if NB_MODULE!=NB_N701
					p_nblc_common_func(TRUE, "CEDRXS=0", NULL, 1000, NULL,FALSE);
					#endif
					//p_nblc_common_func(TRUE, "CEDRXS?", NULL, 1000, NULL,FALSE);
					
					break;
				}
				
				Sleep_ms(500);
				//Sleep_ms(1000);	
			}
			#if	NB_MODULE==NB_MN316
			if(FALSE==ca_status)
			{
				p_nblc_common_func(TRUE, "CFUN=0", NULL, 1000, NULL,TRUE);	
				p_nblc_common_func(TRUE, "NCSEARFCN", NULL, 5000, NULL,TRUE);
			}
			else
			{
				break;
			}
							
		}
		#endif
	}	
	
	return ca_status;
}

#if 0
u8 p_nblc_proc_cellid(char *line)
{
	char *s = NULL;
	
	//当前服务小区频点：0 ~ 262143
	//cellId
	//需要全部收取完毕
	if(strstr(line, "\""))
	{
		//获取频点
		s = strstr(line,":");	//:号
		if(s != NULL)
		{
			g_run_info.earfcn = atoi((s+1));
			s = strstr(s,",");	//第1个逗号
			if(s != NULL)
			{
				g_run_info.cellid = atoi((s+1));
				return TRUE;
			}
		}
	}
	return FALSE;
	
}

u8 p_nblc_get_cellid(void)
{
	return p_nblc_common_func(TRUE, "ECBCINFO=1,4,0,1,1", "\"", 4000, p_nblc_proc_cellid,FALSE);
}
#endif

void NB_pro(void)
{
	#if SET_USE_PSM
	//需要唤醒模块
	if(g_run_info.m_nbiot_init_ok == FALSE)
	{
		if(TRUE==g_run_info.m_flg_psm)
		{
			if(FALSE==NB_PsmWake())
			{
				g_run_info.m_nbiot_init_ok = FALSE;
			}
			else
			{			
				//NB_ReadVbatt();
				g_run_info.m_flg_psm = FLASE;
				#if (NB_MODULE==NB_N306) || (NB_MODULE==NB_N701) 
				p_nblc_common_func(TRUE, "ECPMUCFG=1,1", NULL, 1000, NULL,FALSE);
				
				//230608:增加超时查询链路状态
				g_bWaitTimeOut1s = 8;
				while(g_bWaitTimeOut1s!=0)
				{
					/*查询卡准备好了没*/
					if(FALSE == p_nblc_common_func(TRUE, "CTM2MREG?","+CTM2MREG: 1",1000, NULL,TRUE))
					{
						continue;
					}	
					g_run_info.m_nbiot_init_ok = TRUE;	
					break;
				}
				#else
				p_nblc_common_func(TRUE, "CPSMS=0", NULL, 1000, NULL,FALSE);
				#endif
			}
		}		
	}
	#endif

	/* 初始化NBIOT */
	if (/*(g_run_info.m_nbiot_canbe_init == TRUE) &&*/
			((g_run_info.m_nbiot_init_ok == FALSE) || (g_run_info.m_nbiot_err_cnt >= 10))
			)
	{
		
		g_run_info.m_nbiot_init_ok = FALSE;
		g_run_info.m_nbiot_err_cnt = 0x00;
		g_run_info.m_nbiot_create_sock_ok = FALSE;
		

		g_run_info.m_nbiot_init_ok = p_sn12_init();
	
		//g_run_info.m_nbiot_canbe_init == FALSE;
	}
	/* NBIOT连接网络 */
	if ((g_run_info.m_nbiot_init_ok == TRUE) &&
		(g_run_info.m_nbiot_create_sock_ok == FALSE) )
	{
		if ((g_run_info.m_nbiot_create_sock_ok = p_nblc_UDP_connect_net()) == FALSE)
		{
		  #if (NB_COAP_EN == 1)   
			g_run_info.m_nbiot_err_cnt += 5;        // 等待时间长，重发2次即可；        
		  #else
			g_run_info.m_nbiot_err_cnt++;                                  
		  #endif
		}else{
		//   p_nblc_communic_test();                     // 测试
		}
	}
	
	/* NBIOT正常处理流程 */
	if ((g_run_info.m_nbiot_init_ok == TRUE) &&
		(g_run_info.m_nbiot_create_sock_ok == TRUE) /*&&
		(memcmp(s_null_guid, g_run_info.m_dev_uuid, sizeof(s_null_guid)) != 0x00)*/)
	{

		//正在发送不能发数据
		//推迟2秒再发送
		//if((FALSE==g_run_info.m_flg_sending) /*&& (g_cbSta.bDelayRe10ms==0)*/)
		//if((FALSE==g_run_info.m_flg_sending)&&((g_cbSta.flag_Init & SAMPLE_ALL_OK) == SAMPLE_ALL_OK))
		if((FALSE==g_run_info.m_flg_sending)&& TouchSend.isGet)
		{
			if (g_run_info.m_last_nbiot_resend_count > 3)
			{
				g_run_info.m_last_nbiot_resend_count = 0;
				g_run_info.m_flg_retry = 0;
			}
			
			NB_TxPro();
		}
			
		/* 是否有NBIOT数据需要接收
		 * 顺便检查一下信号
		 */
		//if ((my_time() - g_nbiot_last_read_time) > 1000)
		if (((my_time() - g_nbiot_last_read_time) > 1000)&& (COM1.RX_TimeOut==0))
		{
			g_nbiot_last_read_time = my_time();
					
			if (p_nblc_read_data() == FALSE)
			{
			}
					
			/* 判断无信号则也算错误 */
			//if ((p_nblc_get_csq() == FALSE) || (g_run_info.m_csq == 99))
			if ((p_nblc_get_csq() == FALSE) || (g_run_info.m_csq == 0xff) || (FALSE == p_nblc_get_cereg()))
			{
					g_run_info.m_nb_stat.m_nbiot_no_signal_cnt++;
					
					if(g_run_info.m_nb_stat.m_nbiot_no_signal_cnt>2)
					{
						g_run_info.m_nbiot_err_cnt++;
					}			
			}
			else
			{
					static u8 s_Cnt;
				
					g_run_info.m_nb_stat.m_nbiot_no_signal_cnt = 0;
					
				
					//230718:增加查IP
					s_Cnt++;
					if((s_Cnt%5) ==0)
					{
						#if NB_MODULE==NB_N701
						#else
//						  p_nblc_common_func(TRUE,"CGPADDR","+CGPADDR",3000,NULL,TRUE);
						NB_getNMstatus();
						  //p_nblc_common_func(TRUE, "CEDRXRDP", NULL, 5000, NULL,FALSE);
													
//							p_uart_send_string("AT+CTM2MUPDATE");		
//						  p_nblc_common_func(FALSE, NULL, "update", 10000, NULL,FALSE);
						#endif
					}
			}
			
			#if  NB_MODULE==NB_MN316
			//超过1分钟与平台失去连接
			if ((my_time() - g_run_info.m_last_nbiot_connect_time) > (60*1000))	
			{
				//手动平台更新状态成功
				p_uart_send_string("AT+MCWUPDATE");		
				//以更新CT状态为准
				if(p_nblc_common_func(FALSE, NULL, "+MCWEVENT:3", 20000, NULL,FALSE)==FALSE)
				{				
					p_uart_send_string("AT+MCWUPDATE");		
					//以更新CT状态为准
					if(p_nblc_common_func(FALSE, NULL, "+MCWEVENT:3", 20000, NULL,FALSE)==FALSE)
					{
						pDelConnect();
					}
					else
					{
						g_run_info.m_last_nbiot_connect_time = my_time();
					}
					
				}
				else
				{
					g_run_info.m_last_nbiot_connect_time = my_time();
				}
			}
			#elif NB_MODULE==NB_N701
			//超过1分钟与平台失去连接
			if ((my_time() - g_run_info.m_last_nbiot_connect_time) > (60*1000))	
			{
				//手动平台更新状态成功
				p_uart_send_string("AT+CTM2MUPDATE");		
				//以更新CT状态为准
				if(p_nblc_common_func(FALSE, NULL, "update", 10000, NULL,FALSE)==FALSE)
				{				
					//手动平台更新状态成功
					p_uart_send_string("AT+CTM2MUPDATE");		
					//以更新CT状态为准
					if(p_nblc_common_func(FALSE, NULL, "update", 10000, NULL,FALSE)==FALSE)
					{				
						pDelConnect();						
					}
					else
					{
						g_run_info.m_last_nbiot_connect_time = my_time();
					}
				}
				else
				{
					g_run_info.m_last_nbiot_connect_time = my_time();
				}
			}

			
			#endif
		}
		
		/* 判断UDP数据包是否收到反馈 */
		/*10s没有收到反馈数据则重发*/
		if (TRUE == g_run_info.m_flg_sending
			&& ((my_time() - g_run_info.m_last_nbiot_send_time) > (10*1000)))
		{
			g_run_info.m_flg_retry = TRUE;
			g_run_info.m_flg_sending = FLASE;
			g_run_info.m_last_nbiot_resend_count++;
			LOG("重发\n");
			
		}
	}
		
	//发送前清接收缓冲可能造成接收不到数据
	//发送前不清，可能造成ok是上一个的数据！！
}

#if NB_MODULE==NB_N306	
void NB_setBand(void)
{
	u32 	 start_time;
	 
	#if 1
	start_time = my_time();
	p_uart_send_string("AT+ECSTATUS=PHY\r\n");
	while ((my_time() - start_time) < 1000)
	{
		if(strstr(g_c_m26l_rbuf, "Band:5"))
		{
			p_nblc_common_func(TRUE, "ECBAND=0,5", NULL, 1000, NULL,FALSE);
			break;
		}
		else if(strstr(g_c_m26l_rbuf, "Band:8"))
		{
			p_nblc_common_func(TRUE, "ECBAND=0,8", NULL, 1000, NULL,FALSE);
			break;
		}
	}
	p_nblc_common_func(TRUE, "ECBAND?", NULL, 1000, NULL,FALSE);

	#else
	g_run_info.cellid = 0;
	//p_nblc_get_cellid();
	#if 0
	/*读一下服务小区*/
	start_time = my_time();
	p_uart_send_string("AT+ECBCINFO=1,4,0,1,1\r\n");
	while ((my_time() - start_time) < 4000)
	{
		//当前服务小区频点：0 ~ 262143
		//cellId
		//需要全部收取完毕
		if(strstr(g_c_m26l_rbuf, "\""))
		{
			char *s = NULL;
			//获取频点
			s = strstr(g_c_m26l_rbuf,":");	//:号
			if(s != NULL)
			{
				g_run_info.earfcn = atoi((s+1));
				s = strstr(s,",");	//第1个逗号
				if(s != NULL)
				{
					g_run_info.cellid = atoi((s+1));
					break;
				}
				break;
			}
		}
	}
	#else
	g_run_info.earfcn = 2506;
	g_run_info.cellid = 80;
	#endif

	#if 1
	//锁频段修改成锁频点和物理小区
	//if(TRUE==p_nblc_get_cellid())
	{
		/*230608:锁一下物理小区ID*/
		if(g_run_info.cellid!=0)
		{
			char  Tmp_str[50];
			p_nblc_common_func(TRUE, "CFUN=0", NULL, 1000, NULL,FALSE);

			p_nblc_common_func(TRUE, "ECFREQ=?", NULL, 1000, NULL,FALSE);

			sprintf(Tmp_str,"AT+ECFREQ=2,%ld,%d",g_run_info.earfcn,g_run_info.cellid);
			p_uart_send_string(Tmp_str);
			p_nblc_common_func(FALSE, NULL, "OK", 1000, NULL,FALSE);

			p_nblc_common_func(TRUE, "ECFREQ?", NULL, 1000, NULL,FALSE);
		}
		//g_run_info.cellid = 0;
	}
	#endif
	#endif

	return;
	
}
#endif

#endif



