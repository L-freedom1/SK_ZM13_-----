/*
 * @Author: dujianhuang
 * @Date: 2020-05-09 10:10:24
 * @LastEditors: error: git config user.name && git config user.email & please set dead value or install git
 * @LastEditTime: 2022-12-22 00:29:37
 */
#ifndef __DATA_POOL_H
#define __DATA_POOL_H

/*结构定义*/

#define	NET_TRAN_SIZE	660

/* 字节队列 */
typedef struct
{
	unsigned char *iRec_Buf;
  	unsigned short iRec_In;
	unsigned short iRec_Out;
	unsigned char iRec_Flag;
	// unsigned char iRec_Buf_Temp[80];		// 单帧串口数据缓存
	unsigned short len;
}_BYTE_QUEUE_Data_;	

/* 消息队列 */
typedef struct  
{
	unsigned char		*m_pool;
	const unsigned int   m_pool_size;
	unsigned int		 m_pool_idx;
}st_pool;

unsigned char p_my_pool_clear(st_pool *pool);
unsigned char p_my_pool_push(st_pool *pool, const unsigned char *data, unsigned short len);
unsigned char p_my_pool_push_ch(st_pool *pool, const unsigned char *data, unsigned short len,unsigned char ch);
unsigned char p_my_pool_push_front(st_pool *pool, const unsigned char *data, unsigned short len);
unsigned char *p_my_pool_peek(st_pool *pool, unsigned short *len);
unsigned char p_my_pool_pop(st_pool *pool);
unsigned char p_my_pool_push_add(st_pool *pool, const unsigned char *data, unsigned short len,unsigned char *add, unsigned char add_len);
unsigned char p_my_pool_get_num(st_pool *pool);
unsigned char *p_my_pool_get_n_msg(st_pool *pool, unsigned char no,unsigned short *len);
#endif
