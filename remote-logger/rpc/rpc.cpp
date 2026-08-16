#include "rpc.h"
#include <sstream>

std::string serialize(const RPCMessage& msg) {
    std::string out = msg.procedure;
    for (const auto& arg : msg.args) {
        out += "|" + arg;
    }
    out += "\n";
    return out;
}

RPCMessage deserialize(const std::string& raw) {
    RPCMessage msg;
    std::stringstream ss(raw);
    std::string token;

    if (std::getline(ss, token, '|')) {
        while (!token.empty() && (token.back() == '\n' || token.back() == '\r'))
            token.pop_back();
        msg.procedure = token;
    }

    while (std::getline(ss, token, '|')) {
        while (!token.empty() && (token.back() == '\n' || token.back() == '\r'))
            token.pop_back();
        if (!token.empty())
            msg.args.push_back(token);
    }

    return msg;
}