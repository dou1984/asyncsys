#ifndef __MSG_H__
#define __MSG_H__
#include <string>
#include "../mytype.h"

namespace ashan
{
	struct MSG
	{
		u32 len;
		int cmd;
		int error;
		char buffer[];

		u32 message_size() const { return sizeof(MSG) + len; }
		int message_method() const { return cmd; }
		int last_error() const { return error; }
		u32 size() const { return len; }
		const char *data() const { return buffer; }
		char *data() { return buffer; }
		static std::string make(int cmd, const char *buf, u32 s);
		static std::string make(int cmd, int err, const char *buf, u32 s);
		static std::string make(int cmd, int err);
	};
}
#endif