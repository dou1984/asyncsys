#ifndef __SP_H__
#define __SP_H__
#include <memory>

template <class CLS>
struct sp : std::enable_shared_from_this<CLS>
{
	template <class... ARGS>
	static auto make(ARGS &&...args)
	{
		return std::make_shared<CLS>(std::forward<ARGS>(args)...);
	}	
};

#endif
