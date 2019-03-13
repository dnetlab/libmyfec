/*
 * common.h
 *
 *  Created on: Nov 26, 2018
 *      Author: pp
 */

#ifndef MYCOMMON_H_
#define MYCOMMON_H_

typedef unsigned long long u64_t;   //this works on most platform,avoid using the PRId64
typedef long long i64_t;

typedef unsigned int u32_t;
typedef int i32_t;

typedef unsigned short u16_t;
typedef short i16_t;

u64_t pack_u64(u32_t a,u32_t b);
u32_t get_u64_h(u64_t a);
u32_t get_u64_l(u64_t a);
void write_u16(char * p,u16_t w);
u16_t read_u16(char * p);
void write_u32(char * p,u32_t l);
u32_t read_u32(char * p);

#endif /* MYCOMMON_H_ */
