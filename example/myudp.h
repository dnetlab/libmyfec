/*
 * myudp.h
 *
 *  Created on: Nov 29, 2018
 *      Author: pp
 */

#ifndef MYUDP_H_
#define MYUDP_H_

#include "myaddr.h"

int new_udp_client();
int new_udp_server(myaddr_t* addr);

void delete_udp_client(int fd);
void delete_udp_server(int fd);

int udp_sendto(int sock, char* buf, size_t buf_len, myaddr_t* addr);
int udp_recvfrom(int sock, char* buf, size_t buf_len, myaddr_t* addr);

#endif /* MYUDP_H_ */
