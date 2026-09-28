/*
 * 433M 1527 编码发送(PHY6222 / OSAL 工程)
 * 说明: 波形由硬件定时器中断逐拍翻转 IO 产生, 发送期间不能进入休眠(休眠会关掉定时器)。
 *       节拍定时器: AP_TIMER_ID_6, 50us 一次中断, 由 my_timer.c 的
 *       timer_int_process(HAL_EVT_TIMER_6) 调用 user_1527_tick()。
 */
#include "config.h"
#include "user_1527.h"

/* 调试打印: SET_TEST=1 时才输出 */
#if SET_TEST
#define RF1527_LOG(...)     LOG(__VA_ARGS__)
#else
#define RF1527_LOG(...)
#endif

/* 发送节拍定时器 */
#define RF1527_TIMER_ID         AP_TIMER_ID_6
#define RF1527_TICK_US          50

/* 发送状态机 */
typedef enum {
    TX_STATE_IDLE1 = 0,
    TX_STATE_SYNC_HIGH,      /* 同步头高电平 */
    TX_STATE_SYNC_LOW,      /* 同步头低电平 */
    TX_STATE_BIT_HIGH,      /* 数据位高电平 */
    TX_STATE_BIT_LOW,       /* 数据位低电平 */
    TX_STATE_FRAME_GAP,     /* 帧间间隔 */
} tx_state_t;

/* 时序参数 (单位: 50us tick, 由定时器提供) */
#define TX_SYNC_HIGH_TICKS     3        /* 同步高电平 */
#define TX_SYNC_LOW_TICKS      100      /* 同步低电平 */
#define TX_BIT0_HIGH_TICKS     3        /* Bit0高电平 */
#define TX_BIT0_LOW_TICKS      11       /* Bit0低电平 */
#define TX_BIT1_HIGH_TICKS     11       /* Bit1高电平 */
#define TX_BIT1_LOW_TICKS      3        /* Bit1低电平 */
#define TX_REPEAT_COUNT        20       /* 重复发送次数 */
#define TX_FRAME_BITS          24       /* 每帧24bit */
/* 帧间隔必须为0: 24bit发完后直接接下一帧同步头, 不能再插入低电平。
   否则最后一bit的低电平会和间隔连在一起被拉长(bit1的短低150us+550us=700us),*/
#define TX_FRAME_GAP_TICKS     0        /* 帧间隔: 不用, 保留兼容 */

/* 发送状态变量(定时器中断里修改, 需 volatile) */
static volatile tx_state_t  tx_state = TX_STATE_IDLE1;
static volatile uint32      tx_data = 0;
static volatile uint8       tx_bit_index = 0;
static volatile uint16      tx_tick_count = 0;   /* 帧间隔可能超过255 tick, 用16位避免溢出 */
static volatile uint8       tx_repeat_count = 0;

/**
 * @description: 发送收尾: 关定时器、拉低数据脚、关模块供电
 * @return {*}
 */
static void user_1527_tx_finish(void)
{
    // hal_timer_stop(RF1527_TIMER_ID);
    tx_state = TX_STATE_IDLE1;
    RF1527_TX(0);
    RF1527_EN(1);           /* 关断 433 发射模块 */
}

/**
 * @description: 定时器节拍(50us)驱动 1527 发送状态机, 由 my_timer.c 的 HAL_EVT_TIMER_6 调用
 * @return {*}
 */
void user_1527_tick(void)
{
    uint8 bit_val;
    uint8 high_ticks;
    uint8 low_ticks;

    if (tx_state == TX_STATE_IDLE1)
        return;

    tx_tick_count++;

    /* 时序统一规则: 每个阶段在进入时把 tx_tick_count 清0,
       计数达到该阶段 tick 数的那一拍立即翻转电平并清0。
       这样每个阶段严格持续 N 个 tick, 不会多一拍也不会少一拍 */
    switch (tx_state)
    {
        /*--- 同步头: 高电平 ---*/
        case TX_STATE_SYNC_HIGH:
            if (tx_tick_count >= TX_SYNC_HIGH_TICKS)
            {
                tx_tick_count = 0;
                tx_state = TX_STATE_SYNC_LOW;
                RF1527_TX(0);
            }
            break;

        /*--- 同步头: 低电平 ---*/
        case TX_STATE_SYNC_LOW:
            if (tx_tick_count >= TX_SYNC_LOW_TICKS)
            {
                /* 同步头结束, 当前 tick 立即拉高, 作为 bit0 高电平的起点 */
                tx_tick_count = 0;
                tx_state = TX_STATE_BIT_HIGH;
                RF1527_TX(1);
            }
            break;

        /*--- 数据位高电平 ---*/
        case TX_STATE_BIT_HIGH:
            bit_val = (tx_data >> (TX_FRAME_BITS - 1 - tx_bit_index)) & 0x01;
            high_ticks = bit_val ? TX_BIT1_HIGH_TICKS : TX_BIT0_HIGH_TICKS;

            if (tx_tick_count >= high_ticks)
            {
                tx_tick_count = 0;
                tx_state = TX_STATE_BIT_LOW;
                RF1527_TX(0);
            }
            break;

        /*--- 数据位低电平 ---*/
        case TX_STATE_BIT_LOW:
            bit_val = (tx_data >> (TX_FRAME_BITS - 1 - tx_bit_index)) & 0x01;
            low_ticks = bit_val ? TX_BIT1_LOW_TICKS : TX_BIT0_LOW_TICKS;

            if (tx_tick_count >= low_ticks)
            {
                tx_tick_count = 0;
                tx_bit_index++;

                /* 一帧24bit发完 */
                if (tx_bit_index >= TX_FRAME_BITS)
                {
                    tx_repeat_count++;
                    if (tx_repeat_count >= TX_REPEAT_COUNT)
                    {
                        /* 发送完成, 回到空闲 */
                        user_1527_tx_finish();
                    }
                    else
                    {
                        /* 帧间隔并入同步头: 当前 tick 立即拉高, 开始下一帧同步头。
                           24bit后不要再插低电平, 否则最后一bit的低电平被拉长会误判 */
                        tx_tick_count = 0;
                        tx_bit_index = 0;
                        tx_state = TX_STATE_SYNC_HIGH;
                        RF1527_TX(1);
                    }
                }
                else
                {
                    /* 下一个bit: 当前 tick 立即拉高, 作为下一个高电平的起点 */
                    tx_state = TX_STATE_BIT_HIGH;
                    RF1527_TX(1);
                }
            }
            break;

        /*--- 帧间隔 ---*/
        case TX_STATE_FRAME_GAP:
            if (tx_tick_count >= TX_FRAME_GAP_TICKS)
            {
                tx_tick_count = 0;
                tx_bit_index = 0;
                tx_state = TX_STATE_SYNC_HIGH;
                RF1527_TX(1);
            }
            break;

        default:
            user_1527_tx_finish();
            break;
    }
}

/**
 * @description: 1527 发送(非阻塞, 波形在定时器中断里产生)
 * @param {uint32} tx_dat 24bit 码值
 * @return {*}
 */
void user_1527_tx(uint32 tx_dat)
{

    RF1527_EN(0);           /* 打开 433 发射模块(同步头低电平5ms, 足够模块上电稳定) */

    tx_data = tx_dat & 0x00FFFFFF;
    tx_state = TX_STATE_SYNC_HIGH;
    tx_bit_index = 0;
    tx_tick_count = 0;
    tx_repeat_count = 0;

    /* 起始: 高电平 */
    RF1527_TX(1);
    LOG("1527 tx: 0x%06lX\r\n", tx_data);
}

/**
 * @description: 发送心跳码
 * @param {u8} hb_code 心跳状态码(00/C3/C1/C4)
 * @return {*}
 */
void user_1527_tx_heartbeat(u8 hb_code)
{
    RF1527_LOG("1527 tx heartbeat: 0x%02X\r\n", hb_code);
    // user_1527_tx(RF1527_HEARTBEAT_CODE(hb_code));
}

/**
 * @description: 停止发送
 * @return {*}
 */
void user_1527_tx_stop(void)
{
    user_1527_tx_finish();
    RF1527_LOG("1527 tx stop\r\n");
}

/**
 * @description: 1527初始化(在 GPIO_config() 里调用)
 * @return {*}
 */
void user_1527_init(void)
{
    /* 数据脚: 推挽输出, 默认低 */
    app_gpio_set(PIN_RF1527_TX, GPIO_OUTPUT, GPIO_FLOATING);
    RF1527_TX(0);

#if RF1527_USE_PWR_EN
    /* 433 模块供电使能脚: 默认关断 */
    app_gpio_set(PIN_RF1527_EN, GPIO_OUTPUT, GPIO_FLOATING);
    RF1527_EN(1);
#endif

    hal_timer_stop(RF1527_TIMER_ID);

    tx_state = TX_STATE_IDLE1;
    tx_bit_index = 0;
    tx_tick_count = 0;
    tx_repeat_count = 0;
}

/**
 * @description: 1527 是否正在发送(休眠前必须判断: 休眠会关掉定时器, 发送中休眠会破坏波形)
 * @return {*} 1: 正在发送  0: 空闲
 */
uint8 user_1527_is_tx_busy(void)
{
    if (tx_state == TX_STATE_IDLE1)
    {
        return 0;
    }
    return 1;
}
