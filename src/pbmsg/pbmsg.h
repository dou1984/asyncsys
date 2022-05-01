#ifndef __PBMSG_H__
#define __PBMSG_H__
#include "../mytype.h"

namespace ashan
{
	struct PBMSG
	{
		u32 len;
		char buffer[];

		u32 message_size() const;
		u32 size() const;
		const char *data() const;
		char *data();
	};
}
#endif