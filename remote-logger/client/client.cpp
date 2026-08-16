#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../config.h"
#include "../rpc/rpc.h"

std::string call_rpc(const RPCMessage& msg) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return "ERROR: socket failed";

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_HOST, &server_addr.sin_addr);

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        return "ERROR: could not connect to server";
    }

    std::string payload = serialize(msg);
    send(sock, payload.c_str(), payload.size(), 0);

    char buf[BUFFER_SIZE] = {};
    recv(sock, buf, sizeof(buf) - 1, 0);
    close(sock);

    std::string response(buf);
    while (!response.empty() && (response.back() == '\n' || response.back() == '\r'))
        response.pop_back();

    return response;
}

int main() {
    std::cout << "=== Remote Logger Client ===\n";
    std::cout << "Choose your app type:\n";
    std::cout << "  1. WebApp\n";
    std::cout << "  2. PaymentApp\n";
    std::cout << "  3. InventoryApp\n";
    std::cout << "Enter choice: ";

    int choice;
    std::cin >> choice;
    std::cin.ignore();

    std::string source;
    if      (choice == 1) source = "WebApp";
    else if (choice == 2) source = "PaymentApp";
    else if (choice == 3) source = "InventoryApp";
    else {
        std::cout << "Invalid choice. Defaulting to WebApp.\n";
        source = "WebApp";
    }

    std::cout << "\nConnected as: " << source << "\n";
    std::cout << "Commands:\n";
    std::cout << "  log <message>   — send a log event\n";
    std::cout << "  logs            — fetch all logs\n";
    std::cout << "  stats           — get event statistics\n";
    std::cout << "  quit            — exit\n\n";

    std::string line;
    while (true) {
        std::cout << source << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "quit" || line == "exit") break;
        if (line.empty()) continue;

        RPCMessage msg;
        std::string response;

        if (line.substr(0, 4) == "log ") {
            std::string message = line.substr(4);
            msg.procedure = "log_event";
            msg.args      = { source, message };
            response      = call_rpc(msg);

        } else if (line == "logs") {
            msg.procedure = "get_logs";
            response      = call_rpc(msg);

        } else if (line == "stats") {
            msg.procedure = "get_stats";
            response      = call_rpc(msg);

        } else {
            std::cout << "Unknown command. Use: log <msg> | logs | stats | quit\n";
            continue;
        }

        std::cout << "\n" << response << "\n\n";
    }

    std::cout << "Bye!\n";
    return 0;
}