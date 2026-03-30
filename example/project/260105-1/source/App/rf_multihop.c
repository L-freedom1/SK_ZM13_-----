/*
 * @Author: your name
 * @Date: 2021-11-27 11:54:48
 * @LastEditTime: 2021-12-02 14:24:16
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \SK_ZH101DS_吊扇智能面板,测试多跳\project\peripheral\sk_pro\Source\rf_profile\rf_multihop.c
 */
#include <stdio.h>
#include <string.h>
#include "log.h"
#include "note_profile.h"
#include "rf_multihop.h"
#include "datapool.h"

#define MULTIHOP_DEBUG  0
#if MULTIHOP_DEBUG
#define MULTIHOP_LOG(fmt,arg...)     LOG(fmt, ##arg)
#else
#define MULTIHOP_LOG(fmt,arg...)     ;
#endif

/* 多跳缓存队列长度 */
#define MULTIHOP_POOL_SIZE  1024
unsigned char g_multihop_pool_buf[MULTIHOP_POOL_SIZE] = { 0x00 };
st_pool	g_multihop_pool = {g_multihop_pool_buf, MULTIHOP_POOL_SIZE, 0 };

/* 比较2包 结果*/
typedef enum
{
    PACK_IRRELEVANT = 0,//不相关包
    PACK_SAME,//相同包
    PACK_REVERSE,//互为反向包
}TYPE_COMPARE_PACK;

/**
 * @description: 比对两个包
 * @param {unsigned char} *rx_dat1
 * @param {unsigned short} rx_len1
 * @param {unsigned char} *rx_dat2
 * @param {unsigned short} rx_len2
 * @return {*}
 */
TYPE_COMPARE_PACK rf_mul_compare_pack(unsigned char *rx_dat1, unsigned short rx_len1,unsigned char *rx_dat2, unsigned short rx_len2)
{
    TYPE_COMPARE_PACK compara_pack = PACK_IRRELEVANT;
		//220305 判断一下重发包
    if(memcmp(&rx_dat1[0],&rx_dat2[0],REPEAT_PACK_SIZE) == 0 && rx_len1 == rx_len2 && rx_dat1[SITE_REP]==rx_dat2[SITE_REP])
    {//相同包
        compara_pack = PACK_SAME;
    }
    else if(memcmp(&rx_dat1[SITE_SRC_UID],&rx_dat2[SITE_TAR_UID],6) == 0 && memcmp(&rx_dat1[SITE_TAR_UID],&rx_dat2[SITE_SRC_UID],6) == 0)
    {
        compara_pack = PACK_REVERSE;
    }
    return compara_pack;
}


/**
 * @description: 多跳数据入队 准备转发
 * @param {unsigned char} *rx_dat
 * @param {unsigned short} rx_len
 * @param {unsigned char} delay_hop_time
 * @return {*}
 */
void rf_mul_push(unsigned char *rx_dat, unsigned short rx_len, unsigned char delay_hop_time)
{
    TYPE_POOL_HEAD head;
    unsigned char temp_buf[256] = {0};
    if(rx_len > 256)
    {
        return;
    }
    memcpy(temp_buf, rx_dat, rx_len);
    /* 多跳次数需要累加 */
    temp_buf[SITE_MULTIHOP]++;
    /* 校验需要重新计算 */
    temp_buf[rx_len - 1] = profile_check(temp_buf, rx_len - 1, profile_is_up(rx_dat[SITE_NUM]) ? PROFILE_UP : PROFILE_DOWN);

    head.info.is_valid = 1;//数据有效
    head.info.countdown = delay_hop_time;//准备转发倒计时
    head.info.data_hold_timer = DEF_DATA_HOLD_TIME;//数据保持时间

    p_my_pool_push_add(&g_multihop_pool, temp_buf, rx_len, head.buf, MULTIHOP_HEAD_SIZE);//进入待转发缓存队列

    MULTIHOP_LOG("待转发包入队\r\n");

}

/**
 * @description: 多跳接收数据处理
 * @param {unsigned char} *rx_dat
 * @param {unsigned short} rx_len
 * @param {UP_DOWN} up_down 上下行
 * @return {*}
 */
void rf_mul_rx_dat(unsigned char *rx_dat, unsigned short rx_len, UP_DOWN up_down)
{
#if MULTIHOP_EN
    unsigned char *buf = NULL;
    unsigned short temp_len = 0;
    TYPE_POOL_HEAD *head;
    TYPE_COMPARE_PACK compara_pack = PACK_IRRELEVANT;//此包 与 缓存包的关系
    unsigned char msg_num = p_my_pool_get_num(&g_multihop_pool);
    if(memcmp(&rx_dat[SITE_TAR_UID], g_note.device_uid, 6) == 0 || memcmp(&rx_dat[SITE_SRC_UID], g_note.device_uid, 6) == 0 || g_note.is_scan_status)
    {//如果是发给自己的 或者 当前是扫描状态 直接退出
        MULTIHOP_LOG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>发给自己或者自己发出的 不转发\r\n");
        return;
    }
    if(rx_dat[SITE_LOW_POWER])
    {//低功耗设备数据暂时也不做处理
        return;
    }
    if (msg_num)
    {
        for (int i = 0; i < msg_num; i++)
        {
            if (((buf = p_my_pool_get_n_msg(&g_multihop_pool, i + 1, &temp_len)) != NULL))
            { //获取第几个队列内容
                // memcpy(head.buf, buf, MULTIHOP_HEAD_SIZE);
                head = (TYPE_POOL_HEAD*)buf;
                if (rf_mul_compare_pack(&buf[DAYA_START], temp_len - DAYA_START, rx_dat, rx_len) == PACK_SAME)
                { //如果是相同数据 直接过滤掉
                    compara_pack = PACK_SAME;
                    MULTIHOP_LOG(">>>>>>>>相同包不需要转发\r\n");
                    break;
                }
                if (head->info.is_valid)
                { //数据有效
                    if ((rf_mul_compare_pack(&buf[DAYA_START], temp_len - DAYA_START, rx_dat, rx_len) == PACK_REVERSE  && profile_is_ack(rx_dat[SITE_NUM])))
                    { //如果是反向包 并且是回复包 则队列缓存包无效 不再转发
                        head->info.is_valid = 0;
                        compara_pack = PACK_REVERSE;
                        MULTIHOP_LOG(">>>>>>>>>收到回复关闭转发\r\n");                                    
                        if(multihop_limit(rx_dat[SITE_MULTIHOP])>multihop_cur_hop(rx_dat[SITE_MULTIHOP]))
                        {//如果当前跳的次数小于多跳次数
                           	rf_mul_push(rx_dat, rx_len,0); //回复包直接跳，不延时， 入队
                        }
                        break;
                    }
                }
            }
        }
        if (compara_pack == PACK_IRRELEVANT)
        { //如果该包为不相关包 则也入队 准备转发
            if(multihop_limit(rx_dat[SITE_MULTIHOP]) > multihop_cur_hop(rx_dat[SITE_MULTIHOP]))
            {                                //如果当前跳的次数小于多跳次数
							rf_mul_push(rx_dat, rx_len, profile_is_ack(rx_dat[SITE_NUM])? 0 : DELAY_HOP_TIME); //入队准备转发   
            }
        }
    }
    else
    {
        if(multihop_limit(rx_dat[SITE_MULTIHOP]) > multihop_cur_hop(rx_dat[SITE_MULTIHOP]))
        {//如果当前跳的次数小于多跳次数
						rf_mul_push(rx_dat, rx_len, profile_is_ack(rx_dat[SITE_NUM])? 0 : DELAY_HOP_TIME);//入队准备转发
        }
    }
#endif
}

/**
 * @description: 多跳数据扫描处理 10ms 跑一次
 * @param {*}
 * @return {*}
 */
void rf_multihop_scan(void)
{
#if MULTIHOP_EN
    unsigned char *buf = NULL;
    unsigned short temp_len = 0;
    TYPE_POOL_HEAD *head;
    unsigned char msg_num = p_my_pool_get_num(&g_multihop_pool);
    static unsigned char _10ms_count = 0;
    static unsigned char _100ms_count = 0;
    unsigned char temp = 0;
	if(msg_num)
	{
		// printf("网路重发队列剩余数:%d, 队列长度:%d\r\n",msg_num,g_multihop_pool.m_pool_idx);
		for (int i = 0; i < msg_num; i++)
		{
			if(((buf = p_my_pool_get_n_msg(&g_multihop_pool,i+1, &temp_len)) != NULL))
			{//获取第几个队列内容
				head = (TYPE_POOL_HEAD*)buf;
                if(head->info.is_valid)
				{//需要转发
					if(head->info.countdown) head->info.countdown--;
					if(head->info.countdown == 0)
					{//转发计时到
								if(g_note.is_tx_busy == 0)
								{//如果当前发送不繁忙则准备发送
										MULTIHOP_LOG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>转发队列, 剩余个数:%d\r\n",msg_num);
										rf_tx(&buf[DAYA_START], temp_len - DAYA_START,0);
										head->info.is_valid = 0;
										break;
								}
					}
				}
			}
		}
        temp = msg_num;
		for (int i = 0; i < msg_num; i++)
		{
			if (((buf = p_my_pool_peek(&g_multihop_pool, &temp_len)) != NULL))
			{
                head = (TYPE_POOL_HEAD*)buf;
				if(head->info.data_hold_timer == 0)
				{//保持时间到的数据出对
                    temp--;
					p_my_pool_pop(&g_multihop_pool); //出队
                    MULTIHOP_LOG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>数据出队, 剩余个数:%d\r\n",temp);
                }
				else
				{
					break;
				}
				
			}
		}
	}

    if (++_10ms_count >= 10)
    { //100ms
        _10ms_count = 0;
        if (++_100ms_count >= 10)
        {//1s计时
            _100ms_count = 0;
            msg_num = p_my_pool_get_num(&g_multihop_pool);
            for (int i = 0; i < msg_num; i++)
            {
                if (((buf = p_my_pool_get_n_msg(&g_multihop_pool, i + 1, &temp_len)) != NULL))
                { //获取第几个队列内容
                    head = (TYPE_POOL_HEAD *)buf;
                    if (head->info.data_hold_timer)
                    {//保持时间计时
                        head->info.data_hold_timer--;
                    }
                }
            }
        }
    }
#endif
}

