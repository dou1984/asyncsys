#include "joint.h"

namespace ashan
{
	namespace detail
	{
		void joint(std::ostringstream &oss, const char *data)
		{
			oss << data;
		}
		void joint(std::ostringstream &oss, char *data)
		{
			oss << data;
		}
		void joint(std::ostringstream &oss, const std::string &data)
		{
			oss << data;
		}
		void joint(std::ostringstream &oss, std::string &data)
		{
			oss << data;
		}
	}
}