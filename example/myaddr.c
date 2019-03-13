/*
 * myaddr.c
 *
 *  Created on: Nov 29, 2018
 *      Author: pp
 */
#include <arpa/inet.h>
#include <errno.h>
#include "myaddr.h"

//convert the addr_t to a 64-bit key for hash
uint64_t addr2key(myaddr_t* addr)
{
	uint64_t ret = 0;
	ret = (uint64_t)addr->ip;
	ret <<= 32;
	ret |= (uint64_t)addr->port;
	return ret;
}

//convert the addr_t to a human string
void addr2human(myaddr_t* addr, char* out)
{
	struct in_addr ip_addr;
	ip_addr.s_addr = htonl(addr->ip);
	char ip_buf[100] = "";
	socklen_t buf_len = sizeof(ip_buf);
	const char* ret = inet_ntop(AF_INET, (const void*)&ip_addr, ip_buf, buf_len);
	if (!ret)
	{
		perror("addr2human error:");
	}
	uint16_t port_addr = addr->port;
	sprintf(out, "%s:%u", ip_buf, port_addr);
	return;
}

//convert the human string to addr_t
int human2addr(myaddr_t* addr, char* in)
{
	int ret = -1;
	uint32_t	port_addr;
	struct in_addr ip_addr;
	//socklen_t socklen = sizeof(ip_addr);
	char ip_buf[100];

	int scan_ret = sscanf(in, "%[^:]:%u", ip_buf, &port_addr);
	if (scan_ret == 2)
	{
		inet_pton(AF_INET, ip_buf, &ip_addr);
		addr->ip = ntohl(ip_addr.s_addr);
		addr->port = (uint16_t)port_addr;
		ret = 0;
	}

	return ret;
}
