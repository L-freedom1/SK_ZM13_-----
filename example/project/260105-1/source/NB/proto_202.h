#ifndef	PROTO_H_202
#define	PROTO_H_202

#include	"config.h"

#if	SET_PROTO_VER==PROTO_VER_ZH202
//#define	TIME_OUT_NET			300		//5分钟
#define	TIME_OUT_NET			10
#define	SMART_DEVICE_NUM	1
extern u32  UnixTimestamp;
extern char *DEVICE_VER;

extern u32  g_nbiot_last_read_time;
//extern u8  gMCUID[6];
#define	gMCUID	g_device_info.s_cpu_id
/*******************************
协议属性字段定义
*******************************/
#define	ID_UPDATE	0x23

#ifdef SET_IP
extern bit g_IP_PORT_Change;
#endif
extern bit pIsNeedSnd(void);
extern void NB_TxPro(void);
extern u8 p_prto_dispose_data(u8 *buf, u16 len);


void pCreatHead(void);
void pCreat9120Head(void);
extern u8  VER_SW[8];

/************************************************** 新协议相关内容 *****************************************************/
/* 网络字段头 位置 */
#define NET_SITE_SYCN_NUM 0//魔术字固定0xFF
#define NET_SITE_PN       1//协议号用于指示不同的协议。本协议PN为0
#define NET_SITE_VCH      2//版本号（Version）和网络通道（Channel）
#define NET_SITE_CHK      3//校验和（Checksum）是对整个包（只一个分片）的所有字节进行按位异或操作的结果


#define NET_SITE_SN       4//序列号2字节（Sequence Number）从0开始逐次加1
#define NET_SITE_FLAG     6//标识字节，错误包和应答包均不分片。一个包不能既是错误包又是应答包。a，W=0且A=0时, D0~D4表示分片序号，用以排序和合并分片。如果没有分片则E=1且分片序号=0。
#define NET_SITE_CT      7//
/* 包类型2字节（Content Type），最高位D15表示DATA是否被加密。D10~D14表示包数据类型（见表格3）。
D0~D9表示DATA的字节数，最大长度为2^10字节（1KB）。一个带分片的包最多可传2^10 * 2^5 = 32KB数据。
本协议目前未加入加密通信功能，仅使用CT_HEARTBEAT和CT_APPLICATION_DATA。 */
#define NET_SITE_DI      9//设备类型标识
#define NET_SITE_DUID    10//设备唯一码6字节
#define NET_SITE_DATA_START    16//数据起始字节


#define PROTOCOL_OLD      1//旧协议，远程升级用
#define PROTOCOL_NEW      2//新协议
#define PROTOCOL_6100	  3//6100协议


#define NET_HEAD_FF 0xFF//魔术字0xFF
#define NET_PN 0x00 //协议号  0x00
// #define NET_V 0x08//协议版本号 2.0

#define NET_V 0x04//协议版本号 1.0

/* 联网方式 */
#define NET_TYPE_GPRS 0
#define NET_TYPE_WIFI 1
#define NET_TYPE_WIRE 2
#define NET_TYPE_NBIOT 3
#define NET_TYPE_4G 4
#define NET_TYPE_5G 5

/* 标记字节 */
#define NET_FLAG_ERR	0x80//是否错误包
#define NET_FLAG_ACK	0x40//是否确认包
#define NET_FLAG_M		0x00//既不是错误包也不是确认包，主动发送包
#define NET_FLAG_END_PACK	0x20//是否末尾包

/* 错误类型 */
#define ERR_NULL 		0	//无错误
#define ERR_HEARD_ERR 1	//包头格式错，无法解析
#define ERR_PROTO_NO 2	//协议号错误
#define ERR_PROTO_VER 3	//不支持的协议版本
#define ERR_CHECK_ERR 4	//校验和错
#define ERR_LENS_ERR 5	//数据长度与包头字段不符合
#define ERR_JSON_ERR 6	//JSON格式错
#define ERR_DECRYPT_ERR 7	//无法解密

#define ERR_RESET_SN 31	//重置SN

/* 包类型 */
#define CT_HEARTBEAT	0	//心跳包。
#define CT_CHANGE_CIPHER_SPEC	20	//更改加密规则。
#define CT_ALERT	21	//告警。
#define CT_HANDSHAKE	22	//握手。
#define CT_APPLICATION_DATA	23	//应用数据（二进制）。
#define CT_JSON_DATA	24	//应用数据（UTF8编码的JSON）。
#define	CT_HEX_DATA		25	//应用数据(十六进制协议)


#define	CMD_HEX_DATA		0xA1
#define CMD_HEX_SIG			0xA2


#define CMD_HEARTBEAT	0X01//	心跳
#define CMD_STATUS_M_UP 0X02//	主动上报状态（状态改变的控制器状态）
#define CMD_EVENT_UP 0X03//	主动上报物理操作


#define	CMD_REPORT_DEVICE_EVENT		0x05		//主动上报主机事件
#define	CMD_REPORT_DETECTOR_EVENT	0x06		//主动上报探头事件
#define CMD_GET_SYNC_TIME 0X0A//	请求同步时间


#define CMD_ACK_SYNC_TIME 0X8A	//回复同步时间

#define CMD_RESET_DEF 0X0F	//请求恢复出厂设置(应以0X03指令回复)
#define CMD_QUERY_DEVICE_TYPE 0X09	//查询设备列表
#define CMD_RET_DEVICE_TYPE 0X89	//回复设备列表


#define CMD_QUERY_REAL_STATUS 0X11 //查询探头真实状态
#define CMD_RET_REAL_STATUS 0X91 //回复查询探头真实状态
#define CMD_QUERY_DETECTOR_RSSI 0X12 //查询探头信号强度（特殊点：以确认包回复，见下条指令）
#define CMD_RET_DETECTOR_RSSI 0X92 //“确认”查询探头信号强度（注意不是回复）
#define CMD_QUERY_DETECTOR_DATA 0X13 //查询探头的传感数据
#define CMD_RET_DETECTOR_DATA 0X93 //回复查询探头的传感数据
#define CMD_RESET_DETECTOR 0x14	//重置探头
#define	CMD_RET_RESET_DETECTOR				0x94	//确认重置探头

/* 网络通信 键值表 */
#define KEY_OP_CODE 'o'	//操作码,表示何种指令
#define KEY_ENCODE 'c'	//编码
#define KEY_KEY 'v'	//值
#define KEY_VER 'V'	//版本号
#define KEY_LIST	"WL"	//白名单校验

#define	KEY_PARA	'P'	//参数校验
#define KEY_STATUS 's'	//状态
#define KEY_SINGAL 'i'	//信号强度
#define	KEY_ONLINE	'L'		//在线状态
#define KEY_START_UP 'b'	//开机启动
#define KEY_RESET 'r'	//重置（恢复出厂设置）
#define KEY_TIME_TIME 't'	//时间戳（距1970-01-01 00:00:00的秒数），4个元素的整型数组合起来表示一个整数，0元素为最低字节。
	
#define KEY_DETECTOR 'k'	//探头（控制器或传感器）类型
#define KEY_ZONE_NO 'a'	//防区号
#define KEY_RET 'R'	//结果值
	
#define KEY_CHECK '0'	//校验码
#define KEY_SWITCH_NO '1'	//开关-防区
#define KEY_SWITCH_VAL '2'	//开关-值
#define KEY_SOCKET_NO '3'	//插座-防区
#define KEY_SOCKET_VAL '4'	//插座-值
#define KEY_RC_NO '5'	//红外遥控-防区
#define KEY_RS_NO '7'	//红外插座的编号(防区号)
#define KEY_RS_VAL '8'	//红外插座的状态，数组
#define KEY_RC_USER_LIB 'L'	//空调码源(码库或自定义)-值
#define KEY_RC_CODE_LIB 'C'	//空调控制码(码库或自定义)-值
#define KEY_ON_OFF 'S'	//开或关
#define KEY_MODE 'M'	//模式
#define KEY_TEMP 'T'	//温度
#define KEY_SPEED 'W'	//风速
#define KEY_DIR 'D'	//风向
#define KEY_RC_OP 'B'	//控制项

#define KEY_SWITCT_CHECK 'X'	//开关值校验码
#define KEY_SOCKET_CHECK 'Y'	//插座值校验码
#define KEY_RC_CHECK 'Z'	//红外值校验码
#define KEY_JSON_SN	'n'	//SN，序列号（2字节无符号整数）,服务器下传原数据回复即可

#define KEY_DEVICE_CHECK '0'	//设备类型校验值

#define KEY_SV "sv"	//关键字，电压
#define KEY_SI "si"	//关键字，电流
#define KEY_SL "sl"	//关键字，剩余电流
#define KEY_SR "sr"	//关键字，功率因数
#define KEY_SP "sp"	//关键字，有功功率
#define KEY_SQ "sq"	//关键字，累计电量
#define KEY_ST "st"	//关键字，温度
#define KEY_SH "sh"	//关键字，湿度


#define KEY_MUL_STATUS "s"	//状态 
#define KEY_MUL_ZONE_NO "a"	//防区号
#define	KEY_EVENT	"e"

#define	KEY_ST1	"st1"
#define	KEY_ST2	"st2"
#define	KEY_AIR	"at"



/********************************透传HEX协议相关****************/
#define	CMD_LORA_DATA_BAK		0xa0		//不在白名单的数据，不做重发处理
#define	CMD_LORA_DATA			0xa1
#define	CMD_LORA_SIG			0xa2
#define	CMD_LORA_CAD			0xa3
#define	CMD_LORA_FRE			0xa4

#define	CMD_LIST_CLR			0xb1
#define	CMD_LIST_ADD			0xb2
#define	CMD_LIST_DEL			0xb3
#define	CMD_LIST_UID			0xb4

#define	CMD_PARA_QUERY			0xb5
#define	CMD_PARA_SET			0xb6
#define	CMD_WAKE_DELAY			0xb7
#define	CMD_RESET				0xb8


#define	CMD_BACK_HOST			0xc1
#define	CMD_BACK_DE1			0xc2
#define	CMD_BACK_DE2			0xc3
#define	CMD_BACK_DE3			0xc4

#define	CMD_TRAN_6100			0xe1

#define	LEN_MAX_UDP_TX		660
#define 	NET_HEARTBEAT_TIME	50	//30s一次上行心跳用于保存链路

typedef struct 
{
	u16 TxLen;					//发送长度
	u8 TxBuf[LEN_MAX_UDP_TX];	//发送缓冲
	#if SET_CAT1
	u8 ReTxBuf[LEN_MAX_UDP_TX];
	u16 ReTxLen;
	#endif
	u8 rx_buf[LEN_MAX_UDP_TX];	//接收缓冲
	u8 UDPCh1Timer120s;
	u8 reconn_count;
	u8 server1Status;
	u8 server2Status;
	u16 hreadbeat_timer1; 		//wifi定时心跳上行，出现一段时间（十几秒）设备不与服务器通信则链路断开
	
	//协议
	u16 sn_down; 				//下行唯一包序号
	u16 sn_up[1];	  			//上行唯一包序号
	u16 json_sn[1];	 			//服务器下行json内容的sn，服务器要用，设备不管，原样回复即可
	
	u8 IpChangeFlag[2];

}CbNet_InitTypeDef;

extern CbNet_InitTypeDef  g_cbNet;
extern u8 IsServerFail(void);

extern void pNetCut(u16 Len,u8 SocketIndex);

//不做网关
#if SET_GATEWAY
#define TRAN_UP_QUEUE_SIZE		    660
extern st_pool	 g_tran_up_pool;
#endif

extern u8	DEVICE_EEPROM[SMART_DEVICE_NUM*8];
extern u8 cust_del_smart_device(u16 channal);

#endif

#endif
