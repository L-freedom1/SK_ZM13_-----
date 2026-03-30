/*
 * @Author: dujianhuang
 * @Date: 2019-08-30 15:24:46
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-02-22 19:43:52
 */
#ifndef __CUSTOM_H
#define	__CUSTOM_H

#define BCD_TO_REAL 1
#define REAL_TO_BCD 0


extern u8 BCDToU8(u8 iBCD);/* BCD转u8 */
extern u8 U8ToBCD(u8 iU8); /* l两位数U8 -> BCD */
extern u8 get_check_data(u8 *dataBuf, u16 len);//指纹锁校验计算

extern unsigned long mktime_linux(u32 year0, u32 mon0,
	u32 day, u32 hour,
	u32 min, u32 sec);

extern void cust_hex_to_str(char *hex_arr, u16 hex_len, char *str);
#endif

