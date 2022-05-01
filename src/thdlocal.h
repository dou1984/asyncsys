#ifndef __THDLOCAL_H__
#define __THDLOCAL_H__

namespace ashan
{
	template <class _CLS>
	struct thdlocal
	{
		static auto &get()
		{
			static thread_local _CLS _;
			return _;
		}
	};

}
#endif