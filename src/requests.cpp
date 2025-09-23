#include "requests.h"
#include "hash.h"

#include <arpa/inet.h>
#include <stdexcept>
#include <algorithm>
#include <cstdio>

#define ERR_SEND(cls, field) ("Sending " #cls "::" #field " failed!")
#define ERR_RECV(cls, field) ("Receiving " #cls "::" #field " failed!")

using namespace std;

void sendAny(int sockfd, const void* data, ssize_t size, const char* errMsg) {
    ssize_t sent = send(sockfd, data, size, 0);
    if (sent != size)
        throw runtime_error(errMsg);
}

void receiveAny(int sockfd, void* bytes_received, ssize_t size, const char* errMsg) {
    ssize_t total = 0;
    char* buffer = reinterpret_cast<char*>(bytes_received);

    while (total < size) {
        ssize_t received = recv(sockfd, buffer + total, size - total, 0);
        if (received <= 0)
            throw runtime_error(errMsg);

        total += received;
    }
}

array<uint8_t, 32> receiveHash(int sockfd, ssize_t size, const string& salt, const char* errMsg) {
    const ssize_t CHUNK = UPDATE_PAYLOAD_SIZE;
    array<uint8_t, 32> digest{};
    char buffer[CHUNK];

    const uint8_t* salt_ptr = salt.empty() ? nullptr
                                           : reinterpret_cast<const uint8_t*>(salt.data());

    checksum_ctx* ctx = checksum_create(salt_ptr, salt.size());
    if (!ctx)
        throw runtime_error("Failed to create checksum context");

    ssize_t total = 0;
    while (total < size) {
        ssize_t toRead = min(CHUNK, size - total);
        ssize_t received = recv(sockfd, buffer, toRead, 0);
        if (received < 0) {
            checksum_destroy(ctx);
            throw runtime_error(errMsg);
        }

        total += received;

        if (total < size) {
            if (checksum_update(ctx, reinterpret_cast<uint8_t*>(buffer)) != 0) {
                checksum_destroy(ctx);
                throw runtime_error("checksum_update failed");
            }
        } else {
            if (checksum_finish(ctx, reinterpret_cast<uint8_t*>(buffer), received, digest.data()) != 0) {
                checksum_destroy(ctx);
                throw runtime_error("checksum_finish failed");
            }
        }
    }
    checksum_destroy(ctx);
    return digest;
}

void InitRequest::setValues(int n) {
    Type = htonl(static_cast<uint32_t>(MessageType::InitRequest));
    N = htonl(n);
}

void InitRequest::sendTo(int sockfd) const {
    sendAny(sockfd, &Type, sizeof(Type), ERR_SEND(InitRequest, Type));
    sendAny(sockfd, &N, sizeof(N), ERR_SEND(InitRequest, N));
}

void InitRequest::receive(int sockfd) {
    receiveAny(sockfd, &Type, sizeof(Type), ERR_RECV(InitRequest, Type));
    receiveAny(sockfd, &N, sizeof(N), ERR_RECV(InitRequest, N));
}

void AckResponse::setValues(MessageType type, int length) {
    Type = htonl(static_cast<uint32_t>(type));
    Length = htonl(length);
}

void AckResponse::sendTo(int sockfd) const {
    sendAny(sockfd, &Type, sizeof(Type), ERR_SEND(AckResponse, Type));
    sendAny(sockfd, &Length, sizeof(Length), ERR_SEND(AckResponse, Length));
}

void AckResponse::receive(int sockfd) {
    receiveAny(sockfd, &Type, sizeof(Type), ERR_RECV(AckResponse, Type));
    receiveAny(sockfd, &Length, sizeof(Length), ERR_RECV(AckResponse, Length));
}

void HashRequest::setValues(int length, FILE* file) {
    Type = htonl(static_cast<uint32_t>(MessageType::HashRequest));
    Length = htonl(length);
    Payload.resize(length);
    if (fread(Payload.data(), 1, length, file) != size_t(length))
        throw runtime_error("Failed to read expected payload from file");
}

void HashRequest::sendTo(int sockfd) const {
    sendAny(sockfd, &Type, sizeof(Type), ERR_SEND(HashRequest, Type));
    sendAny(sockfd, &Length, sizeof(Length), ERR_SEND(HashRequest, Length));
    sendAny(sockfd, Payload.data(), Payload.size(), ERR_SEND(HashRequest, Payload));
}

array<uint8_t, 32> HashRequest::receive(int sockfd, const string& salt) {
    receiveAny(sockfd, &Type, sizeof(Type), ERR_RECV(HashRequest, Type));
    receiveAny(sockfd, &Length, sizeof(Length), ERR_RECV(HashRequest, Length));
    return receiveHash(sockfd, ntohl(Length), salt, ERR_RECV(HashRequest, Payload));
}

void HashResponse::setValues(MessageType type, int i) {
    Type = htonl(static_cast<uint32_t>(type));
    I = htonl(i);
}

void HashResponse::sendTo(int sockfd) const {
    sendAny(sockfd, &Type, sizeof(Type), ERR_SEND(HashResponse, Type));
    sendAny(sockfd, &I, sizeof(I), ERR_SEND(HashResponse, Index));
    sendAny(sockfd, &Hash, sizeof(Hash), ERR_SEND(HashResponse, Hash));
}

void HashResponse::receive(int sockfd) {
    receiveAny(sockfd, &Type, sizeof(Type), ERR_RECV(HashResponse, Type));
    receiveAny(sockfd, &I, sizeof(I), ERR_RECV(HashResponse, Index));
    receiveAny(sockfd, &Hash, sizeof(Hash), ERR_RECV(HashResponse, Hash));
}