#include "parser_server.h"
#include "requests.h"
#include "hash.h"

#include <iostream>

using namespace std;

void initializeSocket(server_arguments& args, int& sockfd, int& client_fd) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(args.port);

    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
        throw runtime_error("setsockopt(SO_REUSEADDR) failed!");

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        throw runtime_error("Binding failed!");
    cout << "Socket created and bound to the port\n";

    if (listen(sockfd, 1) < 0)
        throw runtime_error("Listen failed!");  
    cout << "Listning on port: " << args.port << "\n";

    socklen_t addrlen = sizeof(addr);
    client_fd = accept(sockfd, (struct sockaddr*)&addr, &addrlen);
    if (client_fd < 0)
        throw runtime_error("Accept failed!");
    cout << "Awaiting connections\n";
}

int main(int argc, char *argv[]) {
    server_arguments args{};
    server_parseopt(args, argc, argv);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        throw runtime_error("Socket init failed!");

    int client_fd;
    initializeSocket(args, sockfd, client_fd);

    InitRequest init;
    init.receive(client_fd);
    int N = ntohl(init.N);

    AckResponse ack{};
    ack.setValues(2, N*40);
    ack.sendTo(client_fd);
    for (int i = 0; i < int(ntohl(init.N)); ++i) {

        HashRequest req{};
        req.receive(client_fd);

        HashResponse resp{};
        resp.setValues(4, i);
        resp.Hash = compute_checksum(req.Payload, args.salt);
        resp.sendTo(client_fd);
    }
}