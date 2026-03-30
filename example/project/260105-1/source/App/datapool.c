/*
 * @Author: dujianhuang
 * @Date: 2020-06-06 13:09:09
 * @LastEditors: error: git config user.name && git config user.email & please set dead value or install git
 * @LastEditTime: 2022-12-09 22:07:48
 */ 
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "datapool.h"
#include "config.h"


/**
 * @description:
 * @param {type} 
 * @return: 
 */
unsigned char p_my_pool_clear(st_pool *pool)
{
	pool->m_pool_idx = 0;
	return 1;
}

/**
 * @description: 入队 
 * @param {type} 
 * @return: 
 */
unsigned char p_my_pool_push(st_pool *pool, const unsigned char *data, unsigned short len)
{
	if(len>650)
	{
		return FALSE;
	}
	
	if ((pool->m_pool_idx + len + 2) >= pool->m_pool_size)
		return FALSE;
	__disable_irq();//关中断
	pool->m_pool[pool->m_pool_idx++] = len >> 8;
	pool->m_pool[pool->m_pool_idx++] = len;
	memcpy(pool->m_pool + pool->m_pool_idx , data, len);
	pool->m_pool_idx += len;
	__enable_irq();//开中断
	return TRUE;
}

/**
 * @description: 入队数据前加一个通道字节
 * @param {type} 
 * @return: 
 */
unsigned char p_my_pool_push_ch(st_pool *pool, const unsigned char *data, unsigned short len,unsigned char ch)
{
	unsigned short tlen = len + 1;

	//LOG("push:%d,%d\r\n",len,ch);
	
	if ((pool->m_pool_idx + len + 3) >= pool->m_pool_size)
		return FALSE;
	__disable_irq();//关中断
	pool->m_pool[pool->m_pool_idx++] = tlen >> 8;
	pool->m_pool[pool->m_pool_idx++] = tlen;
	pool->m_pool[pool->m_pool_idx++] = ch;
	memcpy(pool->m_pool + pool->m_pool_idx , data, len);
	pool->m_pool_idx += len;
	__enable_irq();//开中断
	return TRUE;
}

/**
 * @description: 入队数据前加一段附加内容ad
 * @param {type} 
 * @return: 
 */
unsigned char p_my_pool_push_add(st_pool *pool, const unsigned char *data, unsigned short len,unsigned char *add, unsigned char add_len)
{
	unsigned short tlen = len + add_len;
	if ((pool->m_pool_idx + tlen + 2) >= pool->m_pool_size)
		return FALSE;
	__disable_irq();//关中断
	pool->m_pool[pool->m_pool_idx++] = tlen >> 8;
	pool->m_pool[pool->m_pool_idx++] = tlen;
	memcpy(pool->m_pool + pool->m_pool_idx , add, add_len);
	pool->m_pool_idx += add_len;
	memcpy(pool->m_pool + pool->m_pool_idx , data, len);
	pool->m_pool_idx += len;
	__enable_irq();//开中断
	return TRUE;
}


/**
 * @description: 
 * @param {type} 
 * @return: 
 */
unsigned char p_my_pool_push_front(st_pool *pool, const unsigned char *data, unsigned short len)
{
	if ((pool->m_pool_idx + len + 2) >= pool->m_pool_size)
		return FALSE;
	__disable_irq();//关中断
	memmove(pool->m_pool + len + 2, pool->m_pool, pool->m_pool_idx);
	memcpy(pool->m_pool + 2, data, len);
	pool->m_pool[0] = (len >> 8) & 0xFF;
	pool->m_pool[1] = (len) & 0xFF;

	pool->m_pool_idx += (len + 2);
	__enable_irq();//开中断
	return TRUE;
}

/**
 * @description: 查队列首个数据
 * @param {type} 
 * @return: 
 */
unsigned char *p_my_pool_peek(st_pool *pool, unsigned short *len)
{
	if (pool->m_pool_idx > 0)
	{
		*len = pool->m_pool[0];
		*len = ((*len) << 8) + pool->m_pool[1];
		return pool->m_pool + 2;
	}

	return NULL;
}

/**
 * @description: 获取队列中的消息个数
 * @param {type} 
 * @return: 
 */
unsigned char p_my_pool_get_num(st_pool *pool)
{
	short all_len = 0;
	unsigned char ret_num = 0;
	unsigned short temp_len = 0;

	for(int i = 0; i < 100; i++)
	{//队列最大个数不会超过100，超过就退出
		if (all_len < pool->m_pool_idx && pool->m_pool_idx > 0)
		{
			temp_len = pool->m_pool[all_len];
			temp_len = ((temp_len) << 8) + pool->m_pool[all_len+1];
			all_len += temp_len + 2;
			ret_num++;
		}
		else
		{
			break;
		}
	}
	return ret_num;
}


/**
 * @description: 获取队列中第几个消息
 * @param no 第几个消息 1-xx
 * @return: 
 */
unsigned char *p_my_pool_get_n_msg(st_pool *pool, unsigned char no,unsigned short *len)
{
	short all_len = 0;
	unsigned char ret_num = 0;
	unsigned short temp_len = 0;

	for(int i = 0; i < 100; i++)
	{//队列最大个数不会超过100，超过就退出
		if (all_len < pool->m_pool_idx && pool->m_pool_idx > 0)
		{
			temp_len = pool->m_pool[all_len];
			temp_len = ((temp_len) << 8) + pool->m_pool[all_len+1];
			ret_num++;
			if(ret_num == no)
			{
				*len = temp_len;
				return (pool->m_pool + all_len + 2);
			}
			all_len += temp_len + 2;
		}
		else
		{
			break;
		}
	}
	return NULL;
}

/**
 * @description: 出队
 * @param {type} 
 * @return: 
 */
unsigned char p_my_pool_pop(st_pool *pool)
{
	__disable_irq();//关中断
	if (pool->m_pool_idx > 0)
	{
		u16 len = pool->m_pool[0];
		len = (len << 8) + pool->m_pool[1] + 2;

		pool->m_pool_idx -= len;
		
		memmove(pool->m_pool, pool->m_pool + len, pool->m_pool_idx);
		memset(pool->m_pool + pool->m_pool_idx, 0x00, len);
	}
	__enable_irq();//开中断
	return FALSE;
}

