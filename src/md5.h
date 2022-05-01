#ifndef __MD5_H__
#define __MD5_H__
#include <string>

namespace ashan
{
    struct md5 final
    {
        static std::string encode(unsigned char *buf, int len);
        static std::string encode(char *buf, int len)
        {
            return encode((unsigned char *)buf, len);
        }
        static std::string encode(const char *buf, int len)
        {
            return encode((unsigned char *)buf, len);
        }
        static std::string encode(const unsigned char *buf, int len)
        {
            return encode((unsigned char *)buf, len);
        }
    };
}
#endif
