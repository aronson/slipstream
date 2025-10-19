#include <iostream>
#include <string>
#include <vector>
#include <picosocks.h>
#include "slipstream.h"
#include "quick_arg_parser.hpp"

struct ClientArgs : MainArguments<ClientArgs> {
    using MainArguments<ClientArgs>::MainArguments;

    int listen_port = option("tcp-listen-port", 'l', "Listen port (default: 5201)") = 5201;
    std::vector<std::string> resolver = option("resolver", 'r', "Slipstream server resolver address (e.g., 1.1.1.1 or 8.8.8.8:53). Can be specified multiple times. (Required)");
    std::string congestion_control = option("congestion-control", 'c', "Congestion control algorithm (bbr, dcubic) (default: dcubic)") = "dcubic";
    bool gso = option('g', "GSO enabled (true/false) (default: false). Use --gso or --gso=true to enable.");
    std::string domain = option("domain", 'd', "Domain name used for the covert channel (Required)");
    int keep_alive_interval = option("keep-alive-interval", 't', "Send keep alive pings at this interval (default: 400, disabled: 0)") = 400;

    static std::string help(const std::string& program_name) {
        return "slipstream-client - A high-performance covert channel over DNS (client)\n\n" 
               "Usage: " + program_name + " [options]";
    }

    static const std::string version;
};

const std::string ClientArgs::version = "slipstream-client 0.1";

int main(int argc, char** argv) {
    int exit_code = 0;
    ClientArgs args(argc, argv);

#ifdef _WINDOWS
    WSADATA wsaData = { 0 };
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", iResult);
        return 1;
    }
#endif

    // Ensure output buffers are flushed immediately (useful for debugging/logging)
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    /* Check mandatory client arguments */
    if (args.domain.empty()) {
        std::cerr << "Client error: Missing required --domain option" << std::endl;
        exit(1);
    }
    if (args.resolver.empty()) {
        std::cerr << "Client error: Missing required --resolver option (at least one required)" << std::endl;
        exit(1);
    }

    // Process resolver addresses
    std::vector<st_address_t> resolver_addresses;
    for (const auto& res_str : args.resolver) {
        st_address_t addr;
        char server_name[256];
        int server_port = 53;

        if (sscanf(res_str.c_str(), "%255[^:]:%d", server_name, &server_port) < 1) {
            strncpy(server_name, res_str.c_str(), sizeof(server_name) - 1);
            server_name[sizeof(server_name) - 1] = '\0';
        }

        if (server_port <= 0 || server_port > 65535) {
            std::cerr << "Invalid port number in resolver address: " << res_str << std::endl;
            exit(1);
        }

        int is_name = 0;
        if (picoquic_get_server_address(server_name, server_port, &addr.server_address, &is_name) != 0) {
            std::cerr << "Cannot resolve resolver address '" << server_name << "' port " << server_port << std::endl;
            exit(1);
        }
        resolver_addresses.push_back(addr);
    }

    exit_code = picoquic_slipstream_client(
        args.listen_port,
        resolver_addresses.data(),
        resolver_addresses.size(),
        (char*)args.domain.c_str(),
        (char*)args.congestion_control.c_str(),
        args.gso,
        args.keep_alive_interval
    );

#ifdef _WINDOWS
    WSACleanup();
#endif

    exit(exit_code);
}
