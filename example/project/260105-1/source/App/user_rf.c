/*
 * @Author: your name
 * @Date: 2021-10-08 17:12:50
 * @LastEditTime: 2021-12-02 11:03:16
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \SK_ZH101\project\peripheral\ZH101\Source\user_rf.c
 */

#include "config.h"
#include "note_profile.h"

#if		SET_BLE_SUB
/**
 * @description: 用户端 接收 数据处理
 * @param {unsigned char} *rx_buf
 * @param {unsigned short} len
 * @param {unsigned char} rssi
 * @return {*}
 */
void p_user_rx_cb(unsigned char *rx_buf,unsigned short len, unsigned char rssi)
{
	#if 0
	// if(rx_buf[SITE_V] == )
	// LOG("pro 1\r\n");
	switch (rx_buf[SITE_V + 1])
	{
	case SEND_MAIN_TO_SMART://普通智能设备下行命令
		user_comm_smart_deal(rx_buf, len);
		break;
	default:
		break;
	}
	#endif
	//透传
	//if(rx_buf[SITE_V + 1]==)
	{
		
		memcpy(loraUid, &rx_buf[SITE_TAR_UID], 6);				//uid  //谁发的是谁的uid
		iSlave_Addr = rx_buf[SITE_V];			 				// 模块地址
		
		g_rf_info.is_lora_now = 0;
		if(rssi>100)
		{
			App_Rec_CMD_Process(&rx_buf[SITE_V + 1],rx_buf[SITE_L]-2,0);
		}
		else
		{
			App_Rec_CMD_Process(&rx_buf[SITE_V + 1],rx_buf[SITE_L]-2,100-rssi);
		}		
	}
}
#endif
