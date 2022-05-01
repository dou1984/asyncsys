#ifndef __BASE64_H__
#define __BASE64_H__
#include <string>

namespace ashan
{
	struct base64
	{
		static std::string encode(const char *, int);
		static std::string decode(const char *, int);
	};
}
#endif

