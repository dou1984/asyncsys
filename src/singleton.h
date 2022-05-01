#ifndef __SIGNLETON_H__
#define __SIGNLETON_H__

namespace ashan
{
	template <class _CLS>
	struct singleton
	{
		static auto &get()
		{
			static _CLS _;
			return _;
		}
	};
}
#endif
