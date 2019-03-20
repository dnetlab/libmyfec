/*
 * fec.h
 *
 *  Created on: Nov 26, 2018
 *      Author: pp
 */

#ifndef _MYFEC_H_
#define _MYFEC_H_

#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "sfxhash.h"

#define MAX_MYFEC_BUF_LEN		(1500)
#define MAX_MYFEC_RECV_NUM	(1000)

#define MAX_MYFEC_GROUP_NUM	(20)
#define MAX_MYFEC_BUF_NUM		(200)

#define MAX_MYFEC_ENCODE_NUM	(100)
#define MIN_MYFEC_ENCODE_NUM	(1)
#define MAX_MYFEC_ENCODE_LEN	(1500)

#define MAX_MYFEC_DECODE_NUM	(100)

#define MAX_MYFEC_FECLEN	(1450)
#define MIN_MYFEC_FECLEN	(40)

#define MAX_MYFEC_X	(20)
#define MIN_MYFEC_X	(1)

//key type is group_key_t, value type is myfec_group_t
typedef struct group_key
{
	int seq;
}group_key_t;

//key type is buf_key_t, value type is mybuf_data_t
typedef struct buf_key
{
	int index;
}buf_key_t;

typedef struct myfec_ctx myfec_ctx_t;

typedef struct group_data
{
	int	seq;
	int x;
	int y;
	int len;


	int	cnt;
	time_t last_time;
	SFXHASH* data_map;
	myfec_ctx_t* ctx;
}group_data_t;

typedef struct already_group_data
{
	int	seq;
	time_t last_time;
	myfec_ctx_t* ctx;
	int should_cnt;
	int already_cnt;
}already_group_data_t;

typedef struct buf_data
{
	int	seq;
	int x;
	int y;
	int	inner_index;
	int len;
	int buf_index;
}buf_data_t;

typedef struct packet_data
{
	int seq;
	int used;
	char buf[MAX_MYFEC_BUF_LEN];
}packet_data_t;

typedef struct encode_buf
{
	char buf[MAX_MYFEC_ENCODE_NUM * MAX_MYFEC_ENCODE_LEN];
	int cnt;
	int buf_len;
}encode_buf_t;

typedef struct decode_buf
{
	char *buf_ptr;
	int buf_len;
}decode_buf_t;

//average len + 4	+2		+ 2					+ 2
//fec each packet, because we need save memory, and reduce ping latency
struct myfec_ctx {
	SFXHASH* group_map;
	SFXHASH* already_group_map;
	time_t	time_window;
	//void* user_data;	//only store the pointer, be careful if user_data in stack
	//int	(*user_decode_send_cb)(void* user_data, char* send_buf, int send_len);	//after decode all packets from recved packets, should

	//char* decoded_ptr[MAX_MYFEC_ENCODE_NUM];
	int				re_num;
	int				max_fec_len;
	int				max_en_cnt;
	int				max_fec_x;

	encode_buf_t	en_buf;		//used for encode
	//char*			en_ptr[MAX_MYFEC_ENCODE_NUM];
	int				en_x;								//the fec actual data segments count
	int				en_y;								//the fec REDUNDANT data segments count
	int				en_feclen;
	int				en_headerlen;
	char			end_buf[MAX_MYFEC_ENCODE_NUM * 2 * (MAX_MYFEC_ENCODE_LEN + 20)];	//to store the encoded packets to send out

	decode_buf_t	de_buf[MAX_MYFEC_DECODE_NUM];	//if decode success, user should use de_buf and de_cnt to handle each packet
	int				de_cnt;
	uint32_t		en_seq;
	packet_data_t*	buf;		//used for decode
	int buf_cnt;
	int cur_buf_id;

	int received_cnt;
	int should_receive_cnt;
};

void myfec_init(myfec_ctx_t* ctx, int max_en_cnt, int re_num, int max_fec_len, int max_fec_x);
void myfec_exit(myfec_ctx_t* ctx);

int myfec_encode_input(myfec_ctx_t* ctx, char* in_buf, int in_len);
int myfec_encode_output(myfec_ctx_t* ctx);

int myfec_decode(myfec_ctx_t* ctx, char *src, int src_len);	//return decode len, dst_len should be larger than ACTUAL_MTU

int myfec_cal_packet_lossy(myfec_ctx_t* ctx);
int myfec_reset_packet_lossy(myfec_ctx_t* ctx);
void myfec_set_re_num(myfec_ctx_t* ctx, int re_num);

#endif /* _MYFEC_H_ */
