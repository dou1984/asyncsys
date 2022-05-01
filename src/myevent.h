#ifndef __MYEVENT_H__
#define __MYEVENT_H__
#include "net/asyncevent.h"

namespace ashan
{
	template <class _CLS>
	struct myevent
	{
		static void onevent(const std::string &_str, std::function<void(const _CLS *)> &&_fcb)
		{
			asyncevent::get().onevent(
				_str,
				[_fcb = std::move(_fcb)](const void *_cls)
				{ _fcb((const _CLS *)(_cls)); });
		}
		static void emit(const std::string &_str, const _CLS &_cls)
		{
			asyncevent::get().emit(_str, &_cls);
		}
		static void emit(const std::string &_str, _CLS&& _cls)
		{
			asyncevent::get().emit(_str, &_cls);
		}
	};
}
#endif