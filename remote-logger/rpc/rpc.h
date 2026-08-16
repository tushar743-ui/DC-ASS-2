#pragma once
#include <string>
#include <vector>

struct RPCMessage {
    std::string procedure;
    std::vector<std::string> args;
};

std::string serialize(const RPCMessage& msg);

RPCMessage deserialize(const std::string& raw);