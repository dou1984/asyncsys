#ifndef __TRYCATCH_H__
#define __TRYCATCH_H__
#include "log.h"

#define __TRY() \
	try         \
	{
#define __CATCH()                     \
	}                                 \
	catch (const char *w)             \
	{                                 \
		LOG_ERR("catch throw: %s", w) \
	}                                 \
	catch (...) {}

#endif