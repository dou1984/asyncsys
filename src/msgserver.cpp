#include "msg.h"
#include "asyncsys.h"
#include "msgserver.h"
#include "session.h"
#include "mythrow.h"

namespace ashan
{	
	msgserver::msgserver(const std::string &url) : m_url(url)
	{
		asyncsys::get().start(
			m_url,
			[this](auto, auto fd)
			{
				auto cli = client::make(fd)->init();
				auto s = session::make();
				s->set_client(cli);
				auto _this = shared_from_this();
				cli->dispatch = [=](auto, auto buf, auto size)
				{
					MSG *_msg = (MSG *)(buf);
					if (_msg->message_size() > size)
					{
						return 0;
					}
					if (auto it = _this->m_router.find(_msg->message_method()); it != _this->m_router.end())
					{
						__TRY()
						it->second(s, _msg->data, _msg->len);
						__CATCH()
					}
					return _msg->message_size();
				};
			});
	}
	void msgserver::add_method(int cmd, rpccb &&_callback)
	{
		m_router.emplace(cmd, std::move(_callback));
	}
}