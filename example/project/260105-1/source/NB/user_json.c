/*
 * @Author: dujianhuang
 * @Date: 2020-06-12 11:26:43
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-03-21 00:07:01
 */ 
#include "user_json.h"
#include "config.h"

#if SET_PROTO_VER==PROTO_ZH202
void printt_json(const char *str,u16 len)
{
	u16 i;
    for(i = 0; i < len; i++)
    {
        LOG("%c",str[i]);
    }
    LOG("\r\n");
}


/**
 * @description: 自定义字符串转数字，(原atoi只能转化到0x7FFF FFFF)
 * @param {type} 
 * @return: 
 */
u32 user_atoi(char *str)
{
    u32 ret = 0;
    u32 digit = 1;//位数
	u16 i;
	LOG("len = %d\r\n",strlen(str));
    if(strlen(str) < 12)
    {//u32 小于12位数
        for (i = strlen(str) - 1; i != 0; i--)
        {
            ret = ret + (str[i] - '0')*digit;
            digit = digit * 10;
        }
		//第1字节
		ret = ret + (str[0] - '0')*digit;
    }
    return ret;
}

#if 0
/**
 * @description: 从中括号字符串中获取数据，
 * @param str:带[]的字符串，str后没有’\0‘,自行判断'['，']' 
 * @param strlen:str长度
 * @param dat_buf:获取的整数数组指针
 * @param max_len:dat_buf中最多放置的数组个数
 * @return: 返回获取到的数组个数
 */
u16 user_json_get_from_bracket(const char *str, u16 strlen, u8 *dat_buf, u8 max_len)
{
    
    char temp_buf[11];
    u8 temp_len = 0;
    u16 dat_num = 0;
    bool is_begin = false;
    bool is_get_num_bigin = false;
	u16 i;
    memset(temp_buf,0x00,sizeof(temp_buf));
    for(i = 0; i < strlen; i++)
    {
        if(str[i] == '[')
        {
            is_begin = true;
        }
        if(is_begin == true)
        {
            if (isdigit(str[i]))
            {
                is_get_num_bigin = true;
                temp_buf[(temp_len++) % 10] = str[i]; //int型最多十个字符数字
            }
            else
            {
                if(is_get_num_bigin == true)
                {
                    is_get_num_bigin = false;
                    if(dat_num < max_len)
                    {
                        dat_buf[dat_num++] = atoi(temp_buf);
                        memset(temp_buf,0x00,sizeof(temp_buf));
                        temp_len = 0;
                    }
                    else
                    {
                        return max_len;
                        // break;
                    }
                    
                }
            }
            
        }
        if(str[i] == ']')
        {
            return dat_num;
        }
    }
    return false;
}


/**
 * @description: 从中括号字符串中获取数据，
 * @param str:带[]的字符串，str后没有’\0‘,自行判断'['，']' 
 * @param strlen:str长度
 * @param dat_buf:获取的整数数组指针
 * @param max_len:dat_buf中最多放置的数组个数
 * @return: 返回获取到的数组个数
 */
u16 user_json_get_int_from_bracket(const char *str, u16 strlen, int *dat_buf, u8 max_len)
{
    char temp_buf[11];
    u8 temp_len = 0;
    u16 dat_num = 0;
    bool is_begin = false;
    bool is_get_num_bigin = false;

    memset(temp_buf,0x00,sizeof(temp_buf));
    for(int i = 0; i < strlen; i++)
    {
        if(str[i] == '[')
        {
            is_begin = true;
        }
        if(is_begin == true)
        {
            if (isdigit(str[i]))
            {
                is_get_num_bigin = true;
                temp_buf[(temp_len++) % 10] = str[i]; //int型最多十个字符数字
            }
            else
            {
                if(is_get_num_bigin == true)
                {
                    is_get_num_bigin = false;
                    if(dat_num < max_len)
                    {
                        dat_buf[dat_num++] = atoi(temp_buf);
                        memset(temp_buf,0x00,sizeof(temp_buf));
                        temp_len = 0;
                    }
                    else
                    {
                        return max_len;
                        // break;
                    }
                    
                }
            }
            
        }
        if(str[i] == ']')
        {
            return dat_num;
        }
    }
    return false;
}
    
#endif
/**
 * @description: 根据键获取一个字节的数值数值
 * @param str 传入的字符串，从中获取键值; 键带"”，例"\"o\" : xx"
 * @param strlen 传入的字符串长度
 * @param key 获取键的字符 例'o'
 * @param val 返回对应键的值
 * @return: 1：获取成功，0：在指定长度里没有找到键值
 */
bool user_json_get_val_from_key(const char *str, u8 strlen, char key, u8 *val)
{
    bool ret = false;
    bool is_queryed_key = false;//是否查到关键字了
    bool is_begin_num = false;//是否开始获取数字字符
    char temp_buf[20];
    u8 temp_len = 0;
	u16 i;
    memset(temp_buf,0x00,sizeof(temp_buf));

    for(i = 0; i < strlen; i++)
    {
        if(str[i] == '\"' && str[i + 1] == key && str[i + 2] == '\"')
        {
            is_queryed_key = true;
            i += 2;
        }
        else if(isdigit(str[i]) && is_queryed_key == true)
        {
            is_begin_num = true;
            temp_buf[(temp_len++)%10] = str[i];//int型最多十个字符数字
        }
        else if(is_queryed_key == true && is_begin_num == true && isdigit(str[i]) == false)
        {
            *val = atoi(temp_buf);
			
            ret = true;
            break;
        }
    }
    return ret;
}


/**
 * @description: 根据键获取int值
 * @param {type} 
 * @return: 
 */
bool user_json_get_int_from_key(const char *str, u8 strlen, char key, u32 *val)
{
    bool ret = false;
    bool is_queryed_key = false;//是否查到关键字了
    bool is_begin_num = false;//是否开始获取数字字符
    char temp_buf[20];
	u8 i;
    u8 temp_len = 0;
    memset(temp_buf,0x00,sizeof(temp_buf));

    for(i = 0; i < strlen; i++)
    {
        if(str[i] == '\"' && str[i + 1] == key && str[i + 2] == '\"')
        {
            is_queryed_key = true;
            i += 2;
        }
        if(isdigit(str[i]) && is_queryed_key == true)
        {
            is_begin_num = true;
            temp_buf[(temp_len++)%12] = str[i];//int型最多十个字符数字
        }
        else if(is_queryed_key == true && is_begin_num == true && isdigit(str[i]) == false)
        {
            //*val = atoi(temp_buf);
            *val = user_atoi(temp_buf);		
            ret = true;
            break;
        }
    }
    return ret;
}


/**
 * @description: 判别字符串中是否是json数据
 * @param {type} 
 * @return: 
 */
bool user_json_is_json(const char *str, u16 strlen)
{
    u16 bracket1 = 0;//'{'个数
    u16 bracket1_1 = 0;//'}'个数
    u16 bracket2 = 0;//'['个数
    u16 bracket2_2 = 0;//']'个数
	u16 i;
    for(i = 0; i < strlen; i++)
    {
        if(str[i] == '{')
        {
            bracket1++;
        }   
        else if(str[i] == '}')
        {
            bracket1_1++;
        } 
        else if(str[i] == '[')
        {
            bracket2++;
        }
        else if(str[i] == ']')
        {
            bracket2_2++;
        }
    }
    
    if(bracket1 == bracket1_1 && bracket2 == bracket2_2)
    {
        return true;
    }
    else
    {
        return false;
    }
    
}

#if 0
/**
 * @description: 从中括号字符串中获取数据，
 * @param 
 * @return: 返回键值对应数据个数
 */
u16 user_json_get_array_from_key(const char *str, u16 strlen, u8 *dat_buf, u8 max_len, char key)
{
    u16 len = 0;
    if(strlen <= 4) return len;
    for(int i = 0; i < strlen - 2; i++)
    {
        if(str[i] == '\"' && str[i + 1] == key && str[i + 2] == '\"')
        {
            if(strncmp(&str[i + 4],"null",4) == 0 || strncmp(&str[i + 5],"null",4) == 0 || \
            strncmp(&str[i + 6],"null",4) == 0)
            {//如果键值为空则直接返回
                return 0;
            }
            len = user_json_get_from_bracket(&str[i + 3], strlen - i - 3, dat_buf, max_len);
            break;
        }
    }
    return len;
}


/**
 * @description: 从中括号字符串中获取数据，
 * @param 
 * @return: 返回键值对应数据个数
 */
u16 user_json_get_int_array_from_key(const char *str, u16 strlen, int *dat_buf, u8 max_len, char key)
{
    u16 len = 0;
	u16 i;
    if(strlen <= 4) return len;
    for(i = 0; i < strlen - 2; i++)
    {
        if(str[i] == '\"' && str[i + 1] == key && str[i + 2] == '\"')
        {
            len = user_json_get_int_from_bracket(&str[i + 3], strlen - i - 3, dat_buf, max_len);
            break;
        }
    }
    return len;
}
#endif

/**
 * @description: 往json缓冲中加如json结构键值字符串
 * @param json_site json中键值位置 首位需要加'{''}'，
 * @return: 
 */
u16 user_json_encode_key_val(char *json, char key, u16 val, JSON_SITE_TYPE json_site)
{
    u16 len = 0;
    char num_buf[10];
    memset(num_buf,0x00,sizeof(num_buf));
    if(json_site == JSON_SITE_FIRST)
    {//首个键值
        json[len++] = '{';
    }
    json[len++] = '\"';
    json[len++] = key;
    json[len++] = '\"';
    json[len++] = ':';
    sprintf(num_buf,"%d",val);
    memcpy(&json[len],num_buf,strlen(num_buf));
    len += strlen(num_buf);
    if(json_site != JSON_SITE_LAST)
    {
        json[len++] = ',';
    }
    else
    {
        json[len++] = '}';
    }
    return len;
}

/**
 * @description: 往json缓冲中加如json结构键 数组 字符串
 * @param json_site json中键值位置 首位需要加'{''}'，
 * @param proc_cb 数组转化回调函数
 * @return: 
 */
u16 user_json_encode_key_array(char *json, char key, u8 *array, u8 array_len, JSON_SITE_TYPE json_site, u8(*proc_cb)(u8))
{
    u16 len = 0;
    char num_buf[10];
	u16 i;
    memset(num_buf,0x00,sizeof(num_buf));
    if(json_site == JSON_SITE_FIRST)
    {//首个键值
        json[len++] = '{';
    }
    json[len++] = '\"';
    json[len++] = key;
    json[len++] = '\"';
    json[len++] = ':';
    json[len++] = '[';
	
    for(i = 0; i < array_len; i++)
    {
        if(proc_cb == NULL)
        {//不需要转化数据
            memset(num_buf, 0x00, sizeof(num_buf));
            if(i == array_len - 1)
            {
                sprintf(num_buf, "%d", (u16)array[i]);
            }
            else
            {
                sprintf(num_buf, "%d,", (u16)array[i]);
            }
            memcpy(&json[len], num_buf, strlen(num_buf));
            len += strlen(num_buf);
        }
        else
        {//需要转化数据，主要用来查设备状态
            memset(num_buf, 0x00, sizeof(num_buf));
            if(i == array_len - 1)
            {
                sprintf(num_buf, "%d", proc_cb(array[i]));
            }
            else
            {
                sprintf(num_buf, "%d,", proc_cb(array[i]));
            }
            memcpy(&json[len], num_buf, strlen(num_buf));
            len += strlen(num_buf);
        }
        
    }
    json[len++] = ']';
    if(json_site != JSON_SITE_LAST)
    {
        json[len++] = ',';
    }
    else
    {
        json[len++] = '}';
    }
    
    return len;
}


/**
 * @description: 往json缓冲中加如json结构键值字符串
 * @param is_add_escape str是否添加转义字符'\""'
 * @return: 
 */
u16 user_json_encode_key_string(char *json, char key, char *str, bool is_add_escape,JSON_SITE_TYPE json_site)
{
    u16 len = 0;
    if(json_site == JSON_SITE_FIRST)
    {//首个键值
        json[len++] = '{';
    }
    json[len++] = '\"';
    json[len++] = key;
    json[len++] = '\"';
    json[len++] = ':';
    if(is_add_escape)
    {//需要添加转义字符
        json[len++] = '\"';
    }
    memcpy(&json[len],str,strlen(str));
    len += strlen(str);
    
    if(is_add_escape)
    {//需要添加转义字符
        json[len++] = '\"';
    }
    if(json_site != JSON_SITE_LAST)
    {
        json[len++] = ',';
    }
    else
    {
        json[len++] = '}';
    }
    return len;
}


/**
 * @description: 往json缓冲中加如json结构键 数组 字符串
 * @param json_site json中键值位置 首位需要加'{''}'，
 * @param proc_cb 数组转化回调函数
 * @param key 字符串
 * @return: 
 */
u16 user_json_encode_mul_key_u16_array(char *json, char *key, u32 *array, u8 array_len, JSON_SITE_TYPE json_site, u16(*proc_cb)(u16))
{
    u16 len = 0;
	u16 i;
    char num_buf[15];
    memset(num_buf,0x00,sizeof(num_buf));
    if(json_site == JSON_SITE_FIRST)
    {//首个键值
        json[len++] = '{';
    }
    json[len++] = '\"';
    memcpy(&json[len],key,strlen(key));
    len += strlen(key);
    json[len++] = '\"';
    json[len++] = ':';
    json[len++] = '[';

    for(i = 0; i < array_len; i++)
    {
        if(proc_cb == NULL)
        {//不需要转化数据
            memset(num_buf, 0x00, sizeof(num_buf));
            if(i == array_len - 1)
            {
                sprintf(num_buf, "%d", (u16)array[i]);
            }
            else
            {
                sprintf(num_buf, "%d,", (u16)array[i]);
            }
            memcpy(&json[len], num_buf, strlen(num_buf));
            len += strlen(num_buf);
        }
        else
        {//需要转化数据，主要用来查设备状态
            memset(num_buf, 0x00, sizeof(num_buf));
            if(i == array_len - 1)
            {
                sprintf(num_buf, "%d", proc_cb(array[i]));
            }
            else
            {
                sprintf(num_buf, "%d,", proc_cb(array[i]));
            }
            memcpy(&json[len], num_buf, strlen(num_buf));
            len += strlen(num_buf);
        }
        
    }
    json[len++] = ']';
    if(json_site != JSON_SITE_LAST)
    {
        json[len++] = ',';
    }
    else
    {
        json[len++] = '}';
    }
    
    return len;
}
#endif


