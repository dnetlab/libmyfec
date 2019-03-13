/*
 * mytun.h
 *
 *  Created on: Nov 28, 2018
 *      Author: pp
 */

#ifndef _MYTUN_H_
#define _MYTUN_H_


int open_tun(char* name, char* ip, char* netmask, int mtu);
void close_tun(int fd);

int read_tun(int fd, char* buf, int buf_len);
int write_tun(int fd, char* buf, int buf_len);

#endif
