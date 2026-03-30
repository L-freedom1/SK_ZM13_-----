/*
 * @Author: dujianhuang
 * @Date: 2019-08-30 15:24:46
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-02-22 19:43:22
 */

/****************************************************************/
/*  公司名称：泉州时刻电子有限公司                    			*/
/*  开发人员：DJH									      		*/
/*  实现功能：部分常用程序。   			    */
/*	其他说明：。				*/
/****************************************************************/
#include "..\APP\config.h"
#if 1

static char HEX_CHAR[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

#if 0
/**
* @brief  指纹锁校验计算
* @param  
* @retval 
*/
u8 get_check_data(u8 *dataBuf, u16 len)
{
    u8 xdata iVer = 0x00;
    u8 i = 0;
    if (len)
    {
        for (i = 0; i < len; i++)
        { // 帧所有数据求和
            iVer += dataBuf[i];
        }
    }
    return iVer;
}
#endif

#if 1
/* Converts Gregorian date to seconds since 1970-01-01 00:00:00.
* Assumes input in normal date format, i.e. 1980-12-31 23:59:59
* => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
*
* [For the Julian calendar (which was used in Russia before 1917,
* Britain & colonies before 1752, anywhere else before 1582,
* and is still in use by some communities) leave out the
* -year/100+year/400 terms, and add 10.]
*
* This algorithm was first published by Gauss (I think).
*
* WARNING: this function will overflow on 2106-02-07 06:28:16 on
* machines where long is 32-bit! (However, as time_t is signed, we
* will already get problems at other places on 2038-01-19 03:14:08)
*/
unsigned long mktime_linux (u32 year0, u32 mon0,
	u32 day, u32 hour,
	u32 min, u32 sec)
{
	u32 mon = mon0, year = year0;

	/* 1..12 -> 11,12,1..10 */
	if (0 >= (int)(mon -= 2)) {
		mon += 12;	/* Puts Feb last since it has leap day */
		year -= 1;
	}

	return ((((unsigned long)
		(year / 4 - year / 100 + year / 400 + 367 * mon / 12 + day) +
		year * 365 - 719499
		) * 24 + hour /* now have hours */
		) * 60 + min /* now have minutes */
		) * 60 + sec; /* finally seconds */
}
#endif



/**
 * @description: 十六进制数组转字符串数组
 * @param {char} *hex_arr
 * @param {u16} hex_len
 * @param {char} *str
 * @return {*}
 */
void cust_hex_to_str(char *hex_arr, u16 hex_len, char *str)
{
	u16 i = 0;
	u16 str_len = 0;
	for(i = 0; i <hex_len; i++)
	{
		str[str_len++] = HEX_CHAR[((hex_arr[i] >> 4) & 0x0F)];
		str[str_len++] = HEX_CHAR[((hex_arr[i]) & 0x0F)];
	}
	str[str_len++] = '\0';
}

#endif
