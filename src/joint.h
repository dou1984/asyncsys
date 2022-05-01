#ifndef __JOINT_H__
#define __JOINT_H__
#include <string>
#include <sstream>

namespace ashan
{
	namespace detail
	{
		void joint(std::ostringstream &oss, const char *data);
		void joint(std::ostringstream &oss, char *data);
		void joint(std::ostringstream &oss, const std::string &data);
		void joint(std::ostringstream &oss, std::string &data);

		template <class _CLS>
		void joint(std::ostringstream &oss, _CLS &data)
		{
			static_assert(std::is_arithmetic<_CLS>::value);
			oss << std::to_string(data);
		}
	}

	class ostring
	{
		std::ostringstream oss;

	public:
		ostring() = default;
		template <class... _Args>
		std::string joint(_Args &&..._args)
		{
			((detail::joint(oss, _args)), ...);
			return oss.str();
		}
		template <class _First, class... _Args>
		std::string joint_with(const std::string &sep, _First &&_first, _Args &&..._args)
		{
			detail::joint(oss, _first);
			((joint(sep, _args)), ...);
			return oss.str();
		}
		std::string data() { return oss.str(); }
		void clear() { oss.str(""); }
	};
}
#endif