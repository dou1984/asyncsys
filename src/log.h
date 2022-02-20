#ifndef __LOG_H__
#define __LOG_H__

#define LOG_DBG(MSG, ...)             \
	if (log_level(e_log_level_debug)) \
	printf("[DBG %s:%d]" MSG "\n", __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_INF(MSG, ...)            \
	if (log_level(e_log_level_info)) \
	printf("[INF %s:%d]" MSG "\n", __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_ERR(MSG, ...)             \
	if (log_level(e_log_level_error)) \
	printf("[ERR %s:%d]" MSG "\n", __FUNCTION__, __LINE__, __VA_ARGS__)

enum e_log_level
{
	e_log_level_debug,
	e_log_level_error,
	e_log_level_info,
	e_log_level_fatal,
};

bool log_level(int);
void set_log_level(int);
#endif
