#ifndef __SERIALIZE_H__
#define __SERIALIZE_H__
#include <string>
#include <functional>
#include "pbmsg.h"
#include "proto/pbmsg.pb.h"

namespace ashan
{
	std::string serialize(MsgBase &_base);
	std::string __serialize(const std::string &_SRV, const std::string &_CMD, int error, const std::string &code);
	std::string __serialize(const std::string &_SRV, const std::string &_CMD, int error, const ::google::protobuf::Message &cls);
	std::string __serialize(const std::string &_SRV, const std::string &_CMD, int error, const std::string &sess, const ::google::protobuf::Message &cls);
	std::string __serialize(const std::string &_SRV, const std::string &_CMD, int error, u64 index, const std::string &sess, const ::google::protobuf::Message &cls);
	std::string __serialize(const std::string &_SRV, const std::string &_CMD, int error, u64 index, const ::google::protobuf::Message &cls);

}
#endif