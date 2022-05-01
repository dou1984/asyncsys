#ifndef __POOL_H__
#define __POOL_H__
#include <map>
#include <mutex>
#include <functional>

namespace ashan
{
	template <class _ID, class _CLS>
	struct pool
	{
		std::mutex m_mutex;
		std::map<_ID, _CLS> m_objects;
		void insert(const _ID &_id, const _CLS &s)
		{
			std::lock_guard<decltype(m_mutex)> _(m_mutex);
			m_objects.emplace(_id, s);
		}
		void insert(const _ID &_id, _CLS &&s)
		{
			std::lock_guard<decltype(m_mutex)> _(m_mutex);
			m_objects.emplace(_id, s);
		}
		void erase(_CLS &s)
		{
			std::lock_guard<decltype(m_mutex)> _(m_mutex);
			m_objects.erase(s->m_id);
		}
		void erase(const _ID &_id)
		{
			std::lock_guard<decltype(m_mutex)> _(m_mutex);
			if (auto it = m_objects.find(_id); it != m_objects.end())
			{
				m_objects.erase(it);
			}
		}
		_CLS get(const _ID &_id)
		{
			std::lock_guard<decltype(m_mutex)> _(m_mutex);
			if (auto it = m_objects.find(_id); it != m_objects.end())
			{
				return it->second;
			}
			return nullptr;
		}
		void for_each(const std::function<void(const _ID &, const _CLS &)> &_fn)
		{
			std::lock_guard<decltype(m_mutex)> _(m_mutex);
			for (auto it = m_objects.begin(); it != m_objects.end(); ++it)
			{
				_fn(it->first, it->second);
			}
		}	
	};
}
#endif