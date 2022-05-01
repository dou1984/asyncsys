#ifndef __CONNECTPOOL_H__
#define __CONNECTPOOL_H__
#include <memory>
#include <list>
#include <map>
#include <atomic>
#include <string>
#include <assert.h>
#include "../sp.h"
#include "../log.h"
#include "../invalid.h"

namespace ashan
{

	template <class _CLI, int _MAX_TOTAL = 40>
	class connectpool : public sp<connectpool<_CLI, _MAX_TOTAL>>
	{
		enum tag_id
		{
			tag_manager,
			tag_waiter,
			tag_id_max,
		};
		using cli = std::shared_ptr<_CLI>;
		std::string m_url;
		int m_max_total = _MAX_TOTAL;
		std::atomic_int m_cnt_total = {0};

		class wrapper
		{
			std::shared_ptr<_CLI> m_cli;
			std::shared_ptr<connectpool> m_pool;

		public:
			wrapper(std::shared_ptr<_CLI> _cli,
					std::shared_ptr<connectpool> _pool) : m_cli(_cli), m_pool(_pool)
			{
			}
			wrapper(wrapper &&o) = delete;
			wrapper(const wrapper &) = delete;
			~wrapper()
			{
				if (m_cli)
				{
					m_pool->reset(m_cli);
				}
			}
			auto operator->() { return m_cli.get(); }
			auto get() { return m_cli; }
		};

	public:
		connectpool(const std::string &_url) : m_url(_url)
		{
		}
		~connectpool()
		{
			__mgr(tag_manager).clear();
			__mgr(tag_waiter).clear();
		}
		auto &url() const { return m_url; }
		auto obj() { return wrapper(data(), this->shared_from_this()); }
		auto data()
		{
			auto &_lst = __mgr(tag_manager);
			if (_lst.size() == 0)
			{
				return __create();
			}
			auto obj = _lst.front();
			_lst.pop_front();
			return obj;
		}
		void reset(cli _cli)
		{
			assert(_cli);
			auto &_lst = __mgr(tag_manager);
			if (m_cnt_total-- <= m_max_total)
			{
				_lst.push_back(_cli);
			}
		}
		auto valid_count() const
		{
			int count = 0;
			auto &_lst = __mgr(tag_manager);
			for (auto &it : _lst)
			{
				it.valid() && count++;
			}
			return count;
		}
		template <class... _Args>
		auto query(_Args &&..._args)
		{
			obj()->query(std::forward<_Args>(_args)...);
			return this;
		}
		template <class... _Args>
		auto send(_Args &&..._args)
		{
			obj()->send(std::forward<_Args>(_args)...);
			return this;
		}
	private:
		auto &__mgr(int t)
		{
			static thread_local std::map<std::string, std::list<cli>> _manager[tag_id_max];
			assert(t >= 0);
			assert(t < tag_id_max);
			return _manager[t][m_url];
		}
		auto __create()
		{
			m_cnt_total++;
			return _CLI::make(m_url);
		}
	};

}

#endif