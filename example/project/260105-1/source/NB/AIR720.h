#ifndef __AIR720_H_
#define __AIR720_H_

#include "config.h"


#define	AIR720		1
#define	EC600			2
#define	N716			3
#define	ML307R		4
#define N701			5

#define MODE_TYPE					N701

#if SET_CAT1

#define	MultiLink_EN			1

#define	MAX_UDP_LEN				660
#define	MAX_IP_STR_LEN		20

#define	NB_COAP_EN		1



//sbit POW_NB_EN = P4^0;
//sbit NB_ONOFF = P0^7;
#define	PIN_LTE_RST_N		GPIO_P34
#define	PIN_LTE_PWR_C		GPIO_P33
#define 	SET_LTE_RST_N(x)	hal_gpio_write(PIN_LTE_RST_N,x)
#define 	SET_LTE_PWR_C(x)	hal_gpio_write(PIN_LTE_PWR_C,x)
extern u8 isLte_Onff,isC_Lte;
#define SOCK_NUM	2
extern u8  init_Tmr;
extern u8  sock_Tmr[2];
#define	LTE_POW_OFF	0
#define	LTE_POW_ON	1

#define	NB_ON  1
#define	NB_OFF 0


#define 	g_b_m26l_r_ok		    		COM1.B_RX_OK
#define 	g_u16_m26l_rindex	    		COM1.RX_Cnt
#define		p_swut_write2buff				TX1_write2buff

#define C_SN12_WAIT_AT_SEC			20000
#define C_SN12_WAIT_SIM_SEC     	20000   	//�鿨30��
#define C_SN12_WAIT_CS_PS_SEC   	120000   	//����120��
#define C_SN12_REBOOT_PIN_SEC		3
#define C_M26_MAX_PROC_LINE			20
#define g_m26l_active_receive_pro(r)
#define Sleep_ms(a)                 os_waitMs(a)

extern char *g_c_m26l_rbuf;
extern u8  g_bWaitTimeOut1s;

#define		STA_4G_INT_0		0	//????
#define		STA_4G_INT_BOOT 1	//??
#define		STA_4G_INT_AT 	2	//??AT
#define		STA_4G_INT_CARD	3	//??
#define		STA_4G_INT_REG	4	//????

typedef struct
{
	//u8  m_dev_uuid[6];			
	u8  m_csq;					// NB �ź�
	u8 	m_rsrp;
	
	u32  m_last_nbiot_connect_time;
	u8  m_nbiot_err_cnt;		

	//u16 m_last_eve_id;
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

	u8 m_nbiot_create_sock_ok;
	/* TRUE or FALSE */
	u8  m_flg_sending		   	: 1;		//���ڷ���
	u8  m_nbiot_init_ok        	: 1;
	u8  m_flg_reply			   	: 1;		//�ظ�
	u8  m_flg_retry    		   : 1;		//�ط���־
	u8  m_flag_needSnd			:1;

	/* NBģ����Ϣ */
	char m_socket_id;
	//char m_imei[15];
	char m_imei[16];
	//char m_imsi[15];
	char m_imsi[16];
	char m_iccid[21];

	//u8 isInit;	
	#if SET_BLE_LEARN
	u8 m_nbiot_init_sta;					//��ʼ���ĳ�״̬��
	#endif	
}st_run_info;
extern st_run_info  g_run_info;

void p_sn12_gpio_init		  (void);
void p_sn12_boot              (void);
void p_uart_send_string		  (const char *c);
/*����һ��������ָ��������ã�����ģ��δ���ã��������޷����ŵ����磻*/

/*�ġ�SN12��ʼ��������ָ�*/
u8 p_nblc_communic_test       (void);

u8 itohex(u8 aHex);
u8 hextoi(u8 hexch);

extern void NB_pro(void);
extern void p_RunPro(void);

void p_uart_send_string(const char *c);
void p_m26l_format_data(u16 max_len);
void p_m26l_clear_buffer(u16 bLen);
void p_m26l_clear_recv_buffer(void);
char *p_m26l_get_line(char *line, u32 max_len);

extern bit  is_get_card;
extern bit  is_Reg_Net;
extern bit  is_GetAt;
extern bit  is_GetIp;

#ifdef	SET_IP
extern u8 check_str_is_ip(u8 * ip_str,u8 len);
extern u8 ip_str_to_ByteArray(u8 * ip_str,u8 *ByteArray);
extern u8 ip_byteArray_to_ip_str(u8 * ByteArray,u8 * ip_str);
extern u8 p_check_air720_Udp_ipAndPort(u8 * IP_buf,u16 port);
extern u8 p_air720_get_local_ip(void);
#endif

#define p_nblc_common_func					p_air720_common_func
#define p_nblc_port_send_data(dat,len)	p_air720_port_send_data(0,dat,len)
#define p_nblc_UDP_connect_net				p_air720_UDP_connect_net
#define p_nblc_read_data()					p_air720_read_data()
#define p_nblc_proc_read_data()				p_air720_proc_read_data()

u8 p_air720_common_func(u8 send_string, char *at_cmd, char *match_cmd, u16 wait_ms, u8 (*proc_cb)(char *),u8 iWait_OK);
#if (MultiLink_EN > 0)
u8 p_air720_set_ciicr(void);
u8 p_air720_set_cstt(void);
u8 p_air720_check_local_ip(char * line);
u8 p_air720_get_local_ip(void);
u8 p_air720_start_udp_mux_connect(u8 channel,u8 * IP_buf,u16 port);
u8 p_air720_check_socket_proc(char *line);
u8 p_air720_port_send_data(u8 channel,u8 *udp_data, u16 len);
#else
u8 p_air720_start_udp_connect(u8 * IP_buf,u16 port);
u8 p_air720_set_ciphead(void);
u8 p_air720_port_send_data(u8 channel,u8 *udp_data, u16 len);	
#endif
u8 p_air720_read_data(void);
u8 p_air720_check_socket(void);
u8 p_air720_UDP_connect_net(u8 socket);
u8 p_air720_proc_read_data(char *line);


//u8 p_air720_common_func(u8 send_string, char *at_cmd, char *match_cmd, u16 wait_ms, u8 (*proc_cb)(char *),u8 iWait_OK);
u8 p_air720_init(void);
u8 p_air720_set_npsmr(void);
u8 p_air720_set_ATE(u8 Type);
u8 p_air720_get_cpin(void);
u8 p_air720_get_CREG(void);
u8 p_air720_get_CGREG(void);
u8 p_air720_set_cgatt(void);
u8 p_air720_get_cgatt(void);

u8 p_air720_set_cipshut(void);
u8 p_air720_set_cipmux(u8 Type);
u8 p_air720_proc_csq(char *line);
u8 p_air720_get_csq(void);

// #if (MultiLink_EN > 0)
// u8 p_air720_set_ciicr(void);
// u8 p_air720_set_cstt(void);
// u8 p_air720_check_local_ip(char * line);
// u8 p_air720_get_local_ip(void);
// u8 p_air720_start_udp_mux_connect(u8 channel,u8 * IP_buf,u16 port);
// u8 p_air720_check_socket_proc(char *line);
// u8 p_air720_port_send_data(u8 channel,u8 *udp_data, u16 len);
// #else
// u8 p_air720_start_udp_connect(u8 * IP_buf,u16 port);
// u8 p_air720_set_ciphead(void);
// u8 p_air720_port_send_data(u8 *udp_data, u16 len);
// #endif

// u8 p_air720_read_data(void);
// u8 p_air720_check_socket(void);
// u8 p_air720_UDP_connect_net(void);
// u8 p_air720_proc_read_data(char *line);

u8 p_creat_card_info(void);
u8 StrToHexArr(u8 *buf,u8 buf_len,u8 *Out_Arr);

#define p_sn12_init()						p_air720_init()
#define p_nblc_get_csq()					p_air720_get_csq()
// #define p_nblc_common_func				p_air720_common_func
// #define p_nblc_port_send_data(dat,len)	p_air720_port_send_data(dat,len)
// #define p_nblc_UDP_connect_net()			p_air720_UDP_connect_net()
// #define p_nblc_read_data()				p_air720_read_data()
// #define p_nblc_proc_read_data()			p_air720_proc_read_data()
#endif


extern unsigned char note_cur_is_send(void);
#endif








