#include "parser_server.h"
#include "requests.h"
#include "hash.h"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>

using namespace std;

bool initializeSocket(server_arguments& args, int sockfd) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(args.port);

    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        cerr << "setsockopt(SO_REUSEADDR) failed: " << strerror(errno) << "\n";
        return true;
    }

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        cerr << "bind() failed on port " << args.port << ": " << strerror(errno) << "\n";
        return true;
    }

    if (listen(sockfd, 1) < 0) {
        cerr << "listen() failed on port " << args.port << ": " << strerror(errno) << "\n";
        return true;
    }

    return false;
}

void handle_client(int client_fd, server_arguments args) {
    try {
        InitRequest init;
        init.receive(client_fd);
        int N = ntohl(init.N);

        AckResponse ack{};
        ack.setValues(MessageType::AckResponse, N*40);
        ack.sendTo(client_fd);

        for (int i = 0; i < N; ++i) {
            HashRequest req{};
            HashResponse resp{};

            resp.setValues(MessageType::HashResponse, i);
            resp.Hash = req.receive(client_fd, args.salt);
            resp.sendTo(client_fd);
        }
    }
    catch (const exception &ex) {
        cerr << "Error: " << ex.what() << "\n";
    } catch (...) {
        cerr << "Unknown error occurred\n";
    }
    close(client_fd);
}


int main(int argc, char *argv[]) {
    server_arguments args{};
    server_parseopt(args, argc, argv);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);;
    if(initializeSocket(args, sockfd)) {
        cerr << "Server setup failed. Exiting cleanly.\n";
        close(sockfd);
        return 1;
    }

    while (true) {
        int client_fd = accept(sockfd, nullptr, nullptr);
        if (client_fd < 0) {
            cerr << "accept() failed: " << strerror(errno) << "\n";
            continue;
        }
        thread t(handle_client, client_fd, args);
        t.detach();
    }
}