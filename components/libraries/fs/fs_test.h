/**************************************************************************************************
*******
**************************************************************************************************/


#ifndef __FS_TEST_H__
#define __FS_TEST_H__

//be care:when assign the fs size,please refer to Flash_Distribution
#define FS_OFFSET_ADDRESS         0x1103c000 //for 256KB Flash
//#define FS_OFFSET_ADDRESS         0x11034000 //for 512KB Flash
#define FS_SECTOR_NUM			  2

#define FS_EXAMPLE		 0x01
#define FS_XIP_TEST      0x02
#define FS_MODULE_TEST   0x04
#define FS_TIMING_TEST   0x08
#define	FS_APP		 	 0x10

//#define FS_TEST_TYPE     FS_EXAMPLE
#define FS_TEST_TYPE     FS_APP

#if (FS_TEST_TYPE == FS_APP)
	#include "config.h"
	typedef unsigned char BYTE;
	typedef unsigned int WORD;

	typedef union
	{
		u8 u8_byte[4];
		u32 u32_byte;
	}TYPE_CHANGE;

	#define ERROR   0
	#define OK      1

	#define MODE_WRITE  0
	#define MODE_READ   1

	//地址
	#define MAIN_DEVICE_ADDR		                  	0x0000		// 1台主机地址每个地址6字节 
	#define MAIN_DEVICE_NO_ADDR		                    0x0006		// 设备在主机上的编号

	#define	DEVICE_HIGHVOL_ADDR							0x0007
	#define	DEVICE_LOWVOL_ADDR							0x000C
	#define	DEVICE_HIGHPOW_ADDR							0x0011
	#define	DEVICE_HIGHTEMP_ADDR						0x0016
	#define	DEVICE_MODE											0x0019
	#define DEVICE_ID_ADDR		                        0x001C			// 是否为设备ID，如果不是则恢复出厂设置
	#define DEVICE_ID_KEY                               0x001D

	#define BIT_DEVICE_ALM  0x08

	// 最大数量定义
	#define DEVICE_STATUS_START_ADDR    				    0x200
	#define DEVICE_STATUS_END_ADDR    						0x400

	#define DEVICE_KEY_START_ADDR			                0x01D
	#define DEVICE_KEY_END_ADDR                             0x200

	//#define MAX_MAP_SIZE    (DEVICE_KEY_START_ADDR+1)
	#define MAX_MAP_SIZE	0x20
	extern uint8 EEPROM_Ptr[MAX_MAP_SIZE];//映射前48字节eeprom地址，（保存主机地址）

//文件ID,长度 （缓冲暂不需要）
	#define	FILE_POWER_ID	1
	#define	FILE_POWER_LEN	4

	#define	FILE_KEY_ID		2
	#define	FILE_KEY_LEN	1

	#define	FILE_PARA_ID	3
	#define	FILE_PARA_LEN	MAX_MAP_SIZE

	extern u8  g_bCpuId[7];
	extern u8 eeprom_save_all_power(u8 mode);
	extern u8 eeprom_save_all_key(u8 mode);
	extern u8 eeprom_save_all_para(u8 mode);
	extern void pGetCpuId(void);

#elif (FS_TEST_TYPE == FS_EXAMPLE)
	void fs_example(void);	
#elif (FS_TEST_TYPE == FS_XIP_TEST)
	void fs_xip_test(void);
#elif (FS_TEST_TYPE == FS_MODULE_TEST)
	void ftcase_simple_write_test(void);
	void ftcase_write_del_test(void);
	void ftcase_write_del_and_ble_enable_test(void);
#elif (FS_TEST_TYPE == FS_TIMING_TEST)
	void fs_timing_test(void);
#else
	#error please check your config parameter
#endif


#endif
