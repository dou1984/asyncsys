#include "pbmsg.h"

namespace ashan
{
	u32 PBMSG::message_size() const
	{
		return sizeof(PBMSG) + len;
	}
	u32 PBMSG::size() const
	{
		return len;
	}
	const char *PBMSG::data() const
	{
		return buffer;
	}
	char *PBMSG::data()
	{
		return buffer;
	}
}