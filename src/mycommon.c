/*
 * common.c
 *
 *  Created on: Nov 26, 2018
 *      Author: pp
 */

#include <assert.h>
#include "mycommon.h"

#include <stdio.h>
#include <stdlib.h>

u64_t pack_u64(u32_t a,u32_t b)
{
	u64_t ret=a;
	ret<<=32u;
	ret+=b;
	return ret;
}
u32_t get_u64_h(u64_t a)
{
	return a>>32u;
}
u32_t get_u64_l(u64_t a)
{
	return (a<<32u)>>32u;
}

void write_u16(char * p,u16_t w)
{
	*(unsigned char*)(p + 1) = (w & 0xff);
	*(unsigned char*)(p + 0) = (w >> 8);
}
u16_t read_u16(char * p)
{
	u16_t res;
	res = *(const unsigned char*)(p + 0);
	res = *(const unsigned char*)(p + 1) + (res << 8);
	return res;
}

void write_u32(char * p,u32_t l)
{
	*(unsigned char*)(p + 3) = (unsigned char)((l >>  0) & 0xff);
	*(unsigned char*)(p + 2) = (unsigned char)((l >>  8) & 0xff);
	*(unsigned char*)(p + 1) = (unsigned char)((l >> 16) & 0xff);
	*(unsigned char*)(p + 0) = (unsigned char)((l >> 24) & 0xff);
}
u32_t read_u32(char * p)
{
	u32_t res;
	res = *(const unsigned char*)(p + 0);
	res = *(const unsigned char*)(p + 1) + (res << 8);
	res = *(const unsigned char*)(p + 2) + (res << 8);
	res = *(const unsigned char*)(p + 3) + (res << 8);
	return res;
}
