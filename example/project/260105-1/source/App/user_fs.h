/*
 * @Author: your name
 * @Date: 2021-06-25 14:07:01
 * @LastEditTime: 2023-10-13 11:30:07
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Description: In User Settings Edit
 * @FilePath: \SK_ZH101\project\peripheral\ZH101\Source\user_fs.h
 */
#ifndef __USER_FS__H_
#define __USER_FS__H_

#include "flash.h"
#include "fs.h"
#include "osal_snv.h"
#include "config.h"

//be care:when assign the fs size,please refer to Flash_Distribution
//#define FS_OFFSET_ADDRESS         0x1103D000 //for 256KB Flash
#define FS_OFFSET_ADDRESS         	0x11034000 //for 512KB Flash
#define FS_SECTOR_NUM             	2

#define	MEMORY_ID					1						//地址
#define	FILE_OTA_ID					2		

#if	SET_CAT1
	extern uint8 EEPROM_Ptr[54];  
#else
	extern uint8 EEPROM_Ptr[12];   
#endif

//需要保存的参数
#define	RF_NUM_ADDR		1
#define	EVENT_ID_ADDR	2


extern u8 user_fs_isNeedSave;

extern void user_fs_init(void);
extern void user_fs_updata_memory(void);
#endif
