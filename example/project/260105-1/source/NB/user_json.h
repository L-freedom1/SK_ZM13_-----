/*
 * @Author: dujianhuang
 * @Date: 2020-06-12 11:26:43
 * @LastEditors: error: git config user.name && git config user.email & please set dead value or install git
 * @LastEditTime: 2022-11-25 15:35:13
 */ 


#ifndef __USER_JSON_H
#define __USER_JSON_H

#include 	<string.h>
#include 	<stdlib.h>
#include 	"stdio.h"
//#include <stdbool.h>
#include "config.h"

/* json ¼üÖµÎ»ÖÃ */
typedef enum
{
    JSON_SITE_FIRST,
    JSON_SITE_LAST,
    JSON_SITE_NO_FIRST_LAST
}JSON_SITE_TYPE;


extern u16 user_json_get_from_bracket(const char *str, u16 strlen, u8 *dat_buf, u8 max_len);
extern bool user_json_get_val_from_key(const char *str, u8 strlen, char key, u8 *val);
extern bool user_json_is_json(const char *str, u16 strlen);
extern bool user_json_get_int_from_key(const char *str, u8 strlen, char key, u32 *val);
extern u16 user_json_get_array_from_key(const char *str, u16 strlen, u8 *dat_buf, u8 max_len, char key);
extern u16 user_json_get_int_array_from_key(const char *str, u16 strlen, int *dat_buf, u8 max_len, char key);

extern u16 user_json_encode_key_val(char *json, char key, u16 val, JSON_SITE_TYPE json_site);
extern u16 user_json_encode_key_array(char *json, char key, u8 *array, u8 array_len, JSON_SITE_TYPE json_site, u8(*proc_cb)(u8));
extern u16 user_json_encode_key_string(char *json, char key, char *str, bool is_add_escape,JSON_SITE_TYPE json_site);
extern void printt_json(const char *str,u16 len);
extern u16 user_json_encode_mul_key_u16_array(char *json, char *key, u32 *array, u8 array_len, JSON_SITE_TYPE json_site, u16(*proc_cb)(u16));
#endif

