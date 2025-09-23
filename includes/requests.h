#ifndef REQUESTS_H
#define REQUESTS_H

#include <cstdint>
#include <vector>
#include <array>
#include <string>

struct checksum_ctx;

/* Protocol Message Types */
enum class MessageType : uint32_t {
    InitRequest  = 1,
    AckResponse  = 2,
    HashRequest  = 3,
    HashResponse = 4
};

/**
 * @brief Send a buffer over a socket.
 *
 * Attempts to send exactly @p size bytes from @p data using the socket
 * file descriptor @p sockfd. If the number of bytes sent does not match
 * the requested size, a std::runtime_error is thrown.
 *
 * @param sockfd   Socket file descriptor.
 * @param data     Pointer to the buffer to send.
 * @param size     Number of bytes to send.
 * @param errMsg   Error message included in the exception if sending fails.
 *
 * @throws std::runtime_error if sending fails or fewer than @p size bytes are sent.
 */
void sendAny(int sockfd, const void* data, ssize_t size, const char* errMsg);

/**
 * @brief Receive a fixed-size buffer from a socket.
 *
 * Reads exactly @p size bytes into @p buffer from the socket @p sockfd.
 * The function will loop until all bytes are received or an error occurs.
 *
 * @param sockfd   Socket file descriptor.
 * @param buffer   Pointer to the buffer where received data will be stored.
 * @param size     Number of bytes to receive.
 * @param errMsg   Error message included in the exception if receiving fails.
 *
 * @throws std::runtime_error if the socket is closed or an error occurs.
 */
void receiveAny(int sockfd, void* buffer, ssize_t size, const char* errMsg);

/**
 * @brief Receive a payload from a socket and compute its checksum hash.
 *
 * Reads exactly @p size bytes from the socket @p sockfd in chunks,
 * updating the checksum as data arrives. If a @p salt is provided,
 * it is included in the checksum computation. When all data is received,
 * the final 32-byte digest is returned.
 *
 * @param sockfd   Socket file descriptor.
 * @param size     Number of bytes to receive.
 * @param salt     Optional string used as checksum salt (may be empty).
 * @param errMsg   Error message included in the exception if receiving fails.
 *
 * @return std::array<uint8_t, 32>  Final computed hash digest.
 *
 * @throws std::runtime_error on socket errors or checksum API errors.
 */
std::array<uint8_t, 32> receiveHash(int sockfd, ssize_t size, const std::string& salt, const char* errMsg);

/* Protocol Structures */
struct InitRequest {
    uint32_t Type;
    uint32_t N;

    void setValues(int n);
    void sendTo(int sockfd) const;
    void receive(int sockfd);
};

struct AckResponse {
    uint32_t Type;
    uint32_t Length;

    void setValues(MessageType type, int length);
    void sendTo(int sockfd) const;
    void receive(int sockfd);
};

struct HashRequest {
    uint32_t Type;
    uint32_t Length;
    std::vector<uint8_t> Payload;

    void setValues(int length, FILE* file);
    void sendTo(int sockfd) const;
    std::array<uint8_t, 32> receive(int sockfd, const std::string& salt);
};

struct HashResponse {
    uint32_t Type;
    uint32_t I;
    std::array<uint8_t, 32> Hash;

    void setValues(MessageType type, int i);
    void sendTo(int sockfd) const;
    void receive(int sockfd);
};

#endif // REQUESTS_H