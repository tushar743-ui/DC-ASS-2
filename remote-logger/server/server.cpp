#include <iostream>
#include <string>
#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../config.h"
#include "../rpc/rpc.h"
#include "logger_service.h"


std::string dispatch(const RPCMessage& msg) {
    if (msg.procedure == "log_event") {
        if (msg.args.size() < 2)
            return "ERROR: log_event requires source and message";
        return log_event(msg.args[0], msg.args[1]);

    } else if (msg.procedure == "get_logs") {
        return get_logs();

    } else if (msg.procedure == "get_stats") {
        return get_stats();

    }
    return "ERROR: unknown procedure";
}


void* client_thread(void* arg) {
    int client_fd = *(int*)arg;
    delete (int*)arg;

    char buf[BUFFER_SIZE];

    while (true) {
        memset(buf, 0, sizeof(buf));
        int bytes = recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (bytes <= 0) break;

        std::string raw(buf);
        std::cout << "[Server] RPC received: " << raw;

        RPCMessage msg      = deserialize(raw);
        std::string result  = dispatch(msg);
        result             += "\n";

        send(client_fd, result.c_str(), result.size(), 0);
    }

    std::cout << "[Server] Client disconnected\n";
    close(client_fd);
    return nullptr;
}


int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(SERVER_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen"); return 1;
    }

    std::cout << "=== Remote Logger Server ===\n";
    std::cout << "Listening on port " << SERVER_PORT << " ...\n\n";

    while (true) {
        sockaddr_in client_addr{};
        socklen_t   client_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) { perror("accept"); continue; }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        std::cout << "[Server] New client connected: " << client_ip << "\n";

        int* fd_ptr = new int(client_fd);
        pthread_t tid;
        pthread_create(&tid, nullptr, client_thread, fd_ptr);
        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}