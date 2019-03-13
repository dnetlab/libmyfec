/*
 * mynode.h
 *
 *  Created on: Nov 29, 2018
 *      Author: pp
 */

#ifndef MYNODE_H_
#define MYNODE_H_
#include <stdio.h>
#include <sys/socket.h>
#include <ev.h>
#include <stdint.h>
#include "myfec.h"
#include "myaddr.h"

typedef struct
{
	myaddr_t			peer_addr;
	myfec_ctx_t			fec_ctx;
	double				timeout;
	ev_timer			timer;
	int					timer_started;
	//char send_buf[2000];
	//int send_len;
	//char recv_buf[2000];
	//int recv_len;

	//mybuf_t*			peer_in_buf;
	//mybuf_t*			peer_out_buf;

}mynode_t;

//mynode_t* mynode_alloc();
mynode_t* mynode_alloc(int max_actual_num, int reduant_num);
void mynode_free(mynode_t* );

#endif /* MYNODE_H_ */
