#ifndef __BC95_H_
#define __BC95_H_

#include	"config.h"

#define	NB_COAP_EN			1
#define	SET_PSM		

#define NB_SN12				1
#define NB_ME3616			0
#define	NB_BC20				2
#define	NB_MN316			3
#define	NB_N306				5
#define	NB_N701				4
#define	NB_MODULE			NB_N701

#if SET_NB
//宏定义
#define 	Sleep_ms(a)            	os_waitMs(a)

#define 	g_b_m26l_r_ok		    		COM1.B_RX_OK
#define 	g_u16_m26l_rindex	    	COM1.RX_Cnt
#define		p_swut_write2buff				TX1_write2buff

extern char *g_c_m26l_rbuf;

typedef struct
{
	//u8  m_dev_uuid[6];			// 主板UUID
	u8  m_csq;					// NB 信号

	u8  m_nbiot_err_cnt;		// NB模块错误次数,超过限制后要重新初始化

	//u16 m_last_eve_id;
	u32  m_last_nbiot_connect_time;

	u32 m_system_uptime;
	u32 m_wwdg_flag;
	u32 m_last_nbiot_send_time;
	u8  m_last_nbiot_resend_count;
	u32 m_nbiot_start_proc_time;

	struct {
		u8  m_nbiot_init_ok_cnt;
		u8  m_nbiot_init_fail_cnt;
		u8	m_nbiot_no_signal_cnt;
		u8  m_nbiot_send_cnt;
		u8  m_nbiot_recv_cnt;
	}m_nb_stat;

	/* TRUE or FALSE */
	//u8  m_fast_sleep	       : 1;
	u8  m_flg_sending		   : 1;		//正在发送
	u8  m_nbiot_init_ok        : 1;
	u8  m_nbiot_create_sock_ok : 1;
	//u8  m_nbiot_canbe_init     : 1;
	u8  m_flg_retry    		   : 1;		//重发标志
	u8  m_flg_psm			   : 1;
	u8  m_flg_str			   : 1;
	u8  m_flg_reply				: 1;
	#if (NB_MODULE==NB_N306) || (NB_MODULE==NB_N701) 
	u8  m_flg_n306			   :1;	
	#endif

	/* NB模块信息 */
	char m_socket_id;
//	char m_imei[15];
//	char m_imsi[15];
//	char m_iccid[20];
	char m_imei[16];
	char m_imsi[16];
	char m_iccid[21];
	
	u16 mMessageId;

	u32 earfcn;			//服务小区频点
	u16 cellid;			//服务小区id
}st_run_info;
extern st_run_info  g_run_info;


//管脚定义
#define	ME3616_WAKE_IN
#define	ME3616_WAKE_OUT
#define	ME3616_POW_EN					GPIO_P33
#define	SET_ME3616_POW_EN(x)			hal_gpio_write(ME3616_POW_EN,x)

#define	ON_POW_NB		0
#define	OFF_POW_NB		1

#define	ME3616_NB_ON					GPIO_P34
#define	SET_ME3616_NB_ON(x)				hal_gpio_write(ME3616_NB_ON,x)

#define	ON_RST_NB		1
#define	OFF_RST_NB		0

#define	MN306_WAKE						GPIO_P32
#define	SET_MN306_WAKE(x)				hal_gpio_write(MN306_WAKE,x)

#define	ON_WAKE_NB		0
#define	OFF_WAKE_NB		1


#define 	p_sn12_init()				p_me36_init()
u8		p_me36_init(void);

void p_sn12_gpio_init		  (void);
void p_sn12_boot              (void);
u8   p_nblc_common_func       (u8 send_string, char *at_cmd, char *match_cmd, u16 wait_ms, u8(*proc_cb)(char *),u8 iWait_OK);
void p_uart_send_string		  (const char *c);
/*三、一次性配置指令（扰码配置）：新模块未配置，将导致无法附着到网络；*/

/*四、SN12初始化、联网指令：*/
u8 p_nblc_communic_test       (void);
u8 p_nblc_get_cimi            (void);
u8 p_nblc_get_cereg           (void);
u8 p_nblc_set_cereg           (void);
u8 p_nblc_UDP_connect_net     (void);
u8 p_nblc_port_send_data      (u8 *udp_data, u16 len);
u8 p_nblc_read_data(void);
u8 p_nblc_get_csq			  (void);
u8 p_nblc_get_npsmr			  (void);
u8 p_nblc_set_npsmr			  (void);
//u8 p_blkc_proc_fuck			  (char *line);
//u8 p_nblc_proc_at_cmd		  (void);
u8 itohex                     (u8 aHex);
u8 hextoi                     (u8 hexch);

u8 p_nblc_proc_adc(char *line);

extern void NB_pro(void);

#ifdef	SET_PSM
extern void NB_PsmSleep(void);
extern u8 NB_PsmWake(void);
#endif
extern void NB_ReadVbatt(void);

extern void pDelConnect(void);
#if NB_MODULE==NB_N306
extern void NB_setBand(void);
#endif

#endif

#endif
