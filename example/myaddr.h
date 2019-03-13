/*
 * myvpn.h
 *
 *  Created on: Nov 29, 2018
 *      Author: pp
 */

#ifndef MYADDR_H_
#define MYADDR_H_

#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>

typedef struct
{
	uint32_t ip;	//host order
	uint16_t port;	//host order
}myaddr_t;

//convert the addr_t to a 64-bit key for hash
uint64_t addr2key(myaddr_t* addr);

//convert the addr_t to a human string
void addr2human(myaddr_t* addr, char* out);

//convert the human string to addr_t
int human2addr(myaddr_t* addr, char* in);

#endif /* MYVPN_H_ */
