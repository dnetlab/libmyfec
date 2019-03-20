/*
 * myfec.c
 *
 *  Created on: Nov 26, 2018
 *      Author: pp
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "mycommon.h"
#include "rs.h"
#include "myfec.h"
#include "sfxhash.h"
#include "my_debug.h"

#define PACKET_TYPE_FEC		(0)
#define PACKET_TYPE_ALIVE		(1)
#define PACKET_TYPE_FEEDBACK		(2)

void group_recycle_all_data(group_data_t* g_data, myfec_ctx_t* ctx);
group_data_t* group_find(SFXHASH* group_map, int seq);
group_data_t* group_new(SFXHASH* group_map, myfec_ctx_t* ctx, int seq, int x, int y, int fec_len);
group_data_t* group_add_data(SFXHASH* group_map, myfec_ctx_t* ctx, int seq, int x, int y, int index, int fec_len, char* data, int* need_fec);
void group_init(group_data_t* data, myfec_ctx_t* ctx, int seq, int x, int y, int fec_len);
void group_exit(group_data_t* data);
int buf_map_free(void* key, void* value);
int group_map_free(void* key, void* value);
u32_t next_seq(u32_t a);
int check_already_recved(myfec_ctx_t* ctx, int seq);
int make_already_recved(myfec_ctx_t* ctx, int seq, int should_cnt, int already_cnt);
void aging_already_group(myfec_ctx_t* ctx, int age_time);
int next_buf_id(myfec_ctx_t* ctx);

int myfec_cal_packet_lossy(myfec_ctx_t* ctx)
{
	if (ctx && ctx->should_receive_cnt > 0)
	{
		return (ctx->should_receive_cnt - ctx->received_cnt) * 100 / ctx->should_receive_cnt;
	}
	else
	{
		return 10;
	}
}

int myfec_reset_packet_lossy(myfec_ctx_t* ctx)
{
	ctx->should_receive_cnt = 0;
	ctx->received_cnt = 0;
}

void myfec_set_re_num(myfec_ctx_t* ctx, int re_num)
{
	ctx->re_num = re_num;
}

group_data_t* group_find(SFXHASH* group_map, int seq)
{
	group_key_t key;
	key.seq = seq;
	group_data_t* ret = sfxhash_find(group_map, &key);
	return ret;
}

group_data_t* group_new(SFXHASH* group_map, myfec_ctx_t* ctx, int seq, int x, int y, int fec_len)
{
	group_key_t key;
	key.seq = seq;
	group_data_t new_data;
	group_init(&new_data, ctx, seq, x, y, fec_len);
	sfxhash_add(group_map, &key, &new_data);
	return (group_data_t*)group_find(group_map, seq);
}

//return 0: no need fec, 1: need fec now, -1: error happends
group_data_t* group_add_data(SFXHASH* group_map, myfec_ctx_t* ctx, int seq, int x, int y, int index, int fec_len, char* data, int* need_fec)
{
	*need_fec = 0;
	//MY_DEBUG_INFO("=== group_add_data for seq:%d\n", seq);
	group_data_t* g_data = group_find(group_map, seq);
	if (!g_data)
	{
		MY_DEBUG_INFO("=== new group for seq:%d\n", seq);
		g_data = group_new(group_map, ctx, seq, x, y, fec_len);
		//MY_DEBUG_INFO("=== new group for seq:%d ok\n", g_data->seq);
	}

	if (g_data)
	{
		buf_key_t buf_key;
		buf_key.index = index;
		buf_data_t* buf_node = (buf_data_t*)sfxhash_find(g_data->data_map, &buf_key);
		if (buf_node)
		{
			//the packet already in, just ignore the current
		}
		else
		{
			packet_data_t *cur_buf = ctx->buf + ctx->cur_buf_id;
			cur_buf->used = 1;
			cur_buf->seq = seq;
			memcpy(cur_buf->buf, data, fec_len);
			buf_data_t buf_data;
			buf_data.x = g_data->x;
			buf_data.y = g_data->y;
			buf_data.len = fec_len;
			buf_data.seq = g_data->seq;
			buf_data.inner_index = index;
			buf_data.buf_index = ctx->cur_buf_id;
			sfxhash_add(g_data->data_map, (void*)&buf_key, (void*)&buf_data);
			g_data->cnt++;
			if (g_data->cnt == g_data->x)
			{
				*need_fec = 1;
			}
			ctx->cur_buf_id = next_buf_id(ctx);
		}
	}
	return g_data;
}

/*
void group_del_data(SFXHASH* group_map, int seq, int index)
{

}
*/

void group_del(SFXHASH* group_map, int seq)
{
	group_key_t key;
	key.seq = seq;
	group_data_t* del_node = sfxhash_find(group_map, &key);
	if (del_node)
	{
		sfxhash_remove(group_map, &key);
	}
}

void group_recycle_all_data(group_data_t* g_data, myfec_ctx_t* ctx)
{
	if (g_data)
	{
		if (g_data->data_map)
		{
			SFXHASH_NODE* cur = sfxhash_ghead(g_data->data_map);
			while(cur)
			{
				buf_key_t* key = (buf_key_t*)cur->key;
				ctx->buf[key->index].used = 0;
				cur = sfxhash_gnext(cur);
			}
		}
	}
}

void group_init(group_data_t* data, myfec_ctx_t* ctx, int seq, int x, int y, int fec_len)
{
	data->x = x;
	data->y = y;
	data->len = fec_len;
	data->data_map =	sfxhash_new(20,        /* one row per element in table, when possible */
			sizeof(buf_key_t),        /* key size :  padded with zeros */
			sizeof(buf_data_t),        /* data size:  padded with zeros */
			0,       /* max bytes,  0=no max */
			0,         /* enable AutoNodeRecovery */
			NULL,   /* provide a function to let user know we want to kill a node */
			NULL, /* provide a function to release user memory */
			1);
	sfxhash_splaymode(data->data_map, 1);
	data->seq = seq;
	data->ctx =	ctx;
	data->cnt = 0;
	data->last_time = time(NULL);
}

//delete the group from group_map
void group_exit(group_data_t* data)
{
	if (data->data_map)
	{
		group_recycle_all_data(data, data->ctx);
		sfxhash_delete(data->data_map);
		data->data_map = NULL;
	}
}

int buf_map_free(void* key, void* value)
{
	return 0;
}

//TODO: need free data_map in group_data_t
int group_map_free(void* key, void* value)
{
	//group_key_t* g_key = (group_key_t*)key;
	group_data_t* g_data = (group_data_t*)value;
	group_exit(g_data);
	return 0;
}

//return 0:a = b
//return 1:a > b
//return -1:a < b
u32_t next_seq(u32_t a)
{
	u32_t ret = 0;
	if (a == 0xffffffffUL)
	{
		ret = 1;
	}
	else
	{
		ret = a+1;
	}
	return ret;
}

static void dump_data(unsigned char* data, int data_len)
{
	int i;
	MY_DEBUG_INFO("<");
	for(i = 0; i < data_len; i++)
	{
		MY_DEBUG_INFO("%02x ", data[i]);
	}
	MY_DEBUG_INFO(">\n");
	return;
}

int check_already_recved(myfec_ctx_t* ctx, int seq)
{
	int ret = 0;
	group_key_t key;
	key.seq = seq;
	already_group_data_t* data = (already_group_data_t*)sfxhash_find_node(ctx->already_group_map, (void*)&key);
	if (data)
	{
		if (data->already_cnt < data->should_cnt)
		{
			data->already_cnt++;
		}
		ret = 1;
	}
	return ret;
}

int make_already_recved(myfec_ctx_t* ctx, int seq, int should_cnt, int already_cnt)
{
	int ret = 0;
	group_key_t key;
	key.seq = seq;
	group_data_t* data = group_find(ctx->group_map, key.seq);
	if (data)
	{
		ret = 1;
		group_exit(data);
		sfxhash_delete(data->data_map);
		data->data_map = NULL;
		already_group_data_t already_group;
		already_group.last_time = time(NULL);//data->last_time;
		already_group.seq = seq;
		already_group.already_cnt = already_cnt;
		already_group.should_cnt = should_cnt;
		MY_DEBUG_INFO("==== add seq : %d to already_group_map\n", seq);
		sfxhash_add(ctx->already_group_map, (void*)&key, (void*)&already_group);
	}
	return ret;
}

void aging_already_group(myfec_ctx_t* ctx, int age_time)
{
	SFXHASH_NODE* tail = sfxhash_gtail(ctx->already_group_map);
	if (tail)
	{
		already_group_data_t* ag_data = (already_group_data_t*)tail->data;
		time_t cur_time = time(NULL);
		if (ag_data->last_time + age_time < cur_time)
		{
			ctx->should_receive_cnt += ag_data->should_cnt;
			ctx->received_cnt += ag_data->already_cnt;
			sfxhash_free_node(ctx->already_group_map, tail);
		}
	}
}

int next_buf_id(myfec_ctx_t* ctx)
{
	int ret = (ctx->cur_buf_id + 1)%ctx->buf_cnt;
	return ret;
}

void decode_index_init(int x, int y, group_data_t* g_data, char *index_buf[], int data_offset)
{
	int i;
	for(i = 0; i < x + y; i++)
	{
		index_buf[i] = NULL;
	}
	if (g_data)
	{
		if (g_data->data_map)
		{
			SFXHASH_NODE* cur = sfxhash_ghead(g_data->data_map);
			while(cur)
			{
				buf_key_t* key = (buf_key_t*)cur->key;
				//ctx->buf[key->index].used = 0;
				//index_buf[key->index] = g_data->ctx->buf[key->index].buf + data_offset;
				MY_DEBUG_INFO("==== found index %d\n", key->index);

				buf_data_t* cur_buf = (buf_data_t*)cur->data;
				int buf_index = cur_buf->buf_index;
				index_buf[key->index] = g_data->ctx->buf[buf_index].buf;
				cur = sfxhash_gnext(cur);
			}
		}
	}
	return;
}

void decode_buf_merge(int x, int y, int fec_len, char *index_buf[], char* out_buf)
{
	int i;
	for(i = 0; i < x; i++)
	{
		memcpy(out_buf + fec_len * i, index_buf[i], fec_len);
	}
}

//return : <0 ,received a wrong packet. 0: type no need to decode 1: decode success
int myfec_decode(myfec_ctx_t* ctx, char *src, int src_len)
{
	int ret = -1;
	int tmp_index = 0;
	//the first 2 bytes to indicate the packet type
	uint16_t packet_type = read_u16(src + tmp_index);
	tmp_index += sizeof(packet_type);
	if (packet_type == PACKET_TYPE_FEEDBACK)
	{
		return 0;
	}
	else if (packet_type == PACKET_TYPE_ALIVE)
	{
		return 0;
	}

	u32_t seq = read_u32(src + tmp_index);
	tmp_index += 4;
	u16_t fec_num = read_u16(src + tmp_index);
	tmp_index += 2;
	u16_t fec_redundant_num = read_u16(src + tmp_index);
	tmp_index += 2;
	u16_t fec_cur_index = read_u16(src + tmp_index);
	tmp_index += 2;
	u16_t fec_len = read_u16(src + tmp_index);
	tmp_index += 2;
	//int actual_cnt = 0;
	MY_DEBUG_INFO("recved : <seq = %d> <fec_num = %d> <fec_redundant_num = %d> <cur_index = %d> <fec_len = %d>\n", seq, (int)fec_num, (int)fec_redundant_num, (int)fec_cur_index,
			(int)fec_len);
	assert(seq != 0);

	if (!check_already_recved(ctx, seq))
	{
		//if the data buf already used by another group, need to free the group
		packet_data_t* last_buf = ctx->buf + ctx->cur_buf_id;
		if (last_buf->used)
		{
			group_del(ctx->group_map, last_buf->seq);
		}
		int need_fec = 0;
		group_data_t* g_data = group_add_data(ctx->group_map, ctx, seq, (int)fec_num, (int)fec_redundant_num, (int)fec_cur_index, (int)fec_len, src + tmp_index, &need_fec);
		if (g_data)
		{
			if (need_fec == 1)
			{
				MY_DEBUG_INFO("fec now\n");
				//fec decode
				static char *fec_buf[MAX_MYFEC_BUF_NUM];
				static char fec_decode_buf[MAX_MYFEC_BUF_NUM * MAX_MYFEC_BUF_LEN];
				decode_index_init(fec_num, fec_redundant_num, g_data, fec_buf, tmp_index);
				MY_DEBUG_INFO("fec before for <seq:%d>\n", seq);
				//dump_data(fec_buf[0], (int)fec_len);
				int decode_ret = rs_decode2(fec_num, fec_num + fec_redundant_num, fec_buf, fec_len);
				MY_DEBUG_INFO("fec after for <seq:%d>\n", seq);
				dump_data(fec_buf[0], (int)fec_len);
				if (decode_ret == 0)
				{
					decode_buf_merge((int)fec_num, (int)fec_redundant_num, (int)fec_len, fec_buf, fec_decode_buf);
					//TODO:decode all packets from buf, and call the user set callback send
					uint16_t i;
					uint16_t packet_cnt = read_u16(fec_decode_buf);
					MY_DEBUG_INFO("packet_cnt = %d\n", packet_cnt);
					char* packet = fec_decode_buf + sizeof(uint16_t);
					for(i = 0; i < packet_cnt; i++)
					{
						uint16_t packet_len = read_u16(packet);
						MY_DEBUG_INFO("<packet:%d> <size:%d>\n", (int)i, (int)packet_len);
						//send out the packet
						//may write to tun fd
						//or forward to another node
						packet += sizeof(packet_len);
						ctx->de_buf[i].buf_ptr = packet;
						ctx->de_buf[i].buf_len = (int)packet_len;
						//ctx->user_decode_send_cb(ctx->user_data, packet, (int)packet_len);
						packet += packet_len;
					}
					ctx->de_cnt = (int)packet_cnt;
					ret = 1;
				}

				//TODO: after fec decode, should add the seq into already_group_map
				MY_DEBUG_INFO("=== make already for seq:%d\n", seq);
				make_already_recved(ctx, seq, fec_num + fec_redundant_num, fec_num);
				aging_already_group(ctx, 3);
				//TODO: after fec decode, auto recycle the packet buffers used by freeing a node from group_map
				group_key_t g_key;
				g_key.seq = seq;
				sfxhash_remove(ctx->group_map, (void*)&g_key);
			}
		}
	}

	//TODO: each packet should trigger the process that checking the already_group_map, to remove oldest seq that expired
	return ret;
}

int is_encode_buf_full(myfec_ctx_t* ctx, char* in_buf, int in_len)
{
	int ret = 0;
	int old_len = ctx->en_buf.buf_len;
	if (ctx->en_buf.cnt >= ctx->max_en_cnt - 1
			||
			(old_len + in_len + MAX_MYFEC_ENCODE_LEN >= MAX_MYFEC_ENCODE_LEN * MAX_MYFEC_ENCODE_NUM)
			|| (ctx->en_buf.cnt >= ctx->max_fec_x - 1)
			)	//We must reserve the buff for the next packet
	{
		ret = 1;
	}
	return ret;
}

void encode_buf_init(myfec_ctx_t* ctx)
{
	//the first 2 bytes is the packet cnt
	ctx->en_buf.cnt = 0;
	ctx->en_buf.buf_len = 2;
}

void encode_buf_add(myfec_ctx_t* ctx, char* data, int data_len)
{
	uint16_t len = (uint16_t)data_len;
	write_u16(ctx->en_buf.buf + ctx->en_buf.buf_len, len);
	memcpy(ctx->en_buf.buf + ctx->en_buf.buf_len + sizeof(len), data, data_len);
	ctx->en_buf.buf_len += (data_len + sizeof(len));
	ctx->en_buf.cnt++;
	write_u16(ctx->en_buf.buf, (uint16_t)ctx->en_buf.cnt);
}

//return 0: there is still space to store the next packet
//return 1: there is no space to store the next packet, need fec encode now
int myfec_encode_input(myfec_ctx_t* ctx, char* in_buf, int in_len)
{
	int ret = 0;
	if (is_encode_buf_full(ctx, in_buf, in_len))
	{
		ret = 1;
	}
	encode_buf_add(ctx, in_buf, in_len);
	return ret;
}

int round_up_div(int num, int div_num)
{
	int ret;
	ret = (num + div_num - 1)/div_num;
	return ret;
}

//return :0 no need encode
//return :1 encode ok
int myfec_encode_output(myfec_ctx_t* ctx)
{
	int ret = 0;
	if (ctx->en_buf.cnt == 0)
	{
		return ret;
	}
	//start encode
	//calculate the suitable x:y
	//calculate the suitable x
	int x =	round_up_div(ctx->en_buf.buf_len, ctx->max_fec_len);
	//calculate the real fec len;
	int fec_len = round_up_div(ctx->en_buf.buf_len, x);
	int y = ctx->re_num;

	MY_DEBUG_INFO("buf len = %d, max_fec_len = %d\n", ctx->en_buf.buf_len, ctx->max_fec_len);
	MY_DEBUG_INFO("x = %d, y =%d\n", x, y);
	ctx->en_x = x;
	ctx->en_y = y;
	ctx->en_feclen = fec_len;
	ctx->en_seq++;

	char *en_ptr[MAX_MYFEC_ENCODE_NUM];

	int i;
	int offset = 0;
	int tmp_idx = 0;
	//generate actual data segments
	for(i = 0; i < x; i++)
	{
		tmp_idx = 0;
		char* packet = ctx->end_buf + offset;
		//write type
		uint16_t type = PACKET_TYPE_FEC;
		write_u16(packet + tmp_idx, type);
		tmp_idx += sizeof(type);

		//write seq
		uint32_t seq = (uint32_t)ctx->en_seq;
		write_u32(packet + tmp_idx, seq);
		tmp_idx += sizeof(seq);

		//write x
		uint16_t x = (uint16_t)ctx->en_x;
		write_u16(packet + tmp_idx, x);
		tmp_idx += sizeof(x);

		//write y
		uint16_t y = (uint16_t)ctx->en_y;
		write_u16(packet + tmp_idx, y);
		tmp_idx += sizeof(y);

		//write index
		uint16_t index = (uint16_t)i;
		write_u16(packet + tmp_idx, index);
		tmp_idx += sizeof(index);

		//write fec len
		uint16_t fec_len = (uint16_t)ctx->en_feclen;
		write_u16(packet + tmp_idx, fec_len);
		tmp_idx += sizeof(fec_len);

		//copy data
		memcpy(packet + tmp_idx, ctx->en_buf.buf + (ctx->en_feclen * i), ctx->en_feclen);

		en_ptr[i] = packet + tmp_idx;

		offset += (ctx->en_feclen + tmp_idx);
	}

	//generate REDUNDANT data segments
	for(i = 0; i < y; i++)
	{
		tmp_idx = 0;
		char* packet = ctx->end_buf + offset;
		//write type
		uint16_t type = PACKET_TYPE_FEC;
		write_u16(packet + tmp_idx, type);
		tmp_idx += sizeof(type);

		//write seq
		uint32_t seq = (uint32_t)ctx->en_seq;
		write_u32(packet + tmp_idx, seq);
		tmp_idx += sizeof(seq);

		//write x
		uint16_t x = (uint16_t)ctx->en_x;
		write_u16(packet + tmp_idx, x);
		tmp_idx += sizeof(x);

		//write y
		uint16_t y = (uint16_t)ctx->en_y;
		write_u16(packet + tmp_idx, y);
		tmp_idx += sizeof(y);

		//write index
		uint16_t index = (uint16_t)(i + x);
		write_u16(packet + tmp_idx, index);
		tmp_idx += sizeof(index);

		//write fec len
		uint16_t fec_len = (uint16_t)ctx->en_feclen;
		write_u16(packet + tmp_idx, fec_len);
		tmp_idx += sizeof(fec_len);

		//copy data
		//memcpy(packet + tmp_idx, ctx->en_buf.buf + (ctx->en_feclen * (i + x)), ctx->en_feclen);

		en_ptr[i + x] = packet + tmp_idx;
		offset += (ctx->en_feclen + tmp_idx);
	}
	ctx->en_headerlen = tmp_idx;
	rs_encode2(x, x + y, en_ptr, ctx->en_feclen);
	encode_buf_init(ctx);
	ret = 1;
	return ret;
}

void myfec_adjust_re_num(myfec_ctx_t* ctx, int re_num)
{
	if (ctx)
		ctx->re_num = re_num;
}

void myfec_init(myfec_ctx_t* ctx, int max_en_cnt, int re_num, int max_fec_len, int max_fex_x)
{
	ctx->group_map =     /* Create a Hash Table */
			sfxhash_new(100,        /* one row per element in table, when possible */
							sizeof(group_key_t),        /* key size :  padded with zeros */
							sizeof(group_data_t),        /* data size:  padded with zeros */
							0,       /* max bytes,  0=no max */
							0,         /* enable AutoNodeRecovery */
							group_map_free,   /* provide a function to let user know we want to kill a node */
							group_map_free, /* provide a function to release user memory */
							1);
	sfxhash_splaymode(ctx->group_map, 1);
	ctx->buf = (packet_data_t*)calloc(MAX_MYFEC_BUF_NUM, sizeof(packet_data_t));
	ctx->buf_cnt = MAX_MYFEC_BUF_NUM;

	ctx->already_group_map =	/* Create a Hash Table */
			sfxhash_new(100,        /* one row per element in table, when possible */
							sizeof(group_key_t),        /* key size :  padded with zeros */
							sizeof(already_group_data_t),        /* data size:  padded with zeros */
							0,       /* max bytes,  0=no max */
							0,         /* enable AutoNodeRecovery */
							NULL,   /* provide a function to let user know we want to kill a node */
							NULL, /* provide a function to release user memory */
							1);
	sfxhash_splaymode(ctx->already_group_map, 1);
	ctx->re_num = re_num;
	if (max_fec_len >= MIN_MYFEC_FECLEN && max_fec_len <= MAX_MYFEC_FECLEN)
	{
		ctx->max_fec_len = max_fec_len;
	}
	else
	{
		ctx->max_fec_len = MAX_MYFEC_FECLEN;
	}

	if (max_fex_x <= MAX_MYFEC_X && max_fex_x >= MIN_MYFEC_X)
	{
		ctx->max_fec_x = max_fex_x;
	}
	else
	{
		ctx->max_fec_x = MAX_MYFEC_X;
	}

	if (max_en_cnt >= MIN_MYFEC_ENCODE_NUM && max_en_cnt <= MAX_MYFEC_ENCODE_NUM)
	{
		ctx->max_en_cnt = max_en_cnt;
	}
	else
	{
		ctx->max_fec_len = MAX_MYFEC_ENCODE_NUM;
	}
	ctx->en_seq = 0;
	encode_buf_init(ctx);
}

void myfec_exit(myfec_ctx_t* ctx)
{
	if (ctx->group_map)
	{
		sfxhash_delete(ctx->group_map);
		ctx->group_map = NULL;
	}
	if (ctx->buf)
	{
		free(ctx->buf);
		ctx->buf = NULL;
		ctx->buf_cnt = 0;
	}
}
