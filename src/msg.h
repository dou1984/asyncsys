#ifndef __MSG_H__
#define __MSG_H__
#include <string>

#define CHECK_MSG(_MSG, _BUF, _SIZE)         \
	ashan::MSG *_MSG = (ashan::MSG *)(_BUF); \
	if (_MSG->message_size() > _SIZE)        \
		return 0;
namespace ashan
{
	struct MSG
	{
		int len;
		int cmd;
		int error;
		char data[];

		int message_size() const { return sizeof(MSG) + len; }
		int message_method() const { return cmd; }
		static std::string make(int c, const char *buf, int s);
		static std::string make(int c, int err);		
	};
}
#endif