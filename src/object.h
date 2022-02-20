#ifndef __OBJECT_H__
#define __OBJECT_H__
#include <map>
#include <mutex>
#include "sp.h"

namespace ashan
{
#define OBJLOCK(MTX) std::lock_guard<decltype(MTX)> _(MTX)

	template <class _ID, class _CLS>
	struct object : sp<_CLS>
	{
		using cls = std::shared_ptr<_CLS>;
		_ID m_id;
		std::recursive_mutex m_mutex;
		struct pool
		{
			static std::map<_ID, cls> g_objects;
			static std::mutex g_mutex;
			static cls get(const _ID &_id)
			{
				std::lock_guard<std::mutex> _(g_mutex);
				if (auto it = g_objects.find(_id); it != g_objects.end())
					return it->second;
				return nullptr;
			}
			static void insert(const _ID &_id, cls s)
			{
				s->m_id = _id;
				std::lock_guard<std::mutex> _(g_mutex);
				g_objects.emplace(_id, s);
			}			
		};
	};

	template <class _ID, class _CLS>
	std::map<_ID, std::shared_ptr<_CLS>> object<_ID, _CLS>::pool::g_objects;
	template <class _ID, class _CLS>
	std::mutex object<_ID, _CLS>::pool::g_mutex;
}

#endif