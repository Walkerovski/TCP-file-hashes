#ifndef REQUESTS_H
#define REQUESTS_H

#include <cstdint>
#include <vector>
#include <array>
#include <stdexcept>
#include <arpa/inet.h>


// sendAny: sends a buffer over a socket.
inline void sendAny(int& sockfd, const void* data, ssize_t size, const char* errMsg) {
    ssize_t sent = send(sockfd, data, size, 0);
    if (sent != size)
        throw std::runtime_error(errMsg);
}

// receiveAny: receives a buffer sent over a socket.
inline void receiveAny(int sockfd, void* bytes_received, ssize_t size, const char* errMsg) {
    ssize_t total = 0;
    char* buffer = reinterpret_cast<char*>(bytes_received);

    while (total < size) {
        ssize_t received = recv(sockfd, buffer + total, size - total, 0);
        if (received <= 0)
            throw std::runtime_error(errMsg);

        total += received;
    }
}


// InitRequest: sent by client to initialize communication
struct InitRequest {
    uint32_t Type;
    uint32_t N;

    void setValues(int n) {
        Type = htonl(1);
        N = htonl(n);
    }

    void sendTo(int& sockfd) const {
        sendAny(sockfd, &Type, sizeof(Type), "Sending initialisation's type failed!");
        sendAny(sockfd, &N, sizeof(N), "Sending initialisation's number of hashes failed!");
    }

    void receive(int& sockfd) {
        receiveAny(sockfd, this, sizeof(*this), "Receiving an initialisation failed!");
    }
};

// AckResponse: server acknowledgment
struct AckResponse {
    uint32_t Type;
    uint32_t Length;

    void setValues(int type, int length) {
        Type = htonl(type);
        Length = htonl(length);
    }

    void sendTo(int& sockfd) const {
        sendAny(sockfd, &Type, sizeof(Type), "Sending acknowledgement's type failed!");
        sendAny(sockfd, &Length, sizeof(Length), "Sending acknowledgement's length failed!");
    }

    void receive(int& sockfd) {
        receiveAny(sockfd, this, sizeof(*this), "Receiving an acknowledgement failed!");
    }
};

// HashRequest: client sends data to be hashed
struct HashRequest {
    uint32_t Type;
    uint32_t Length;
    std::vector<uint8_t> Payload;

    void setValues(int length, FILE* file) {
        Type = htonl(3);
        Length = htonl(length);
        Payload.resize(length);
        if (fread(Payload.data(), 1, length, file) != size_t(length))
            throw std::runtime_error("Failed to read expected payload from file");
    }

    void sendTo(int& sockfd) const {
        sendAny(sockfd, &Type, sizeof(Type), "Sending a HashRequest's type failed!");
        sendAny(sockfd, &Length, sizeof(Length), "Sending a HashRequest's length failed!");
        sendAny(sockfd, Payload.data(), Payload.size(), "Sending a HashRequest's payload failed!");
    }

    void receive(int& sockfd) {
        receiveAny(sockfd, &Type, sizeof(Type), "Receiving a HashRequest's type failed!");
        receiveAny(sockfd, &Length, sizeof(Length), "Receiving a HashRequest's length failed!");
        Payload.resize(ntohl(Length));
        receiveAny(sockfd, Payload.data(), Payload.size(), "Receiving a HashRequest's payload failed!");
    }
};

// HashResponse: server returns hash result
struct HashResponse {
    uint32_t Type;
    uint32_t I;
    std::array<uint8_t, 32> Hash;


    void setValues(int type, int i) {
        Type = htonl(type);
        I = htonl(i);
    }

    void sendTo(int& sockfd) const {
        sendAny(sockfd, &Type, sizeof(Type), "Sending a HashRequest's type failed!");
        sendAny(sockfd, &I, sizeof(I), "Sending a HashRequest's index failed!");
        sendAny(sockfd, &Hash, sizeof(Hash), "Sending a hash failed!");
    }

    void receive(int& sockfd) {
        receiveAny(sockfd, &Type, sizeof(Type), "Receiving a HashRequest's type failed!");
        receiveAny(sockfd, &I, sizeof(I), "Receiving a HashRequest's index failed!");
        receiveAny(sockfd, &Hash, sizeof(Hash), "Receiving a hash failed!");
    }
};

#endif // REQUESTS_H