/*
 * client.c
 *
 *  Created on: Nov 28, 2018
 *      Author: pp
 */
#include <stdio.h>
#include <stdlib.h>
#include <mongoose.h>
#include <ev.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#include "myfec.h"
#include "myaddr.h"
#include "myudp.h"
#include "mynode.h"
#include "mytun.h"
#include "my_debug.h"

#define TUN_TIMEOUT	(8.0)

enum WORK_MODE_T {
	UNKNOWN_MODE,
	CLIENT_MODE,
	SERVER_MODE
};

enum WORK_MODE_T work_mode;


int		no_local  = 1;
int		no_remote = 1;
int		reduant_num = 3;

myaddr_t	g_laddr;
myaddr_t	g_raddr;

mynode_t* remote_node;

int tun_fd = -1;
int sock_fd = -1;

void tun_timer_cb(struct ev_loop* main_loop, ev_timer *t, int revents);

static void dump_data(unsigned char* data, int data_len)
{
	int i;
	MY_DEBUG_INFO("[%04d]  ", data_len);
	MY_DEBUG_INFO("<");
	for(i = 0; i < data_len; i++)
	{
		MY_DEBUG_INFO("%02x ", data[i]);
	}
	MY_DEBUG_INFO(">\n");
}

int parse_args(int argc, char** argv)
{
	int ret = -1;
	int c;
	const char* options = "csl:r:y:";
	while((c = getopt(argc, argv, options)) != -1)
	{
		switch (c)
		{
		case 'c':
			work_mode = CLIENT_MODE;
			break;
		case 's':
			work_mode = SERVER_MODE;
			break;
		case 'l':
			no_local = 0;
			human2addr(&g_laddr, optarg);
			break;
		case 'r':
			no_remote = 0;
			human2addr(&g_raddr, optarg);
			break;
		case 'y':
			reduant_num = atoi(optarg);
			break;
		default:
			ret = -1;
			break;
		}
	}
	if ((work_mode == CLIENT_MODE && no_remote == 0)
			||
			(work_mode == SERVER_MODE && no_local == 0)
		)
	{
		ret = 0;
	}
	return ret;
}



void udp_server_cb(struct ev_loop* main_loop, ev_io* watcher, int e)
{
	char buf[2000];
	myaddr_t peer_addr;
	int recv_len = udp_recvfrom(watcher->fd, buf, sizeof(buf), &peer_addr);
	if (recv_len > 0)
	{
		MY_DEBUG_INFO("Got data:\n");
		dump_data((unsigned char*)buf, recv_len);
		if (!remote_node)
		{
			remote_node = mynode_alloc(10, reduant_num);
			remote_node->peer_addr = peer_addr;
			remote_node->timeout = TUN_TIMEOUT/1000.0;
			ev_timer_init(&remote_node->timer, tun_timer_cb, remote_node->timeout, 0);
			remote_node->timer_started = 0;
			char peer_addr_str[200];
			addr2human(&peer_addr, peer_addr_str);
			MY_DEBUG_INFO("Got a peer : %s\n", peer_addr_str);
		}
#if 0
		int dec_ret = myfec_decode(&remote_node->fec_ctx, buf, recv_len);
		if (dec_ret == 0)
		{
			char tun_buf[2000];
			int tun_len = myfec_decode_output(&remote_node->fec_ctx, tun_buf, sizeof(tun_buf));
			//TODO:write_tun
			write_tun(tun_fd, tun_buf, tun_len);
		}
#else
		int dec_ret = myfec_decode(&remote_node->fec_ctx, buf, recv_len);
		if (dec_ret == 1)
		{
			MY_DEBUG_INFO("==== decode success de_cnt = %d\n", remote_node->fec_ctx.de_cnt);
			int i;
			for(i = 0; i < remote_node->fec_ctx.de_cnt; i++)
			{
				int tun_len = remote_node->fec_ctx.de_buf[i].buf_len;
				char* tun_data = remote_node->fec_ctx.de_buf[i].buf_ptr;
				write_tun(tun_fd, tun_data, tun_len);
			}
		}
#endif
	}
}

void udp_client_cb(struct ev_loop* main_loop, ev_io* watcher, int e)
{
	char buf[2000];
	myaddr_t peer_addr;
	int recv_len = udp_recvfrom(watcher->fd, buf, sizeof(buf), &peer_addr);
	if (recv_len > 0)
	{
		MY_DEBUG_INFO("Got data:\n");
#if 0
		int dec_ret = myfec_decode(&remote_node->fec_ctx, buf, recv_len);
		if (dec_ret == 0)
		{
			char tun_buf[2000];
			int tun_len = myfec_decode_output(&remote_node->fec_ctx, tun_buf, sizeof(tun_buf));
			//TODO:write_tun
			write_tun(tun_fd, tun_buf, tun_len);
		}
#else
		int dec_ret = myfec_decode(&remote_node->fec_ctx, buf, recv_len);
		if (dec_ret == 1)
		{
			int i;
			for(i = 0; i < remote_node->fec_ctx.de_cnt; i++)
			{
				int tun_len = remote_node->fec_ctx.de_buf[i].buf_len;
				char* tun_data = remote_node->fec_ctx.de_buf[i].buf_ptr;
				write_tun(tun_fd, tun_data, tun_len);
			}
		}
#endif
	}
}

void tun_timer_cb(struct ev_loop* main_loop, ev_timer *t, int revents)
{
	MY_DEBUG_INFO("timer expired\n");
	ev_timer_stop(main_loop, t);
	remote_node->timer_started = 0;
	myfec_encode_output(&remote_node->fec_ctx);
	//TODO:sendto remote side
	int i;
	for(i = 0; i < remote_node->fec_ctx.en_x + remote_node->fec_ctx.en_y; i++)
	{
		char* packet = remote_node->fec_ctx.end_buf + (remote_node->fec_ctx.en_feclen + remote_node->fec_ctx.en_headerlen) * i;
		dump_data(packet, remote_node->fec_ctx.en_feclen + remote_node->fec_ctx.en_headerlen);
		char peer_addr_str[200];
		//addr2human(&remote_node->peer_addr, peer_addr_str);
		//MY_DEBUG_INFO("Send to peer : %s\n", peer_addr_str);
		udp_sendto(sock_fd, packet, remote_node->fec_ctx.en_feclen + remote_node->fec_ctx.en_headerlen, &remote_node->peer_addr);
	}
}

void tun_fd_cb(struct ev_loop* main_loop, ev_io* watcher, int e)
{
	char	tun_buf[2000];
	int		tun_len;
	MY_DEBUG_INFO("Get a packet\n");
	tun_len = read_tun(tun_fd, tun_buf, sizeof(tun_buf));
	if (tun_len > 0)
	{
		if (!remote_node)
		{
			return;
		}

#if 0
		//dump_data((unsigned char*)tun_buf, tun_len);
		int enc_ret = myfec_encode(&remote_node->fec_ctx, tun_buf, tun_len);
		if (enc_ret > 0)
		{
			int i;
			for(i = 0; i < enc_ret; i++)
			{
				char* out_ptr = NULL;
				int out_len = myfec_encode_output(&remote_node->fec_ctx, &out_ptr, i);
				//dump_data((unsigned char*)out_ptr, out_len);
				udp_sendto(sock_fd, out_ptr, out_len + 16, &remote_node->peer_addr);
			}
		}
#else
		int input_ret = myfec_encode_input(&remote_node->fec_ctx, tun_buf, tun_len);
		MY_DEBUG_INFO("encode input ret = %d\n", input_ret);
		if (input_ret == 1)
		{
			myfec_encode_output(&remote_node->fec_ctx);
			//TODO:sendto remote side
			int i;
			for(i = 0; i < remote_node->fec_ctx.en_x + remote_node->fec_ctx.en_y; i++)
			{
				char* packet = remote_node->fec_ctx.end_buf + (remote_node->fec_ctx.en_feclen + remote_node->fec_ctx.en_headerlen) * i;
				udp_sendto(sock_fd, packet, remote_node->fec_ctx.en_feclen + remote_node->fec_ctx.en_headerlen, &remote_node->peer_addr);
			}
			//TODO:stop the timer
			ev_timer_stop(main_loop, &remote_node->timer);
			remote_node->timer_started = 0;
			MY_DEBUG_INFO("stop timer 1\n");
		}
		else
		{
			if (remote_node->timer_started == 0)
			{
				MY_DEBUG_INFO("start timer 1\n");
				ev_timer_set(&remote_node->timer, TUN_TIMEOUT/1000.0, 0.);
				ev_timer_start(main_loop, &remote_node->timer);
				remote_node->timer_started = 1;
			}
			//TODO:if timer is not started, start it
		}
#endif
	}
}

int main(int argc, char** argv)
{
	char tun_ip[100];
	char tun_netmask[100] = "255.255.255.0";

	int ret = parse_args(argc, argv);
	if (ret == 0)
	{
		struct ev_loop * loop = ev_default_loop(0);

		struct ev_io sock_fd_watcher;
		if (work_mode == CLIENT_MODE)
		{
			strcpy(tun_ip, "10.22.23.2");
			tun_fd = open_tun("tap11", tun_ip, tun_netmask, 1500);
			sock_fd = new_udp_client();
			remote_node = mynode_alloc(10, reduant_num);
			remote_node->peer_addr = g_raddr;
			remote_node->timeout = TUN_TIMEOUT/1000.0;
			remote_node->timer_started = 0;
			ev_timer_init(&remote_node->timer, tun_timer_cb, remote_node->timeout, 0.);
			ev_io_init(&sock_fd_watcher, udp_client_cb, sock_fd, EV_READ);
		}
		else
		{

			strcpy(tun_ip, "10.22.23.1");
			tun_fd = open_tun("tap11", tun_ip, tun_netmask, 1500);
			sock_fd = new_udp_server(&g_laddr);
			ev_io_init(&sock_fd_watcher, udp_server_cb, sock_fd, EV_READ);

		}

		struct ev_io tun_fd_watcher;
		tun_fd_watcher.data = (void*)remote_node;
		ev_io_init(&tun_fd_watcher, tun_fd_cb, tun_fd, EV_READ);
		ev_io_start(loop, &tun_fd_watcher);
		ev_io_start(loop, &sock_fd_watcher);
		ev_run(loop, 0);

		close_tun(tun_fd);
		if (work_mode == CLIENT_MODE)
		{
			delete_udp_client(sock_fd);
		}
		else
		{
			delete_udp_server(sock_fd);
		}
		mynode_free(remote_node);
	}

	return ret;
}
