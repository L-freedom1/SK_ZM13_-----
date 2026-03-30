/*
 * @Author: your name
 * @Date: 2021-10-07 16:36:53
 * @LastEditTime: 2026-03-30 10:21:07
 * @LastEditors: leodjh1 33412386+leodjh1@users.noreply.github.com
 * @Description: 2.4G 设备接入协议，节点协议部分
 * @FilePath: \SK_ZH101\project\peripheral\ZH101\Source\note_profile.h
 */

#include "note_profile.h"
//#include "rf_multihop.h"
#include <stdio.h>
#include <string.h>
#include "log.h"
#include "clock.h"
#include "config.h"
#include "phy_rf.h"
#include "OSAL.h"


#if	SET_BLE_SUB
	
/* 测试 2.4G 丢包率， 如果不是测试模式 两个宏均设置为0即可 */
#define TEST_TX 0//发射端模式
#define TEST_RX 0//应答端模式

/* 测试 2.4G 丢包率 */
#if (TEST_TX || TEST_RX)


#define SEND_TIMEOUT    50//超时发射时间

unsigned char rf_tx(unsigned char *tx_dat, unsigned short len, unsigned short delay_time);
unsigned char profile_check(unsigned char  *dat_buf, unsigned short len, UP_DOWN up_down);

typedef struct
{
    u8 is_rx_ok;
    u16 tx_num;
    u16 rx_num;

    u16 is_rx_cur_pack_num;//是否接收到当前包序号了
    u16 pre_send_timer;

    u16 mulhop_count[16];//多跳统计 0 - 8 跳
}TEST_LOSE_PACK;

TEST_LOSE_PACK g_test_lose_pack;



/**
 * @description: 测试丢包率发射测试包
 * @param {unsigned} pack_num 包序
 * @return {*}
 */
void test_lose_pack_send_pack(unsigned pack_num)
{
    unsigned char tx_buf[50];
    unsigned char len = 0;
    unsigned char tx_num = (pack_num)%64;

#if (TEST_TX)
    profile_set_up(tx_num);//设置上行包
    len++;
    tx_buf[len++] = tx_num;
    memcpy(&tx_buf[len],g_note.device_uid,6);//发送方uid
    len += 6;

    // memset(&tx_buf[len],0xAA,6);//
    // len += 6;
    for(int i = 0; i < 6; i++)
    {
        tx_buf[len++] = g_note.device_uid[i] + 1;
    }
#else
    profile_set_ack(tx_num);//设置回复包
    len++;
    tx_buf[len++] = tx_num;
    // memset(&tx_buf[len],0xAA,6);//
    // len += 6;

    for(int i = 0; i < 6; i++)
    {
        tx_buf[len++] = g_note.device_uid[i] + 1;
    }

    memcpy(&tx_buf[len],g_note.device_uid,6);//发送方uid
    len += 6;
#endif

    tx_buf[len++] = LOW_POWER_FLGA;//低功耗描述符
    tx_buf[len++] = MULTIHOP_FLGA;//预留2字节
    tx_buf[len++] = 0x00;

    tx_buf[len++] = 0xFE;//测试命令
    tx_buf[len++] = 4;//没有数据长度0
    tx_buf[len++] = g_test_lose_pack.rx_num >> 8;//接收到和发送出去的个数
    tx_buf[len++] = g_test_lose_pack.rx_num & 0xFF;//接收到和发送出去的个数
    tx_buf[len++] = g_test_lose_pack.tx_num >> 8;//接收到和发送出去的个数
    tx_buf[len++] = g_test_lose_pack.tx_num & 0xFF;//接收到和发送出去的个数

    len++;//空一下校验字段
    tx_buf[SITE_ALL_LEN] = len - 1;
    tx_buf[len - 1] = profile_check(tx_buf, len - 1, PROFILE_UP);
    
#if (TEST_RX)
    rf_tx(tx_buf, len, 10);//发送
#else
    rf_tx(tx_buf, len, 0);//发送
#endif
     g_test_lose_pack.is_rx_cur_pack_num = 1;
}


/**
 * @description: 测试丢包率 数据接受处理
 * @param {*}
 * @return {*}
 */
void test_lose_pack_rx(unsigned char *rx_dat, unsigned short rx_len, unsigned char rssi)
{
    if((rx_dat[rx_len - 1] != profile_check(rx_dat, rx_len - 1, PROFILE_DOWN) && rx_dat[rx_len - 1] != profile_check(rx_dat, rx_len - 1, PROFILE_UP)) || rx_len < PROFILE_MIN_LEN)
    {//校验错误 或者 数据小于最小协议字节 处理 
        // LOG("rf check error\r\n");
        return;
    }
    LOG("[PHY RX] [-%03ddbm len-%d ] ", rssi, rx_len);
    LOG_DUMP_BYTE(rx_dat, rx_len);

#if (TEST_TX)
    if(memcmp(&rx_dat[SITE_TAR_UID],&g_note.device_uid[0],6) == 0)
    {
        // WaitMs(10);

        if(g_test_lose_pack.is_rx_cur_pack_num && (g_note.tx_num%64) == (rx_dat[SITE_NUM] & 0x3F))
        {
            g_test_lose_pack.rx_num++;
            g_test_lose_pack.is_rx_cur_pack_num = 0;

            g_test_lose_pack.mulhop_count[(rx_dat[SITE_MULTIHOP] & 0x0F)]++;
            // test_lose_pack_send_pack((++g_note.tx_num) % 64);
            // g_test_lose_pack.tx_num++;
            // g_test_lose_pack.pre_send_timer = SEND_TIMEOUT;
        }
        
    }
#endif

#if (TEST_RX)
    if(rx_dat[SITE_TAR_UID] == g_note.device_uid[0] + 1 && rx_dat[SITE_TAR_UID + 1] == g_note.device_uid[1] + 1 && rx_dat[SITE_TAR_UID+2] == g_note.device_uid[2] + 1 \
    && rx_dat[SITE_TAR_UID+3] == g_note.device_uid[3] + 1 && rx_dat[SITE_TAR_UID+4] == g_note.device_uid[4] + 1  && rx_dat[SITE_TAR_UID+5] == g_note.device_uid[5] + 1)
    {
        test_lose_pack_send_pack(rx_dat[SITE_NUM] & 0x3F);
        if(g_test_lose_pack.is_rx_cur_pack_num != (rx_dat[SITE_NUM] & 0x3F) || g_test_lose_pack.is_rx_cur_pack_num == 0)
        {
            g_test_lose_pack.is_rx_cur_pack_num = (rx_dat[SITE_NUM] & 0x3F);
            g_test_lose_pack.rx_num++;
            g_test_lose_pack.mulhop_count[(rx_dat[SITE_MULTIHOP] & 0x0F)]++;
        }

    }
#endif

}

/**
 * @description: 测试丢包率 10ms 跑一次
 * @param {*}
 * @return {*}
 */
void test_lose_pack_per(void)
{
    static unsigned int _10ms_count = 0;
#if (TEST_TX)
    if(g_test_lose_pack.pre_send_timer)
    {
        g_test_lose_pack.pre_send_timer--;
    }
    if (g_test_lose_pack.pre_send_timer == 0)
    {
        g_test_lose_pack.pre_send_timer = SEND_TIMEOUT;
        test_lose_pack_send_pack((++g_note.tx_num)%64);
        g_test_lose_pack.tx_num++;
    }
#endif
    if(++_10ms_count >= 100)
    {//1s
        _10ms_count = 0;
#if (TEST_TX)
        LOG("######################### rx/tx num: %d/%d\r\n",g_test_lose_pack.rx_num,g_test_lose_pack.tx_num);
        for(int i = 0; i < 9; i++)
        {
            LOG("%d hop: %d\r\n",i, g_test_lose_pack.mulhop_count[i]);
        }
#endif

#if (TEST_RX)
        LOG("######################### rx/tx num: %d/%d\r\n", g_test_lose_pack.rx_num, g_test_lose_pack.tx_num);
        for (int i = 0; i < 9; i++)
        {
            LOG("%d hop: %d\r\n", i, g_test_lose_pack.mulhop_count[i]);
        }
#endif
    }
}

#endif


unsigned char all_00_buf[20];
unsigned char all_ff_buf[20];

/* 节点设备 */
TYPE_NOTE_PROFILE g_note;
TYPE_RF_SEND g_rf_send;
TYPE_RF_SEND_BAK g_rf_ackbak;

/**
 * @description: 获取 随机数
 * @param {*}
 * @return {*}
 */
unsigned int get_random(void)
{
    int rand = hal_systick();
    rand = rand + g_note.device_uid[0] + g_note.device_uid[1] + g_note.device_uid[2] + g_note.device_uid[3] + g_note.device_uid[4] + g_note.device_uid[5];
    return rand;
}

/**
 * @description: 当前发送数据
 * @param {unsigned char} *dat_buf
 * @param {unsigned short} len
 * @return {*}
 */
unsigned char rf_tx(unsigned char *tx_dat, unsigned short len, unsigned short delay_time)
{
    if(g_system_ota)
    {
        return 1;
    }

    #if SET_ALARM_VER
    if(g_device_info.RunTime>=180)
        return 1;
    #endif
   
		
    LOG("rf tx,%d: ",len);
    LOG_DUMP_BYTE(tx_dat, len);
    phy_tx_buf_updata(tx_dat, len);
    g_note.is_tx_busy = 1;
    if(delay_time == 0)
    {
        osal_set_event(PhyRf_TaskID,PPP_PERIODIC_TX_EVT);//发送数据
    }
    else
    {
        osal_start_timerEx(PhyRf_TaskID, PPP_PERIODIC_TX_EVT, delay_time); //发送数据
    }
    return 1;
}


/**
 * @description: 协议计算校验
 * @param {unsigned char } *dat_buf
 * @param {unsigned short} len
 * @return {*}
 */
unsigned char profile_check(unsigned char  *dat_buf, unsigned short len, UP_DOWN up_down)
{
    unsigned char check = 'S' + 'K';
    
    if(up_down == PROFILE_UP)
    {//如果是上行
        check = 's' + 'k';
    }
    for (unsigned short i = 0; i < len; i++)
    {
        check += dat_buf[i];
    }

    check = ~check;
    
    return check;
}

/**
 * @description: 节点设备初始化
 * @param {unsigned char} *device_uid 本机uid
 * @param (void)p_rx_cb 接受数据回调
 * @return {*}
 */
void note_init(unsigned char *device_uid, void(*p_rx_cb)(unsigned char *,unsigned short, unsigned char))
{
    memset(&g_note, 0x00, sizeof(TYPE_NOTE_PROFILE));
    memcpy(g_note.device_uid, device_uid, 6);
    /* g_note.reg_status = 0;
    g_note.tx_num = 0; */

    g_note.tx_num = RF_Num;         //保存包序防止开机报告丢失

    memset(all_00_buf, 0x00,sizeof(all_00_buf));
    memset(all_ff_buf, 0xff,sizeof(all_ff_buf));

    memset(&g_rf_send,0x00,sizeof(TYPE_RF_SEND));
    g_note.p_rx_cb = p_rx_cb;
}

/**
 * @description: 发送扫描指令
 * @param {*}
 * @return {*}
 */
void note_scan_gateway(void)
{
    unsigned char tx_buf[50];
    unsigned char len = 0;
    unsigned char tx_num = (++g_note.tx_num)%64;

    profile_set_up(tx_num);//设置上行包
    len++;
    tx_buf[len++] = tx_num;
    memcpy(&tx_buf[len],g_note.device_uid,6);//发送方uid
    len += 6;
    
    memcpy(&tx_buf[len],all_ff_buf,6);//广播
    len += 6;

    tx_buf[len++] = LOW_POWER_FLGA;//低功耗描述符
    #if MULTIHOP_EN_TX
    tx_buf[len++] = MULTIHOP_FLGA;//预留2字节
    #else
    tx_buf[len++] = 0;//预留2字节
    #endif
    tx_buf[len++] = 0x00;

    tx_buf[len++] = T_SCAN_GATEWAY;//扫描命令
    tx_buf[len++] = 0;//没有数据长度0
    len++;//空一下校验字段
    tx_buf[SITE_ALL_LEN] = len - 1;
    tx_buf[len - 1] = profile_check(tx_buf, len - 1, PROFILE_UP);
    
//    if(g_DeviceStatus==DEVICE_STA_TEST_TOUCH)
//    {
//        note_set_send(tx_buf, len, 10, 300);//发送
//    }
//    else
    {
        note_set_send(tx_buf, len, 5, 300);//发送
    }
     
}


/**
 * @description: 开始重新扫描注网
 * @param {*}
 * @return {*}
 */
void note_start_scan_reg(void)
{
    g_note.is_scan_status = 1;
    g_note.reg_timer = 10;//注册倒计时
	
    memset(&g_note.scan_result, 0x00, sizeof(TYPE_SCAN_RESULT) * MAX_SCAN_NUM);
    g_note.cur_scan_num = 0;

    note_scan_gateway();//发送数据

    LOG("restart reg\r\n");
}

/**
 * @description: 注网
 * @param {*}
 * @return {*}
 */
void note_reg_net(unsigned char *gateway_uid)
{
    unsigned char tx_buf[50];
    unsigned char len = 0;
    unsigned char tx_num = (++g_note.tx_num)%64;

    profile_set_up(tx_num);//设置上行包
    len++;
    tx_buf[len++] = tx_num;
    memcpy(&tx_buf[len],g_note.device_uid,6);//发送方uid
    len += 6;
    
    memcpy(&tx_buf[len],gateway_uid,6);//注册网关uid
    len += 6;

    tx_buf[len++] = LOW_POWER_FLGA;//低功耗描述符
    #if MULTIHOP_EN_TX
    tx_buf[len++] = MULTIHOP_FLGA;//预留2字节
    #else
    tx_buf[len++] = 0;//预留2字节
    #endif
    tx_buf[len++] = 0x00;

    tx_buf[len++] = T_REG_NET;//注册命令
    tx_buf[len++] = 0;//没有数据长度0
    len++;//空一下校验字段
    tx_buf[SITE_ALL_LEN] = len - 1;
    tx_buf[len - 1] = profile_check(tx_buf, len - 1, PROFILE_UP);
    
//    if(g_DeviceStatus==DEVICE_STA_TEST_TOUCH)
//    {
//        LOG("TEST_RUN\r\n");
//        g_DeviceStatus = DEVICE_STA_TEST_RUN;
//        g_rf_send.send_test_ok = 0;
//        note_set_send(tx_buf, len, 100, COMM_RESEND_TIME);//发送
//    }
//    else
    {
        note_set_send(tx_buf, len, 3, COMM_RESEND_TIME);//发送
    }
    
}

#if USE_COM_TRAN
//to modification 移植需要修改
/**
 * @description: 通用透传命令上报 (之前的LoRa协议 普通设备上报), 用户需要自己修改填写 模块地址，状态相关
 * @param {unsigned char} cmd1 
 * @param {unsigned char} cmd2
 * @param {unsigned char} times 重发次数
 * @param {unsigned short} resend_interval 重发间隔
 * @param {unsigned char} is_ack 是否应答包
 * @return {*}
 */
void note_comm_tran_up(unsigned char cmd1, unsigned char cmd2, unsigned char times, unsigned short resend_interval,unsigned char is_ack)
{
    unsigned char tx_buf[50];
    unsigned char len = 0;
    unsigned char tx_num = (++g_note.tx_num)%64;
    unsigned char v_temp_len = 0;//V字段数据长度

    profile_set_up(tx_num);//设置上行包
    if(is_ack)
    {//如果是应答包
        profile_set_ack(tx_num);
    }
    len++;//空余一下长度字段
    tx_buf[len++] = tx_num;
    memcpy(&tx_buf[len],g_note.device_uid,6);//发送方uid
    len += 6;
    
    memcpy(&tx_buf[len],g_note.gateway_uid,6);//注册网关uid
    len += 6;

    tx_buf[len++] = LOW_POWER_FLGA;//低功耗描述符
    tx_buf[len++] = 0x00;//预留2字节
    tx_buf[len++] = 0x00;

    tx_buf[len++] = T_TRAN;//命令
    tx_buf[len++] = 0;//没有数据长度0

    /* V字段 */
    v_temp_len = len;
    tx_buf[len++] = 0xfa;

    /*数据*/
    tx_buf[len++] = cmd1;			   //命令编码
    //数据
    // tx_buf[len++] = g_device_info.type_switch; //设备类型
    // tx_buf[len++] = cmd2;
    // tx_buf[len++] = g_device_info.light_status;
    memcpy(&tx_buf[len],&SendBuf[SITE_DATA_STAT],SendBuf[SITE_LEN]);
    len += SendBuf[SITE_LEN];

    tx_buf[SITE_L] = len - v_temp_len;
    len++;//空一下校验字段
    tx_buf[SITE_ALL_LEN] = len - 1;
    tx_buf[len - 1] = profile_check(tx_buf, len - 1, PROFILE_UP);
    

    note_set_send(tx_buf, len, times, resend_interval);//发送
}
#endif
//to modification 移植需要修改
/**
 * @description: 通用透传命令上报 (之前的LoRa协议 普通设备上报), 用户需要自己修改填写 模块地址，状态相关
 * @param {unsigned char} cmd1 
 * @param {unsigned char} cmd2
 * @param {unsigned char} times 重发次数
 * @param {unsigned short} resend_interval 重发间隔
 * @param {unsigned char} is_ack 是否应答包
 * @return {*}
 */
void note_comm_tran_up(unsigned char *SendBuf, unsigned char times, unsigned short resend_interval,unsigned char is_ack)
{
    unsigned char tx_buf[250];
    unsigned char len = 0;
    //unsigned char tx_num = (++g_note.tx_num)%64;
    unsigned char tx_num;

    //应答包的包序采用接收包包序
    if(is_ack)
    {
        tx_num = g_note.rx_num;
    }
    else
    {
        tx_num = (++g_note.tx_num)%64;
				user_fs_isNeedSave = 1;
			  RF_Num = g_note.tx_num;
    }

    profile_set_up(tx_num);//设置上行包
    if(is_ack)
    {//如果是应答包
        profile_set_ack(tx_num);
    }
    len++;//空余一下长度字段
    tx_buf[len++] = tx_num;
    memcpy(&tx_buf[len],g_note.device_uid,6);//发送方uid
    len += 6;
    
    memcpy(&tx_buf[len],g_note.gateway_uid,6);//注册网关uid
    len += 6;

    tx_buf[len++] = LOW_POWER_FLGA;//低功耗描述符
    #if MULTIHOP_EN_TX
    tx_buf[len++] = MULTIHOP_FLGA;//预留2字节
    #else
    tx_buf[len++] = 0;//预留2字节
    #endif
    tx_buf[len++] = 0x00;


    memcpy(&tx_buf[len], &SendBuf[SITE_DATA_STAT], SendBuf[SITE_LEN]);
    len += SendBuf[SITE_LEN];

    len++;//空一下校验字段
    tx_buf[SITE_ALL_LEN] = len - 1;
    tx_buf[len - 1] = profile_check(tx_buf, len - 1, PROFILE_UP);
    
    if(is_ack)
    {
        memcpy(g_rf_ackbak.send_buf,tx_buf,len);    				//备份回复包
        g_rf_ackbak.send_len = len;
				note_set_send(tx_buf, len, 0, COMM_RESEND_TIME);		//透传强行设置
    }
    else
    {
        note_set_send(tx_buf, len, 8, COMM_RESEND_TIME); // 透传强行设置
    }

    //LOG("isAck=%d\r\n",is_ack);
    //note_set_send(tx_buf, len, times, resend_interval);//发送
		
}



/**
 * @description: 通用应答
 * @param {unsigned char} T：命令域
 * @return {*}
 */
void note_comm_ack(unsigned char T)
{
    unsigned char tx_buf[50];
    unsigned char len = 0;
    //unsigned char tx_num = (++g_note.tx_num)%64;
		unsigned char tx_num = g_note.rx_num;
    profile_set_up(tx_num);//设置上行包
    profile_set_ack(tx_num);

    len++;//空余一下长度字段
    tx_buf[len++] = tx_num;
    memcpy(&tx_buf[len],g_note.device_uid,6);//发送方uid
    len += 6;
    
    memcpy(&tx_buf[len],g_note.gateway_uid,6);//注册网关uid
    len += 6;

    tx_buf[len++] = LOW_POWER_FLGA;//低功耗描述符
    #if MULTIHOP_EN_TX
    tx_buf[len++] = MULTIHOP_FLGA;//预留2字节
    #else
    tx_buf[len++] = 0;//预留2字节
    #endif
    tx_buf[len++] = 0x00;

    tx_buf[len++] = T;//命令
    tx_buf[len++] = 0;//没有数据长度0

    len++;//空一下校验字段
    tx_buf[SITE_ALL_LEN] = len - 1;
    tx_buf[len - 1] = profile_check(tx_buf, len - 1, PROFILE_UP);
    

    memcpy(g_rf_ackbak.send_buf,tx_buf,len);    //备份回复包
    g_rf_ackbak.send_len = len;
    
    //  note_set_send(tx_buf, len, times, resend_interval);//发送
    rf_tx(tx_buf,  len,0);


}

/**
 * @description: 扫描结果入队
 * @param {unsigned char} *rx_dat
 * @param {unsigned char} rssi
 * @return {*}
 */
unsigned char note_is_already_scan(unsigned char *rx_dat, unsigned char rssi)
{
    unsigned char ret = 0;
    
    if (g_note.is_scan_status && g_note.cur_scan_num < MAX_SCAN_NUM)
    { //如果在扫描状态, 添加扫描结果
        for(int i = 0; i < g_note.cur_scan_num; i++)
        {
            if(memcmp(g_note.scan_result[i].uid,&rx_dat[SITE_SRC_UID],6) == 0)
            {
                ret = i + 1;
            }
        }
    }
    return ret;
}

/**
 * @description: 获取信号最好的网关
 * @param {unsigned char} *rx_dat
 * @param {unsigned char} rssi
 * @return {*}
 */
unsigned char note_get_max_rssi_gateway(void)
{
    unsigned char max_rssi_id = 0;//最好的信号id
    unsigned char rssi = 255;
    unsigned char hop_times = 255;
    
    if (g_note.cur_scan_num < MAX_SCAN_NUM)
    { //如果在扫描状态, 添加扫描结果
        for(int i = 0; i < g_note.cur_scan_num; i++)
        {
            if(hop_times > g_note.scan_result[i].hop_times && g_note.scan_result[i].is_cannot_reg == 0)
            {//如果跳数比较少，优先
                hop_times = g_note.scan_result[i].hop_times;
                rssi = g_note.scan_result[i].rssi;
                max_rssi_id = i + 1;
            }
            else if(hop_times == g_note.scan_result[i].hop_times && rssi > g_note.scan_result[i].rssi && g_note.scan_result[i].is_cannot_reg == 0)
            {//如果跳数一样 则看信号强度
                rssi = g_note.scan_result[i].rssi;
                max_rssi_id = i + 1;
            }
        }
    }
    return max_rssi_id;
}


/**
 * @description: 设置发送 包含重发次数 重发间隔
 * @param {u8} cmd 发送命令
 * @param {u8} times 重发次数
 * @param {u16} resend_interval 重发间隔 最小为10
 * @return {*}
 */
void note_set_send(unsigned char *tx_dat, unsigned short len, unsigned char times, unsigned short resend_interval)
{
		
	
    if(len < SEND_BUF_SIZE)
    {
        if(g_note.reg_status || tx_dat[SITE_T] != T_TRAN)
        {//如果已经联网，或者 不是透传指令
            g_rf_send.is_need_send = 1;
            g_rf_send.resend_Times = times;
            g_rf_send.wait_ack_time = (resend_interval > 10)? resend_interval/10 : 1; //重发间隔

            memcpy(g_rf_send.send_buf, tx_dat, len);
            g_rf_send.send_len = len;			
        }
        else
        {
            if(tx_dat[SITE_T] == T_TRAN)
            {
                g_note.reg_timer = 0;
                g_note.is_need_report = 1;
            }
            LOG("please reg net\r\n");
						
            //未注册需要扫描网关
            #if	SET_BLE_SUB
            g_note.reg_timer = 0;
            #endif
        }
        // g_delayLoraSendTimer = 1;//lora延时发射，处理闪问题
    }
	
}


/**
 * @description: 关闭重发
 * @param {*}
 * @return {*}
 */
void note_close_resend(void)
{
	g_rf_send.is_need_send = 0x00;
	g_rf_send.resend_Times = 0x00;
}

/**
 * @description: 当前发送缓存是否有数据， 等待重发等
 * @param {*}
 * @return {*}
 */
unsigned char note_cur_is_send(void)
{
    unsigned char ret = 0;
    
    if(g_rf_send.is_need_send || g_rf_send.resend_Times)
    {
        ret = 1;
    }
		
    //正在发送
    if(g_note.is_tx_busy)
    {
        ret = 1;
    }

    return ret;
}
/**
 * @description: LoRa 发送重发扫描 10ms 跑一次
 * @param {*}
 * @return {*}
 */
void note_resend_scan(void)
{
		//220305: 重发次数标志
	
    if(g_rf_send.resend_Times)
    {//需要重发
        g_rf_send.wait_ack_timer++;
        if (g_rf_send.wait_ack_timer >= g_rf_send.wait_ack_time)
        {
            g_rf_send.is_need_send = 1;
            g_rf_send.wait_ack_timer = 0;
            g_rf_send.resend_Times--;

            if(g_rf_send.resend_Times == 0)
            {//重发次数达到上线
                LOG("resend times over:%02X\r\n",g_rf_send.send_buf[SITE_T]);

                //通讯测试完成
//                if(g_DeviceStatus== DEVICE_STA_TEST_RUN)
//                {
//                    g_DeviceStatus = DEVICE_STA_TEST;
//                    LOG("rx/tx num: %d/100\r\n",g_rf_send.send_test_ok);
//                }

                if(g_rf_send.send_buf[SITE_T] == T_SCAN_GATEWAY || g_rf_send.send_buf[SITE_T] == T_REG_NET)
                {//如果是扫描指令，则结束扫描，准备注网
                    g_note.is_scan_status = 0;
                    if(g_note.reg_status)
                    {//如果当前是有注册的        
												if(g_rf_send.send_fail_count<0xff)
                            g_rf_send.send_fail_count++;                       
                    }
                    else
                    {
                        if (note_get_max_rssi_gateway())
                        { //找到 信号最好的
                            LOG("ready reg net\r\n");
                            note_reg_net(g_note.scan_result[note_get_max_rssi_gateway() - 1].uid);
                            g_note.scan_result[note_get_max_rssi_gateway() - 1].is_cannot_reg = 1; //已经尝试注册过了，标记一下，如果失败就不注册这个了
                        
                                //有可能需要等正确网关来抢注
                                #if SET_BAT_WORK
                                if(sleepTimer<3000)
                                {
                                    sleepTimer = 3000;
                                }
                                #endif												
                        }
                    }
                }
                if(g_rf_send.send_buf[SITE_T] == T_TRAN)
                {
                    if(g_rf_send.send_fail_count<0xff)
                        g_rf_send.send_fail_count++;
                }
            }						
        }
    }
    else
    {
        g_rf_send.wait_ack_timer = 0;
    }

    // if(g_rf_send.delay_timer)
    // {
    //     g_rf_send.delay_timer--;
    // }
		//rf发送避开蜂鸣器
    if((g_rf_send.is_need_send)&&(hal_pwm_flag()==0))
    { //当前有需要发送的命令
        g_rf_send.is_need_send = 0;
			
        #if SET_BLE_FAIL
        g_rf_send.send_fail_count = 1;
        g_rf_send.resend_Times = 0;
        #else
			
        #if MULTIHOP_EN_TX
        //当前重发次数
        if(g_rf_send.resend_Times)
        {
            g_rf_send.send_buf[SITE_REP] = g_rf_send.resend_Times;
            g_rf_send.send_buf[g_rf_send.send_len - 1] = profile_check(g_rf_send.send_buf, g_rf_send.send_len - 1, PROFILE_UP);
        }
        rf_tx(g_rf_send.send_buf, g_rf_send.send_len,0);
        #else
        rf_tx(g_rf_send.send_buf, g_rf_send.send_len,0);
        #endif
			
		#endif
			
    }

    #if 0	
    //一次失败即需要重新注册
    //if(g_rf_send.send_fail_count >= 3)
    if(g_rf_send.send_fail_count > 0)
    {//如果大于等于 失败上线 重新注网
        g_rf_send.send_fail_count = 0;
        g_note.reg_status = 0;//注网状态清楚
    }
    #endif
}

/**
 * @description: 检测目标是发给谁的 UID 
 * @param {u8} *src_uid 来源UID
 * @param {u8} *tar_uid 目标UID
 * @param {u8} addr 地址 广播控制多个时使用
 * @return {*}
 */
u8 device_check_addr(u8 *src_uid, u8 *tar_uid, u8 addr)
{
	u8 ret = 0;

    if (/* memcmp(&g_device_info.memory.all_info.main_id[0], src_uid, 6) == 0 && */ memcmp(&g_note.device_uid[0], tar_uid, 6) == 0)
    {//开关面板
        ret = DEVICE_ID;
    }
    return ret;
}



/**
 * @description: 接受数据处理
 * @param {unsigned char} rx_dat 
 * @param {unsigned short} len
 * @param {unsigned char} rssi 信号强度， 需要取负值，例传入 40，实际为-40dbm
 * @return {*}
 */
void note_rx_deal(unsigned char *rx_dat, unsigned short len, unsigned char rssi, unsigned short phyFoff)
{
    unsigned char t_src = 0;
    // LOG("<<");
    UP_DOWN up_down = PROFILE_NULL;//收到的是什么包上行包还是下行包
#if(TEST_TX || TEST_RX)
    test_lose_pack_rx(rx_dat,  len, rssi);
#else
		static u8 iBle_Con_Rx;
    if(len < PROFILE_MIN_LEN)
    {//协议长度不对直接返回
        return;
    }
    if(rx_dat[len - 1] == profile_check(rx_dat, len - 1, PROFILE_DOWN))
    {//下行校验
        up_down = PROFILE_DOWN;
    }
    if(rx_dat[len - 1] == profile_check(rx_dat, len - 1, PROFILE_UP))
    {//上行校验校验
        up_down = PROFILE_UP;
    }
    
    #if MULTIHOP_EN
    //todo 多跳处理
    rf_mul_rx_dat(rx_dat, len, up_down);
    #endif

    if(up_down != PROFILE_DOWN)
    {//如果不是下行数据
			
				//测试
//				if(up_down == PROFILE_UP)
//				{
//					LOG("[PHY RX UP] [-%03ddbm %4dKHz len-%d ] ",rssi, phyFoff-512, len);
//					LOG_DUMP_BYTE(rx_dat, len);
//				}	
        return;
    }

    if(profile_is_up(rx_dat[SITE_NUM]))
    {//如果是上行包退出
        LOG("up dowm bit error\r\n");
        return;
    }

    if(iBle_Con_Rx!=(rx_dat[1]&0x3f))
    {
        iBle_Con_Rx = rx_dat[1]&0x3f;
        LOG("[PHY RX] [-%03ddbm %4dKHz len-%d ] ",rssi, phyFoff-512, len);
        LOG_DUMP_BYTE(rx_dat, len);
    }
	
    g_note.rx_type = NO_CAST;
		
    if(memcmp(&rx_dat[SITE_TAR_UID], g_note.device_uid, 6) == 0 /*|| memcmp(&rx_dat[SITE_TAR_UID], g_device_info.memory.all_info.main_id, 6) == 0*/)
    {// 节点通信, 目标uid为本机uid，或者保存的主机uid ，to modification 移植需要修改 保存的主机uid
        
        //221115:网关回复合二为一，不需要等待
        //多延时500ms,有可能接收lora协议
        // if(sleepTimer<500)
        // {
        //     sleepTimer = 500;
        // }
		
        LOG("note uid OK\r\n");
        
        g_note.rx_type = NOTECAST;
        g_rf_send.send_fail_count = 0;
        g_note.heartbeat_timer = 0;//心跳计时
			
        //211105:保存UID
        if((rx_dat[SITE_T]!=T_REG_NET) && (rx_dat[SITE_T]!=T_SCAN_GATEWAY)&& (g_note.reg_status==0) )
        {
            LOG("add Gate Uid\r\n");
            g_note.reg_status = 1;
            memcpy(g_note.gateway_uid, &rx_dat[SITE_SRC_UID], 6);
        }  				
    }
    else if(memcmp(&rx_dat[SITE_TAR_UID], all_ff_buf, 6) == 0)
    {// 广播通信
        g_note.rx_type = BROADCAST;
    }
    else if(memcmp(&rx_dat[SITE_TAR_UID], all_00_buf, 5) == 0)
    {// 组播通信
        g_note.rx_type = MULTICAST;
    }

    t_src = device_check_addr(&rx_dat[SITE_SRC_UID], &rx_dat[SITE_TAR_UID], 0);

    if (profile_is_ack(rx_dat[SITE_NUM]) && t_src)
    { // 是应答包
        if ((rx_dat[SITE_T] == g_rf_send.send_buf[SITE_T]) && ((rx_dat[SITE_NUM] % 64) == (g_rf_send.send_buf[SITE_NUM] % 64)))
        { // 并且命令一样 则关闭重发
            note_close_resend();
        }
        memcpy(g_note.gateway_uid, &rx_dat[SITE_SRC_UID], 6);
        g_note.reg_status = 1;//当前注网状态
    }
  
#endif
}


/**
 * @description: 节点流程处理 10ms 跑一次
 * @param {*}
 * @return {*}
 */
void note_process(void)
{
    static unsigned char _10ms_count = 0;
    static unsigned char _100ms_count = 0;


    note_resend_scan();

#if 1
    if(++_10ms_count >= 10)
    {//100ms 跑一次
        _10ms_count = 0;
        if(++_100ms_count >= 10)
        {//1s
            _100ms_count = 0;
            if(g_note.reg_timer)
            {//注册倒计时
                g_note.reg_timer--;
            }
            
            g_note.heartbeat_timer++;

        }
    }
#endif
}

#endif







