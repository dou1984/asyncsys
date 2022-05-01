#include <string.h>
#include <assert.h>
#include "wsmsg.h"

namespace ashan
{
	int WSMSG::__payload_size() const
	{
		return (len < wsbit32)	 ? 0
			   : (len < wsbit64) ? sizeof(u32)
								 : sizeof(u64);
	}
	u64 WSMSG::size() const
	{
		return (len < wsbit32)	 ? len
			   : (len < wsbit64) ? (*(u32 *)buffer)
								 : (*(u64 *)buffer);
	}
	u32 WSMSG::get_mask() const
	{
		return (mask == 1) ? (*(u32 *)(buffer + __payload_size())) : 0;
	}
	void WSMSG::set_mask(u32 m)
	{
		mask = 1;
		(*(u32 *)(buffer + __payload_size())) = m;
	}
	void WSMSG::set_size(u64 s)
	{
		if (s < wsbit32)
		{
			len = s;
		}
		else if (s < wsu32max)
		{
			len = wsbit32;
			(*(u32 *)buffer) = s;
		}
		else
		{
			len = wsbit64;
			(*(u64 *)buffer) = s;
		}
	}

	int WSMSG::c_payload_size(u64 s)
	{
		return (s < wsbit32)	? 0
			   : (s < wsu32max) ? sizeof(u32)
								: sizeof(u64);
	}
	int WSMSG::c_mask_size(int mask)
	{
		return sizeof(u32);
	}
	std::string WSMSG::make(u32 mask, const char *buf, u64 s)
	{
		std::string r;
		r.resize(sizeof(WSMSG) + c_mask_size(mask) + c_payload_size(s) + s);
		WSMSG *_msg = (WSMSG *)r.data();
		_msg->set_size(s);
		_msg->set_mask(mask);
		if (s > 0)
		{
			memcpy(_msg->data(), buf, s);
		}
		return r;
	}
	std::string WSMSG::make(const char *buf, u64 s)
	{
		std::string r;
		r.resize(sizeof(WSMSG) + c_payload_size(s) + s);
		WSMSG *_msg = (WSMSG *)r.data();
		_msg->set_size(s);
		if (s > 0)
		{
			memcpy(_msg->data(), buf, s);
		}
		return r;
	}
}