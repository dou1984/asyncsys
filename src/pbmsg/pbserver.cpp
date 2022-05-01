#include "../net/heardbeat.h"
#include "../service/session.h"
#include "pbmsg.h"
#include "pbserver.h"

namespace ashan
{
	static auto g_heartbeat_oninit = heartbeat_init<session, PBMSG>();
}
