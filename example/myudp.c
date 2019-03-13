/*
 * myudp.c
 *
 *  Created on: Nov 29, 2018
 *      Author: pp
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "myaddr.h"
#include "myudp.h"

int new_udp_client()
{
	int ret = -1;
	ret = socket(AF_INET, SOCK_DGRAM, 0);
	return ret;
}

int new_udp_server(myaddr_t* addr)
{
	int ret = -1;
	ret = socket(PF_INET, SOCK_DGRAM, 0);
	if (ret >= 0)
	{
		int sock_opt = 1;
		setsockopt(ret, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_opt, sizeof (sock_opt));

	    struct sockaddr_in servaddr;
	    memset(&servaddr, 0, sizeof(servaddr));
	    servaddr.sin_family = AF_INET;
	    servaddr.sin_port = htons(addr->port);
	    servaddr.sin_addr.s_addr = htonl(addr->ip);
	    bind(ret, (struct sockaddr *)&servaddr, sizeof(servaddr));
	}
	return ret;
}

int udp_sendto(int sock, char* buf, size_t buf_len, myaddr_t* addr)
{
	struct sockaddr_in peer_addr;
	socklen_t peer_len = sizeof(peer_addr);
	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr.sin_family = AF_INET;

	peer_addr.sin_addr.s_addr	= htonl(addr->ip);
	peer_addr.sin_port			= htons(addr->port);
	//printf("sendto <%s>: %s", peer, buf);
	int send_len = sendto(sock, buf, buf_len, 0, (struct sockaddr*)&peer_addr, peer_len);
	return send_len;
}

int udp_recvfrom(int sock, char* buf, size_t buf_len, myaddr_t* addr)
{
	struct sockaddr_in peer_addr;
	socklen_t peer_len = sizeof(peer_addr);
	int recv_len = recvfrom(sock, buf, buf_len, 0, (struct sockaddr*)&peer_addr, &peer_len);
	if (recv_len > 0)
	{
		addr->ip = ntohl(peer_addr.sin_addr.s_addr);
		addr->port = ntohs(peer_addr.sin_port);
	}
	return recv_len;
}

void delete_udp_client(int fd)
{
	close(fd);
}

void delete_udp_server(int fd)
{
	close(fd);
}

