#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "my_debug.h"
//#include "udp_tool.h"

#define MY_DBUG_TO_PORT 7689

int default_dbg_level = DEBUG_LEVEL_NO;

int my_debug(int debug_level, const char *file_name, const char *func_name, int line, const char *fmt, ...) 
{
	int ret = 0;
	va_list ap;
	if (default_dbg_level != DEBUG_LEVEL_NO)
	{
		char *prefix_line = NULL;
		if (debug_level <= default_dbg_level)
		{
			switch (debug_level)
			{
				case DEBUG_LEVEL_ERR:
					prefix_line = (char*)"\033[4m\033[47;31m[ERROR] \033[0m";
					break;
				case DEBUG_LEVEL_WARN:
					prefix_line = (char*)"\033[4m\033[47;33m[WARN] \033[0m";
					break;
				case DEBUG_LEVEL_INFO:
					prefix_line = (char*)"\033[4m\033[47;32m[INFO] \033[0m";
					break;
				default:
					break;
			}
			char line_buf[2048];
			memset(line_buf, 0, sizeof(line_buf));
			if (prefix_line)
			{
				snprintf(line_buf, sizeof(line_buf) - 1, "%s <%s - %s: %d> ", prefix_line, file_name, func_name, line);
			}
			else
			{
				snprintf(line_buf, sizeof(line_buf) - 1, "<%s - %s: %d> ", file_name, func_name, line);
			}

			va_start(ap, fmt);
			char remain_buf[1600];
			memset(remain_buf, 0, sizeof(remain_buf));
			ret = vsnprintf(remain_buf, sizeof(remain_buf) - 1, fmt, ap);
			strcat(line_buf, remain_buf);
			printf("%s", line_buf);
			va_end(ap);
		}
	}

	return ret;
}

void my_debug_set_level(int level)
{
	default_dbg_level = level;
	return;
}
