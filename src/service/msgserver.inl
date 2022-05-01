#include "../net/asyncnet.h"
#include "../net/asyncsys.h"
#include "msgserver.h"

namespace ashan
{
	template <class _ID>
	msgserver<_ID>::msgserver(const std::string &url) : m_url(url)
	{
		asyncsys::get().start(
			m_url,
			[this](auto, auto fd)
			{
				auto cli = client::make(fd)->init();
				auto s = session::make();
				s->set_client(cli.get());
				auto _this = this->shared_from_this();
				cli->dispatch = [=](auto, auto buf, auto size)
				{
					LOG_CORE("dispatch callback size %d", size);
					return _this->dispatch(_this.get(), s.get(), buf, size);
				};				
				s->oninit(s.get());
			});
	}
	template <class _ID>
	void msgserver<_ID>::add_method(const _ID &cmd, rpccb &&_callback)
	{
		m_router.emplace(cmd, std::move(_callback));
	}
	template <class _ID>
	void msgserver<_ID>::on_dispatch(session *s, const _ID &_method, const char *_data, int _size)
	{
		if (auto it = m_router.find(_method); it != m_router.end())
		{
			__TRY()
			it->second(s, _data, _size);
			__CATCH()
		}
	}	
}