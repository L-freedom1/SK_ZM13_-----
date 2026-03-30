
#include "json.h"

#if SET_PROTO_VER==PROTO_VER_JSON

#if SET_SPI

#if SET_CAT1
	#if SET_TEST
	char const CODE_VER[] = "SK-CAT1-241125T";
	#else
	char const CODE_VER[] = "SK-CAT1-250306";
	#endif
#else
	#if SET_TEST
	char const CODE_VER[] = "SK-3.0-250506T";
	#else
	char const CODE_VER[] = "SK-3.0-260105";
	#endif
#endif

#else
char const CODE_VER[] = "SK-2.0-230710";
#endif


char  json_buf[BUFFSIZE] = { 0 };

//service_t  service;

void MyJson_encode_begin(char* payload_data, int payload_len)
{
	int index = strlen(payload_data);
	int left = payload_len - index - 1;
	strncat(payload_data, "{", left);
}


void MyJson_encode_end(char* payload_data, int payload_len)
{
	int index = strlen(payload_data);
	int left = payload_len - index - 1;
	strncat(payload_data, "}", left);
}


void MyJson_encode_string(char* payload_data, /*int payload_len,*/ const char* item, char* value, int is_last)
{
	int index = strlen(payload_data);
	//int left = payload_len - index - 1;
	sprintf(&payload_data[index], "\"%s\":\"%s\"%s", item, value, (is_last == TRUE) ? "" : ",");
	// 从数据末尾的下一个字节开始写入left个字节，按照""的格式
}


void MyJson_encode_number(char* payload_data, /*int payload_len,*/ const char* item, int value, int is_last)
{
	int index = strlen(payload_data);
	//int left = payload_len - index - 1;
	sprintf(&payload_data[index], "\"%s\":%d%s", item, value, (is_last == TRUE) ? "" : ",");
}

void MyJson_encode_float(char* payload_data, /*int payload_len,*/ const char* item, float value, int is_last)
{
	int index = strlen(payload_data);
	//int left = payload_len - index - 1;
	sprintf(&payload_data[index], "\"%s\":%.2f", item, value/*, (is_last == TRUE) ? "" : ","*/);
	index = strlen(payload_data);
	if(is_last != TRUE)
	{
		payload_data[index] = ',';
	}
}


u8 MyJson_pack(char* p_pack_buf, int p_pack_long,u16 taskId)
{
	u16 wDat;
	
	if (p_pack_buf == NULL || p_pack_long <= 0)
		return FALSE;

	MyJson_encode_begin(p_pack_buf, p_pack_long);

	//指令下发响应
	if(service.serviceID>9000)
	{
		//MyJson_encode_number(p_pack_buf,/* p_pack_long,*/ "taskId", service.taskId, FALSE);
		MyJson_encode_number(p_pack_buf,/* p_pack_long,*/ "taskId", taskId, FALSE);
	}
	
	MyJson_encode_number(p_pack_buf,/* p_pack_long,*/  "serviceId", service.serviceID, FALSE);
	switch(service.serviceID)
	{
		//事件上报
		//心跳
		case 4:
		//按键测试上报
		case 1004:
		//有可能上报的时候烟雾已经散去了
		//烟感监测上报
		case 1009:
		case 1010:
		//环境温度监测上报
		case 1005:
		case 1008:
		//电池电压监测上报
		case 1006:
		case 1007:
		//开机报告
		case 1011:
		//230306:增加防拆报警
		case 1012:
		case 1013:
			if(service.serviceID==1009)
			{
				MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "smoke_state", 1, FALSE);
				// if(service.m_smokeMax<service.m_smoke_set)
				// {
					// MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "smoke_value", service.m_smoke_set+1, FALSE);
				// }
				// else
				// {
//					if(service.m_smokeMax<service.m_smoke_set)
//					{
//						service.m_smokeMax = service.m_smoke_set;
//					}
				  //MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "smoke_value", service.m_smokeMax, FALSE); 
					//service.m_smokeMax = 0;
				//}
					
					if(service.m_smoke<service.m_smoke_set)
					{
						MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "smoke_value", service.m_smoke_set, FALSE);
					}
					else
					{
						MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "smoke_value", service.m_smoke, FALSE);
					}
			}
			else if(service.serviceID==1010)
			{
					MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "smoke_state", 0, FALSE);
				// if(service.m_smoke>service.m_smoke_set)
				// {
					// MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "smoke_value", service.m_smoke_set-1, FALSE);
				// }
				// else
				//{
					//230608:烟腔故障，烟雾浓度特殊值
					if(device_info.state.F_SM_ERR)
					{
						MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "smoke_value", CODE_SMOKE_ERR, FALSE);
						device_info.isErr = 1;
					}
					else
					{
						MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "smoke_value", service.m_smoke, FALSE);
						device_info.isErr = 0;
					}

					
				//}
			}
			else 
			{
				MyJson_encode_number(p_pack_buf,/* p_pack_long,*/  "smoke_state", service.m_smoke_state, FALSE);

				//230608:烟腔故障，烟雾浓度特殊值
				if(device_info.state.F_SM_ERR)
				{
					MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "smoke_value", CODE_SMOKE_ERR, FALSE);
					device_info.isErr = 1;
				}
				else
				{
					MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "smoke_value", service.m_smoke, FALSE);
					device_info.isErr = 0;
				}
				
			}
			MyJson_encode_number(p_pack_buf,/* p_pack_long,*/  "smoke_set", service.m_smoke_set, FALSE);
			
			
			if(service.serviceID==1005)
			{
				MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "temp_state", 1, FALSE);
				
				// if(service.m_tempMax<=service.m_temp_rstset)
				// {
					// MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "temp_value", service.m_temp_rstset+1, FALSE);
				// }
				// else
				//{
					MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "temp_value", service.m_tempMax, FALSE);
					service.m_tempMax = service.m_temp;
				//}
			}
			else if(service.serviceID==1008)
			{
				MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "temp_state", 0, FALSE);
				
				// if(service.m_temp>service.m_temp_rstset)
				// {
					// MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "temp_value", service.m_temp_rstset, FALSE);
				// }
				// else
				//{
					MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "temp_value", service.m_temp, FALSE);
				//}
			}
			else
			{
				MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "temp_state", service.m_temp_state, FALSE);
				MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "temp_value", service.m_temp, FALSE);
			}	
			MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "temp_alm_set", service.m_temp_almset, FALSE);
			MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "temp_rst_set", service.m_temp_rstset, FALSE);
			
			MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "battery_state", service.m_battery_state, FALSE);
			
			//221029:格式需要转换一下0.02V单位
			if((service.serviceID==1006)&&(service.m_battery>service.m_battery_set))
			{
				service.m_battery = service.m_battery_set;
			}
			wDat = service.m_battery/20;
			service.f_battery = wDat*0.02;
			wDat = service.m_battery_set/20;
			service.f_battery_set = wDat*0.02;
			MyJson_encode_float(p_pack_buf, /* p_pack_long,*/  "battery_voltage", service.f_battery, FALSE);
			MyJson_encode_float(p_pack_buf, /* p_pack_long,*/  "battery_set", service.f_battery_set, FALSE);
			
			
			//230326：增加防拆属性
			if((service.serviceID==1012)||(service.serviceID==1012))
			{
				MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "tamper_alarm", g_cbSta.isFcOpen, FALSE);
			}
			
			
			MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "muffling", service.m_muffling, FALSE);
			MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "heartbeat_time", service.m_heartbeat_time, FALSE);
			
			MyJson_encode_string(p_pack_buf,/* p_pack_long,*/  "IMEI", g_run_info.m_imei, FALSE);
			MyJson_encode_string(p_pack_buf, /* p_pack_long,*/  "IMSI", g_run_info.m_imsi, FALSE);
			MyJson_encode_string(p_pack_buf, /* p_pack_long,*/  "ICCID", g_run_info.m_iccid, FALSE);
			MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "signalPower", g_run_info.m_csq, FALSE);
			
			//MyJson_encode_string(p_pack_buf, /* p_pack_long,*/  "software_version", CODE_VER, TRUE);
			MyJson_encode_string(p_pack_buf, /* p_pack_long,*/  "software_version", (char *)CODE_VER, FALSE);
			//MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "id", g_cbSta.bEventId, TRUE);
			MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "id", g_cbSta.bEventSndId, TRUE);		
			break;
			
		//指令下发回复
		case 9001:
		case 9020:
			MyJson_encode_number(p_pack_buf,/* p_pack_long,*/  "muffling", service.m_muffling, TRUE);
			break;
		case 9002:
			MyJson_encode_number(p_pack_buf,/* p_pack_long,*/  "smoke_set", service.m_smoke_set, TRUE);
			break;
		case 9003:
			//MyJson_encode_float(p_pack_buf, /* p_pack_long,*/  "battery_set", service.m_battery_set, TRUE);
			MyJson_encode_float(p_pack_buf, /* p_pack_long,*/  "battery_set", service.f_battery_set, TRUE);
			break;
		case 9004:
			MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "temp_alm_set", service.m_temp_almset, FALSE);
			MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "temp_rst_set", service.m_temp_rstset, TRUE);
			break;
		case 9005:
			MyJson_encode_number(p_pack_buf, /* p_pack_long,*/  "heartbeat_time", service.m_heartbeat_time, TRUE);
			break;	
		default:
			break;
	}
	MyJson_encode_end(p_pack_buf, p_pack_long);

	return TRUE;
}


u8 MyJson_parson_json(char* json_string)
{
	char* kh1 = strchr(json_string, '{');
	char* kh2 = strchr(json_string, '}');

	if ((kh1 != NULL) && (kh2 != NULL) && (kh2 > kh1))
	{
		return TRUE;
	}

	return FALSE;
}


u16 MyJson_get_number(char* json_string, const char* item/*, u16* value*/)
{
	//{"serviceId":3,"string":"hello","temp_value":4,"smoke_value":5,"battery_value":6}

	char* p = strstr(json_string, item);
	if (p != NULL)
	{
		int itemLen = strlen(item);
		//*value = atoi(&p[itemLen + 2]);
		//return TRUE;
		return atoi(&p[itemLen + 2]);
	}

	return FALSE;
}

float MyJson_get_float(char* json_string, const char* item/*, u16* value*/)
{
	//{"serviceId":3,"string":"hello","temp_value":4,"smoke_value":5,"battery_value":6}
	//u8 i;
	float temp;
	char* p = strstr(json_string, item);
	if (p != NULL)
	{
		int itemLen = strlen(item);
		u8 retIndex = 0;
		//*value = atoi(&p[itemLen + 2]);
		//return TRUE;
		//需要判断后面有没有小数点 x.xx
		temp = atoi(&p[itemLen + 2]);
		
		retIndex = itemLen+2;
		//查找到‘.’或者 ',' '}'退出
		while((p[retIndex] >='0')&&(p[retIndex]<='9'))
		{
			retIndex++;
		}
		if(p[retIndex]=='.')
		{
			#ifdef	SET_DISP
			//PrintString1("Get Point\r\n");
			#endif
			retIndex++;
			if((p[retIndex] >='0')&&(p[retIndex]<='9'))
			{
				temp += (float)(p[retIndex]-'0')/10;
			}
			retIndex++;
			if((p[retIndex] >='0')&&(p[retIndex]<='9'))
			{
				temp += (float)(p[retIndex]-'0')/100;
			}
		}
		return temp;	
	}
	return FALSE;
}

#if 0
int MyJson_get_string(char* json_string, const char* item, char* value)
{
	char* p = strstr(json_string, item);

	if (p != NULL)
	{
		int itemLen = strlen(item);
		int  retIndex = 0;
		p += 3 + itemLen; // 跳过":"

		while (p[retIndex] != '\"')
		{
			value[retIndex] = p[retIndex];
			retIndex++;
		}

		value[retIndex] = '\0';
		return TRUE;
	}

	return FALSE;

}
#endif


u8 MyJson_Unpack(char* p_pack_buf)
{
	u16 wDat;
	if (FALSE == MyJson_parson_json(p_pack_buf))
		return FALSE;
	
	#if SET_CAT1
	LOG("RCV:%s",p_pack_buf);
	#endif
	
	service.serviceID = MyJson_get_number(p_pack_buf, "serviceId"/*, &service.serviceID*/);
	
	
	service.taskId = MyJson_get_number(p_pack_buf, "taskId"/*, &service.taskId*/);
	switch(service.serviceID)
	{
		case 8001:
		case 8020:
			service.m_muffling = MyJson_get_number(p_pack_buf, "muffling"/*, (u16 *)&service.m_muffling*/);
			
			//service.m_muffling==1 模拟消音；service.m_muffling==2 暂停报警
			//暂停报警
			if(service.m_muffling==2)
			{
				if(service.m_smoke_state)
				{
					g_cbSta.bAlmSta |= BIT_SMOKE_STOP;
				}
				
				if(service.m_temp_state)
				{
					g_cbSta.bAlmSta |= BIT_TEMP_STOP;
				}
				LOG("暂停报警\r\n");
			}
			//模拟短按按键
			else if(service.m_muffling==1)
			{
				g_device_info.DeviceStatus.b.is_mute = 1;

				g_cbSta.muffTimeOut1s = 0;	
				LOG("远程消音\r\n");
			}
			break;
		case 8002:
			service.m_smoke_set = MyJson_get_number(p_pack_buf, "smoke_set"/*, (u16 *)&service.m_smoke_set*/);
			//230427:通过比对上报阈值来决定设置与否
			// if((service.m_smoke_set<=100)&&(service.m_smoke_set>=10))
			// 	TouchSend.m_flag_set = 1;
			user_fs_isNeedSave = 1;
			break;
		case 8003:
			//221029:格式需要转换一下
			service.f_battery_set = MyJson_get_float(p_pack_buf, "battery_set"/*, (u16 *)&service.m_battery_set*/);
			wDat = service.f_battery_set*50;
			service.m_battery_set = wDat*20;
			user_fs_isNeedSave = 1;
			break;
		case 8004:
			service.m_temp_almset = MyJson_get_number(p_pack_buf, "temp_alm_set"/*, (u16 *)&service.m_temp_almset*/);
			service.m_temp_rstset = MyJson_get_number(p_pack_buf, "temp_rst_set"/*, (u16 *)&service.m_temp_rstset*/);
			user_fs_isNeedSave = 1;
			break;
		case 8005:
			service.m_heartbeat_time = MyJson_get_number(p_pack_buf, "heartbeat_time"/*, (u16 *)&service.m_heartbeat_time*/);
			user_fs_isNeedSave = 1;
			break;
		default:
			#if SET_CAT1
			return TRUE;
			#endif
			break;
	}
	Event_AddEvent(service.serviceID-8000+80);
	return TRUE;

}
#endif


