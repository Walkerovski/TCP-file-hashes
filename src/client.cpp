#include "parser_client.h"
#include "requests.h"

#include <iostream>
#include <random>
#include <iomanip>
#include <unistd.h>
#include <cstring>

using namespace std;

ostream& operator<<(ostream& os, HashResponse const& resp) {
    os << "0x";
    for (unsigned char b : resp.Hash) {
        os << hex << setw(2) << setfill('0') << int(b);
    }
    os << dec;
    return os;
}

int main(int argc, char *argv[]) {
    try {
        client_arguments args{};
        client_parseopt(args, argc, argv);

        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            cerr << "socket() failed: " << strerror(errno) << "\n";
            return 1;
        }

        if (connect(sockfd, (struct sockaddr *)&args.addr, sizeof(args.addr)) < 0) {
            cerr << "connect() failed: " << strerror(errno) << "\n";
            close(sockfd);
            return 1;
        }

        InitRequest initreq;
        initreq.setValues(args.hashnum);
        initreq.sendTo(sockfd);

        AckResponse ack;
        ack.receive(sockfd);

        for (int i = 0; i < args.hashnum; ++i) {
            HashRequest hashreq;
            int L = args.smin + rand() % (args.smax - args.smin + 1);
            hashreq.setValues(L, args.file);
            hashreq.sendTo(sockfd);

            HashResponse resp{};
            resp.receive(sockfd);
            cout << i << ": " << resp << "\n";
        }
    } catch (const exception &ex) {
        cerr << "Error: " << ex.what() << "\n";
        return 1;
    } catch (...) {
        cerr << "Unknown error occurred\n";
        return 1;
    }
}