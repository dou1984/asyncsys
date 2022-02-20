#ifndef __TRYCATCH_H__
#define __TRYCATCH_H__

#define __TRY() \
	try         \
	{
#define __CATCH() \
	}             \
	catch (...) {}

#endif