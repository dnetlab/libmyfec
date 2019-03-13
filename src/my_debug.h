#ifndef _MY_DEBUG_H_
#define _MY_DEBUG_H_

#include <stdio.h>
#include <stdarg.h>

enum debug_level_e
{
	DEBUG_LEVEL_NO,
	DEBUG_LEVEL_ERR,
	DEBUG_LEVEL_WARN,
	DEBUG_LEVEL_INFO,
	DEBUG_LEVEL_ALL,
};

#ifdef __cplusplus
extern "C" {
#endif

void my_debug_set_level(int level);
int my_debug(int debug_level, const char *file_name, const char *func_name, int line, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#define MY_DEBUG_ERR(fmt, args...)	my_debug(DEBUG_LEVEL_ERR, __FILE__, __func__, __LINE__, fmt, ##args)
#define MY_DEBUG_WARN(fmt, args...)	my_debug(DEBUG_LEVEL_WARN, __FILE__, __func__, __LINE__, fmt, ##args) 
#define MY_DEBUG_INFO(fmt, args...)	my_debug(DEBUG_LEVEL_INFO, __FILE__, __func__, __LINE__, fmt, ##args)

//#define MY_DEBUG_ERR(fmt, ...)	my_debug(DEBUG_LEVEL_ERR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
//#define MY_DEBUG_WARN(fmt, ...)	my_debug(DEBUG_LEVEL_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__) 
//#define MY_DEBUG_INFO(fmt, ...)	my_debug(DEBUG_LEVEL_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

#endif
