#ifndef REQUESTS_H
#define REQUESTS_H

#include <cstdint>
#include <vector>
#include <array>
#include <stdexcept>
#include <arpa/inet.h>


// sendAny: sends a buffer over a socket.
inline void sendAny(int& sockfd, const void* data, uint32_t size, const char* errMsg) {
    ssize_t sent = send(sockfd, data, size, 0);
    if (sent != static_cast<ssize_t>(size))
        throw std::runtime_error(errMsg);
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
        sendAny(sockfd, &Type, sizeof(Type), "Sending type failed!");
        sendAny(sockfd, &N, sizeof(N), "Sending number of hashes failed!");
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
        sendAny(sockfd, &Type, sizeof(Type), "Sending type failed!");
        sendAny(sockfd, &Length, sizeof(Length), "Sending length failed!");
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
        fread(Payload.data(), 1, length, file);
    }

    void sendTo(int& sockfd) const {
        sendAny(sockfd, &Type, sizeof(Type), "Sending type failed!");
        sendAny(sockfd, &Length, sizeof(Length), "Sending length failed!");
        sendAny(sockfd, Payload.data(), Payload.size(), "Sending payload failed!");
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
        sendAny(sockfd, &Type, sizeof(Type), "Sending type failed!");
        sendAny(sockfd, &I, sizeof(I), "Sending index failed!");
        sendAny(sockfd, &Hash, sizeof(Hash), "Sending hash failed!");
    }
};


#endif // REQUESTS_H
