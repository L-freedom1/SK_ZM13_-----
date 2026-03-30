/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2024-04-15 10:44:25
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-04-18 16:14:15
 * @FilePath: \source\NB\json.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __JSON_H_
#define __JSON_H_

#include "config.h"

#if SET_PROTO_VER==PROTO_VER_JSON

//#define 	BUFFSIZE 		410
#define 	BUFFSIZE 		450
extern char  json_buf[BUFFSIZE];

#if 0
typedef struct    
{
	/* 服务类型 */
	u16 serviceID;
	
	u16 taskId;
	
	/* 属性 */
	u8 m_smoke_state;
	u8 m_smoke;
	u8 m_smoke_set;
	u8 m_smokeMax;
	
	u8 m_temp_state;
	u8 m_temp;
	u8 m_temp_almset;
	u8 m_temp_rstset;
	u8 m_tempMax;
	
	u8 m_battery_state;	
	float m_battery;
	float m_battery_set;
	
	u8 m_muffling;
	u8 m_heartbeat_time;
}service_t;

extern service_t  service;
#endif


extern u8 MyJson_pack(char* p_pack_buf, int p_pack_long,u16 taskId);
extern u8 MyJson_Unpack(char* p_pack_buf);
#endif

#endif
