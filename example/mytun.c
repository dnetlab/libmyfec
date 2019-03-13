/*
 * tun.c
 *
 *  Created on: Nov 28, 2018
 *      Author: pp
 */

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <errno.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>

#include "mytun.h"

int mytun_set_ipaddr(char* tun_name, char *ip)
{
    int s;

    if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
		//cout << "Error up " << tun_name <<  ": ret " << errno << endl;
    	printf("[%s:%d] Error :%d\n", __FUNCTION__, __LINE__, errno);
        return -1;
    }

    struct ifreq ifr;
    //strcpy(ifr.ifr_name, interface_name);
    strcpy(ifr.ifr_name, tun_name);

    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = PF_INET;
    inet_aton(ip, &addr.sin_addr);

    memcpy(&ifr.ifr_ifru.ifru_addr, &addr, sizeof(struct sockaddr_in));

    if(ioctl(s, SIOCSIFADDR, &ifr) < 0)
    {
    	printf("[%s:%d] Error :%d\n", __FUNCTION__, __LINE__, errno);
		//cout << "Error set ip " << tun_name <<  ": ret " << errno << endl;
        return -1;
    }

	close(s);
    return 0;
}

int mytun_set_netmask(char* tun_name, char *ip)
{
    int s;

    if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
    	printf("[%s:%d] Error :%d\n", __FUNCTION__, __LINE__, errno);
		//cout << "Error create socket: ret " << errno << endl;
        return -1;
    }

    //printf("netmask:%s\n", ip);
    struct ifreq ifr;
    //strcpy(ifr.ifr_name, interface_name);
    strcpy(ifr.ifr_name, tun_name);

    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = PF_INET;
    inet_aton(ip, &addr.sin_addr);

    memcpy(&ifr.ifr_ifru.ifru_addr, &addr, sizeof(struct sockaddr_in));

    if(ioctl(s, SIOCSIFNETMASK, &ifr) < 0)
    {
    	printf("[%s:%d] Error :%d\n", __FUNCTION__, __LINE__, errno);
		//cout << "Error set netmask " << tun_name <<  ": ret " << errno << endl;
        return -1;
    }

	close(s);
    return 0;
}

int mytun_set_mtu(char* tun_name, int mtu)
{
    int s;

    if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
		//cout << "Error create socket: ret " << errno << endl;
    	printf("[%s:%d] Error :%d\n", __FUNCTION__, __LINE__, errno);
        return -1;
    }

    struct ifreq ifr;
    //strcpy(ifr.ifr_name, interface_name);
    strcpy(ifr.ifr_name, tun_name);

	ifr.ifr_ifru.ifru_mtu = mtu;

    if(ioctl(s, SIOCSIFMTU, &ifr) < 0)
    {
		//cout << "Error set netmask " << tun_name <<  ": ret " << errno << endl;
    	printf("[%s:%d] Error :%d\n", __FUNCTION__, __LINE__, errno);
        return -1;
    }

	close(s);
    return 0;
}

int mytun_up(char* tun_name)
{
    int s;

    if((s = socket(PF_INET,SOCK_STREAM,0)) < 0)
    {
		//cout << "Error create socket : ret " << errno << endl;
    	printf("[%s:%d] Error :%d\n", __FUNCTION__, __LINE__, errno);
        return -1;
    }

    struct ifreq ifr;
    //strcpy(ifr.ifr_name,interface_name);
    strcpy(ifr.ifr_name, tun_name);

    short flag;
    flag = IFF_UP;
    if(ioctl(s, SIOCGIFFLAGS, &ifr) < 0)
    {
		//cout << "Error ioctl: ret " << errno << endl;
    	printf("[%s:%d] Error :%d\n", __FUNCTION__, __LINE__, errno);
        return -1;
    }

    ifr.ifr_ifru.ifru_flags |= flag;

    if(ioctl(s, SIOCSIFFLAGS, &ifr) < 0)
    {
		//cout << "Error up: ret " << errno << endl;
    	printf("[%s:%d] Error :%d\n", __FUNCTION__, __LINE__, errno);
        return -1;
    }
  	close(s);
    return 0;
}

#if 1
int open_tun(char* tun_name, char* ip, char* netmask, int mtu)
{
    struct ifreq ifr;
    int fd, err;

	//tun_name = dev;

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
    {
        printf("Error :%d\n", errno);
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags |= IFF_TAP | IFF_NO_PI;
    if (*tun_name != '\0')
    {
        strncpy(ifr.ifr_name, tun_name, IFNAMSIZ);
    }

    if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0)
    {
        printf("Error :%d\n", errno);
        close(fd);
        return -1;
    }
    mytun_up(tun_name);
    mytun_set_ipaddr(tun_name, ip);
    mytun_set_netmask(tun_name, netmask);
    mytun_set_mtu(tun_name, mtu);
    //strcpy(dev, ifr.ifr_name);
  	//tun_fd = fd;
    return fd;
}
#else
int open_tun(char *if_name, char* local_ip, char *netmask, int mtu)
{
	//printf("i m here1\n");
	struct ifreq ifr;
	struct sockaddr_in sai;
	printf("ip %s\n", local_ip);
	printf("mask %s\n", netmask);
	memset(&ifr,0,sizeof(ifr));
	memset(&sai, 0, sizeof(sai));

	printf("ip %s\n", local_ip);
	printf("mask %s\n", netmask);

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, if_name, IFNAMSIZ);

    sai.sin_family = AF_INET;
    sai.sin_port = 0;

    struct in_addr local_addr;
    inet_pton(AF_INET, local_ip, &local_addr);

    struct in_addr local_netmask;
    inet_pton(AF_INET, netmask, &local_netmask);

    sai.sin_addr.s_addr = local_addr.s_addr;
    memcpy(&ifr.ifr_addr, &sai, sizeof(sai));
    printf("ip(%s):%08x\n", local_ip, sai.sin_addr.s_addr);
    assert(ioctl(sockfd, SIOCSIFADDR, &ifr)==0); //set source ip

    //sai.sin_addr.s_addr = remote_ip;
    //memcpy(&ifr.ifr_addr,&sai, sizeof(struct sockaddr));
    //assert(ioctl(sockfd, SIOCSIFDSTADDR, &ifr)==0);//set dest ip

    sai.sin_addr.s_addr = local_netmask.s_addr;
    memcpy(&ifr.ifr_addr,&sai, sizeof(sai));
    printf("netmask:%08x\n", sai.sin_addr.s_addr);
    assert(ioctl(sockfd, SIOCSIFNETMASK, &ifr)==0);

    ifr.ifr_mtu=mtu;
    assert(ioctl(sockfd, SIOCSIFMTU, &ifr)==0);//set mtu

    assert(ioctl(sockfd, SIOCGIFFLAGS, &ifr)==0);
   // ifr.ifr_flags |= ( IFF_UP|IFF_POINTOPOINT|IFF_RUNNING|IFF_NOARP|IFF_MULTICAST );
    ifr.ifr_flags = ( IFF_UP|IFF_RUNNING|IFF_MULTICAST|IFF_TAP);//set interface flags
    assert(ioctl(sockfd, SIOCSIFFLAGS, &ifr)==0);

    //printf("i m here2\n");
	return 0;
}
#endif

void close_tun(int fd)
{
	if (fd >= 0)
	{
		close(fd);
	}
	return;
}

int write_tun(int fd, char* buf, int buf_len)
{
	return write(fd, buf, buf_len);
}

int read_tun(int fd, char* buf, int buf_len)
{
	return read(fd, buf, buf_len);
}

