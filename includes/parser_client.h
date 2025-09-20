#ifndef PARSER_CLIENT_H
#define PARSER_CLIENT_H

#include <netinet/in.h>
#include <string>
#include <stdio.h>
#include <argp.h>

// Struct to hold parsed arguments
struct client_arguments {
    struct sockaddr_in addr;
    int hashnum = -1;
    int smin;
    int smax;
    std::string filename;
    FILE *file;
};

/* Verifies whether provided string can be parsed as a number
 * On sucess returns True on fail False
 */
bool isNumber(const std::string& s);

/* Parse client command-line options. Supports:
 *   -a / --addr    : required IPv4 address (validated with inet_pton)
 *   -p / --port    : required port number (range 1025–65535)
 *   -n / --hashreq : required number of hash requests (>= 0)
 *   --smin         : required minimum payload size (>= 1)
 *   --smax         : required maximum payload size (<= 2^24, >= smin)
 *   -f / --file    : required input file (must exist and be readable)
 *
 * Called by argp for each option. Performs validation and fills
 * a client_arguments struct. On invalid or missing options, reports
 * errors via argp_error and terminates execution.
 */
error_t client_parser(int key, char *arg, struct argp_state *state);

/* Parse all client command-line arguments using argp.
 * Defines supported options (addr, port, hashreq, smin, smax, file),
 * delegates validation to client_parser, and fills a client_arguments struct.
 * On parse failure, prints an error; on success, prints the parsed values.
 */
void client_parseopt(int argc, char *argv[]);

#endif // PARSER_CLIENT_H