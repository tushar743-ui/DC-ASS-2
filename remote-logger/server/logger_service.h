#pragma once
#include <string>

std::string log_event(const std::string& source, const std::string& message);

std::string get_logs();

std::string get_stats();