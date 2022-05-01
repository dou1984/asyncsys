#ifndef __PBSERVER_H__
#define __PBSERVER_H__
#include "../service/msgserver.h"
#include "pbrpc.h"
#include "pbdispatch.h"

namespace ashan
{
	using pbserver = msgserver<std::string>;	
}
#endif