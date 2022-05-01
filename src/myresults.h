#ifndef __MYRESULTS_H__
#define __MYRESULTS_H__

namespace ashan
{
	struct myresults
	{
		int last_error = 0;
		const char *last_msg = 0;

		//bool valid() const { return last_error == 0; }
		int error() const { return last_error; }
		const char *strerror() const { return last_msg; }
	};
}
#endif