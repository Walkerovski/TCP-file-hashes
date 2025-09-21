#include "parser_client.h"

#include <iostream>
#include <cstring>
#include <cstdlib> 
#include <arpa/inet.h>

using namespace std;

bool isNumber(const string& s) {
    for (char it:s) {
        if (!isdigit(it)) return false;
    }
    return true;
}

error_t client_parser(int key, char *arg, struct argp_state *state) {
	struct client_arguments *args = (client_arguments *) state->input;
	error_t ret = 0;
	switch(key) {
	case 'a':
        if (inet_pton(AF_INET, arg, &args->addr.sin_addr) <= 0)
            argp_error(state, "Invalid address");

		args->addr.sin_family = AF_INET;

		break;
	case 'p':
		if (!isNumber(arg))
			argp_error(state, "Invalid option for a port, must be a number!");

		if (atoi(arg) < 1025 || atoi(arg) > 65535)
			argp_error(state, "Port is supposed to be a value in between 1025 and 65535!");

		args->addr.sin_port = htons(atoi(arg));
		break;
	case 'n':
		if (!isNumber(arg))
			argp_error(state, "Invalid option for the number of hash requests (-n --hashreq), must be a number!");

		args->hashnum = atoi(arg);
		if (args->hashnum < 0)
			argp_error(state, "The number of hash requests (-n --hashreq), must be >= 0");

		break;
	case 300: // smin
		if (!isNumber(arg))
			argp_error(state, "Invalid option for the minimum size for the data payload (--smin), must be a number!");

		args->smin = atoi(arg);
		if (args->smin < 1)
			argp_error(state, "The minimum size for the data payload (--smin), must be >= 1");
		break;
	case 301: // smax
		if (!isNumber(arg))
			argp_error(state, "Invalid option for the minimum size for the data payload (--smax), must be a number!");

		args->smax = atoi(arg);
		if (args->smax > 1<<24)
			argp_error(state, "The maximum size for the data payload (--smax), must be <= 2^24 (%d)", 1<<24);
		break;
	case 'f':
        args->filename = arg;
        args->file = fopen(arg, "r");
		break;
    case ARGP_KEY_END:
        if (args->addr.sin_addr.s_addr == INADDR_ANY)
            argp_error(state, "Option -a (--addr) is required!");
        if (args->addr.sin_port == 0)
            argp_error(state, "Option -p (--port) is required!");
        if (args->hashnum == -1)
            argp_error(state, "Option -n (--hashreq) is required!");
        if (args->smin == 0)
            argp_error(state, "Option --smin is required!");
		if (args->smax < args->smin)
			argp_error(state, "The maximum size for the data payload (--smax), must be greater or equal than the minimum size (--smin)");
        if (args->smax == 0)
            argp_error(state, "Option --smax is required!");
        if (args->filename == "")
            argp_error(state, "Option -f (--file) is required!");
        if (args->file == 0)
            argp_error(state, "Error: could not open file '%s'", args->filename.c_str());
        break;
	default:
		ret = ARGP_ERR_UNKNOWN;
		break;
	}
	return ret;
}

void client_parseopt(client_arguments& args, int argc, char *argv[]) {
	struct argp_option options[] = {
		{ "addr", 'a', "addr", 0, "The IP address the server is listening at", 0},
		{ "port", 'p', "port", 0, "The port that is being used at the server", 0},
		{ "hashreq", 'n', "hashreq", 0, "The number of hash requests to send to the server", 0},
		{ "smin", 300, "minsize", 0, "The minimum size for the data payload in each hash request", 0},
		{ "smax", 301, "maxsize", 0, "The maximum size for the data payload in each hash request", 0},
		{ "file", 'f', "file", 0, "The file that the client reads data from for all hash requests", 0},
		{ 0, 0, 0, 0, 0, 0 }
	};

	struct argp argp_settings = { options, client_parser, 0, 0, 0, 0, 0 };

	if (argp_parse(&argp_settings, argc, argv, 0, NULL, &args) != 0)
		cout << "Got an error condition when parsing\n";

	cout << "Got " << inet_ntoa(args.addr.sin_addr) << " on port " << ntohs(args.addr.sin_port) << " with n="
        << args.hashnum << " smin=" << args.smin << " smax=" << args.smax << " filename=" << args.filename << "\n";
}
