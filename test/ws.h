#pragma once
#include <string>
#include "mytype.h"
#include "wsmsg/wsmsg.h"

namespace ashan
{
	struct WS
	{
		int cmd;
		int error;
		char buffer[];
		int message_method() const { return cmd; }
		int last_error() const { return error; }
		
		static std::string make(u32 mask, int cmd, int error, const char *buf, u32 size);
		static std::string make(int cmd, int error, const char *buf, u32 s);
	};
} // namespace ashan