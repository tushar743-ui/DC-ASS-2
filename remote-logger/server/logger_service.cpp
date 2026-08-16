#include "logger_service.h"
#include <vector>
#include <mutex>
#include <map>
#include <sstream>
#include <ctime>

static std::vector<std::string> logs;
static std::mutex log_mutex;

static std::string current_time() {
    std::time_t now = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

std::string log_event(const std::string& source, const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex);
    std::string entry = "[" + current_time() + "] [" + source + "] " + message;
    logs.push_back(entry);
    return "OK";
}

std::string get_logs() {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (logs.empty()) return "NO_LOGS";

    std::string result;
    for (const auto& entry : logs) {
        result += entry + "\n";
    }
    return result;
}

std::string get_stats() {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (logs.empty()) return "Total=0";

    std::map<std::string, int> counts;

    for (const auto& entry : logs) {
        size_t start = entry.find("] [");
        size_t end   = entry.find("]", start + 3);
        if (start != std::string::npos && end != std::string::npos) {
            std::string source = entry.substr(start + 3, end - start - 3);
            counts[source]++;
        }
    }

    std::ostringstream ss;
    ss << "Total=" << logs.size();
    for (const auto& pair : counts) {
        ss << " " << pair.first << "=" << pair.second;
    }
    return ss.str();
}