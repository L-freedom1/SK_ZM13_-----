/*
 * @Author: your name
 * @Date: 2021-06-25 14:06:56
 * @LastEditTime: 2024-11-07 17:13:40
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \SK_ZH101\project\peripheral\ZH101\Source\user_fs.c
 */

#include "user_fs.h"

#if SET_CAT1
uint8 EEPROM_Ptr[54];
#else
uint8 EEPROM_Ptr[12];
#endif
u8 user_fs_isNeedSave;


void p_DefaultSet(void)
{
	memset(&EEPROM_Ptr,0,sizeof(EEPROM_Ptr));
	#if SET_NB
		#if NB_MODULE==NB_N701
		EEPROM_Ptr[0] = DEVICE_ID+4;
		#else
		EEPROM_Ptr[0] = DEVICE_ID+1;
		#endif
	#elif SET_CAT1
	EEPROM_Ptr[0] = DEVICE_ID+3;
	#else
		#if SET_SMART
		EEPROM_Ptr[0] = DEVICE_ID+2;
		#else
		EEPROM_Ptr[0] = DEVICE_ID;
		#endif
	#endif
	EEPROM_Ptr[RF_NUM_ADDR] = 0;
	//EEPROM_Ptr[EVENT_ID_ADDR] = 0;
	//初始化参数
	//230427:阈值以我为准
	//service.m_smoke_set = 48;
	service.m_smoke_set = DEFAULT_SMOKE_SET;

	#if SET_NB||SET_CAT1
		#if NB_MODULE==NB_N701
		service.m_battery_set = 2750;
		#else
		service.m_battery_set = 2500;
		#endif
	#else
		service.m_battery_set = 2500;
	#endif

	service.m_temp_almset = 55;
	service.m_temp_rstset = 50;

	// service.m_temp_almset = 20;
	// service.m_temp_rstset = 15;

	service.m_temp_lowset = 0xff;
	service.m_temp_lowrstset = 0xff;	
	#if SET_NB||SET_CAT1
	service.m_heartbeat_time = 24;
	#else
	service.m_heartbeat_time = 7;
	#endif
	service.m_proto_set = PROTO_SHIKE_PLUS;	
		
	#if SET_CAT1
	//101.200.222.252  2159
	// service.ip[0] = 101;
	// service.ip[1] = 200;
	// service.ip[2] = 222;
	// service.ip[3] = 252;
	// service.ip[4] =  2159>>8;
	// service.ip[5] =  2159&0xff;

//	service.port = 2159;
//	memcpy(service.Ip_buf,"101.200.222.252",sizeof(service.Ip_buf));
	
	service.port = 6000;
	memcpy(service.Ip_buf,"121.41.87.203",sizeof(service.Ip_buf));
	#endif

	user_fs_updata_memory();
}

u8 pIsParaOk(void)
{
	#if SET_CAT1
	memcpy(&service.m_battery_set,&EEPROM_Ptr[2],52);
	#else
	memcpy(&service.m_battery_set,&EEPROM_Ptr[2],10);
	#endif

	//检查设备型号
	#if SET_NB
		#if NB_MODULE==NB_N701
		if(EEPROM_Ptr[0]!=DEVICE_ID+4)
		#else
		if(EEPROM_Ptr[0]!=DEVICE_ID+1)
		#endif
	#elif SET_CAT1
	if(EEPROM_Ptr[0]!=DEVICE_ID+3)
	#else
		#if SET_SMART
		if(EEPROM_Ptr[0]!=DEVICE_ID+2)
		#else
		if(EEPROM_Ptr[0]!=DEVICE_ID)
		#endif
	#endif
	{
		return 0;
	}


	//检查协议版本
	if((service.m_proto_set!=PROTO_SHIKE)&&(service.m_proto_set!=PROTO_ZH202)&&(service.m_proto_set!=PROTO_SHIKE_PLUS))
	{
		return 0;
	}

	//检查烟雾浓度阈值
	if((service.m_smoke_set<10)||(service.m_smoke_set>100))
	{
		return 0;
	}

	return 1;
	
}
/**
 * @description: 
 * @param {*}
 * @return {*}
 */
void user_fs_init(void)
{
    // u16 t_len = 0;
    if(hal_fs_initialized() == FALSE)
    {
        if (PPlus_SUCCESS != hal_fs_init(FS_OFFSET_ADDRESS, FS_SECTOR_NUM))
        {
            LOG("init fs error\r\n");
        }
    }
	osal_snv_read(MEMORY_ID, sizeof(EEPROM_Ptr), EEPROM_Ptr);

	if(pIsParaOk()==0)
	{
		p_DefaultSet();
	}
	
	
	RF_Num = EEPROM_Ptr[RF_NUM_ADDR];
	#if SET_BLE_SUB
	g_note.tx_num = EEPROM_Ptr[RF_NUM_ADDR];
	#endif
	//g_cbSta.bEventId = EEPROM_Ptr[EVENT_ID_ADDR];
	g_cbSta.bEventId = service.m_bak1;
	LOG("配置参数：");	
	#if SET_CAT1
	my_dump_byte(&EEPROM_Ptr[2],52);
	#else
	my_dump_byte(&EEPROM_Ptr[2],10);
	#endif
	LOG("\r\n");
}

/**
 * @description: 
 * @param {*}
 * @return {*}
 */
void user_fs_updata_memory(void)
{
	EEPROM_Ptr[0] = DEVICE_ID;
	EEPROM_Ptr[RF_NUM_ADDR] = RF_Num;
	//EEPROM_Ptr[EVENT_ID_ADDR] = g_cbSta.bEventId;
	service.m_bak1 = g_cbSta.bEventId;
	#if SET_CAT1
	memcpy(&EEPROM_Ptr[2],&service.m_battery_set,52);
	#else
	memcpy(&EEPROM_Ptr[2],&service.m_battery_set,10);
	#endif

    osal_snv_write(MEMORY_ID, sizeof(EEPROM_Ptr), EEPROM_Ptr);
}

