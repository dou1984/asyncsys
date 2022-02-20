#include "log.h"

int g_log_level = e_log_level_debug;
bool log_level(int _level)
{
	return g_log_level <= _level;
}
void set_log_level(int _level)
{
	g_log_level = _level;	
}