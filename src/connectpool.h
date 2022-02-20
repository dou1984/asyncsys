#ifndef __CONNECTPOOL_H__
#define __CONNECTPOOL_H__
#include <memory>
#include <list>
#include <map>
#include <atomic>
#include <string>
#include "sp.h"
#include "log.h"

namespace ashan
{
	template <class _CLI>
	class cliwrapper;

	template <class _CLI>
	class connectpool : public sp<connectpool<_CLI>>
	{
		using cli = std::shared_ptr<_CLI>;
		std::string m_url;
		int m_max_thread = 4;
		int m_max_total = 40;
		std::atomic_int m_cnt_total = {0};

	public:
		connectpool(const std::string &_url) : m_url(_url)
		{
		}
		~connectpool()
		{
			at().clear();
		}
		cliwrapper<_CLI> get()
		{
			auto &_lst = at();
			if (_lst.size() == 0)
			{
				if (_lst.size() < m_max_thread || m_cnt_total < m_max_total)
				{
					return create();
				}
				return create();
			}
			auto obj = _lst.front();
			_lst.pop_front();
			return cliwrapper<_CLI>(obj, this->shared_from_this());
		}
		void reset(cli _cli)
		{
			auto &_lst = at();
			if (m_cnt_total <= m_max_total || _lst.size() <= m_max_thread)
			{
				_lst.push_back(_cli);
			}
			else
			{
				m_cnt_total--;
			}
		}

	private:
		auto &at()
		{
			static thread_local std::map<std::string, std::list<cli>> _manager;
			return _manager[m_url];
		}
		auto create()
		{			
			m_cnt_total++;
			return cliwrapper<_CLI>(_CLI::make(m_url), this->shared_from_this());
		}
	};
	template <class _CLI>
	class cliwrapper
	{
		std::shared_ptr<_CLI> m_cli;
		std::shared_ptr<connectpool<_CLI>> m_pool;

	public:
		cliwrapper(std::shared_ptr<_CLI> _cli,
				   std::shared_ptr<connectpool<_CLI>> _pool) : m_cli(_cli), m_pool(_pool)
		{
		}
		cliwrapper(cliwrapper &&o)
		{
			std::swap(m_cli, o.m_cli);
			std::swap(m_pool, o.m_pool);
		}
		cliwrapper(const cliwrapper &) = delete;
		~cliwrapper()
		{
			if (m_cli)
			{
				m_pool->reset(m_cli);
			}
		}
		auto operator->()
		{
			return m_cli.get();
		}
	};
}

#endif