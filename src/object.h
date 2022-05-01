#ifndef __OBJECT_H__
#define __OBJECT_H__
#include <map>
#include <mutex>
#include "sp.h"
#include "pool.h"

namespace ashan
{
#define OBJLOCK(MTX) std::lock_guard<decltype(MTX)> _(MTX)
#define LOCK(MTX, N) std::lock_guard<decltype(MTX)> _##N(MTX)

	template <class _ID, class _CLS>
	struct object : sp<_CLS>
	{
		using cls = std::shared_ptr<_CLS>;
		_ID m_id;
		std::recursive_mutex m_mutex;
		static pool<_ID, cls> m_pool;
		static auto &get_pool() { return m_pool; }
		template <class... _Args>
		static auto insert_pool(const _ID &_id, _Args &&..._args)
		{
			auto _cls = _CLS::make(std::forward<_Args>(_args)...);
			_cls->m_id = _id;
			m_pool.insert(_id, _cls);
			return _cls;
		}
		void remove_pool() { m_pool.erase(m_id); }
	};

	template <class _ID, class _CLS>
	pool<_ID, std::shared_ptr<_CLS>> object<_ID, _CLS>::m_pool;
}

#endif