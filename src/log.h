#ifndef __LOG_H__
#define __LOG_H__
#include "mytime.h"

#define LOG_(LVL, HEAD, MSG, ...)                                                                    \
	if (log_level(LVL))                                                                              \
	{                                                                                                \
		printf("[" HEAD " %ld %s:%d]" MSG "\n", ashan::mytime::now(), __FUNCTION__, __LINE__, __VA_ARGS__); \
	}

#define LOG_CORE(_MSG, ...) LOG_(e_log_level_core, "CORE", _MSG, __VA_ARGS__)
#define LOG_DBG(_MSG, ...) LOG_(e_log_level_debug, "DBG", _MSG, __VA_ARGS__)
#define LOG_INF(_MSG, ...) LOG_(e_log_level_info, "INF", _MSG, __VA_ARGS__)
#define LOG_ERR(_MSG, ...) LOG_(e_log_level_error, "ERR", _MSG, __VA_ARGS__)

enum e_log_level
{
	e_log_level_core,
	e_log_level_debug,
	e_log_level_error,
	e_log_level_info,
	e_log_level_fatal,
};

bool log_level(int);
void set_log_level(int);
#endif
