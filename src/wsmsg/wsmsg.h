#ifndef __WSMSG_H__
#define __WSMSG_H__
#include <string>
#include "../mytype.h"

#define wsbit32 126
#define wsbit64 127
#define wsu32max ((u64)(0x100000000))

namespace ashan
{
	struct WSMSG
	{
		u16 fin : 1;
		u16 rsv : 3;
		u16 opcode : 4;
		u16 mask : 1;
		u16 len : 7;
		char buffer[];

		u64 message_size() const { return head_size() + size(); }
		int mask_size() const { return sizeof(u32); }
		int head_size() const { return sizeof(WSMSG) + __extend_size(); }
		u64 size() const;
		u32 get_mask() const;
		void set_mask(u32 m);
		void set_size(u64 s);
		const char *data() const { return buffer + __extend_size(); }
		char *data() { return buffer + __extend_size(); }
		int __payload_size() const;
		int __mask_size() const { return ((mask == 1) ? sizeof(u32) : 0); }
		int __extend_size() const { return __mask_size() + __payload_size(); }

		static std::string make(u32 mask, const char *buf, u64 s);
		static std::string make(const char *buf, u64 s);
		static int c_payload_size(u64 s);
		static int c_mask_size(int mask);
	};
}
#endif