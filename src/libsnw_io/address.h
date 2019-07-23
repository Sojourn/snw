#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include "socket.h"

namespace snw {

class address {
public:
    address();
    address(const char* name, socket_address_family address_family=socket_address_family::unknown);
    address(const address& other);

    address& operator=(const address& rhs);

    bool operator==(const address& rhs) const;
    bool operator!=(const address& rhs) const;

    explicit operator bool() const;

    socket_address_family address_family() const;

    uint16_t port() const;
    void set_port(uint16_t port);

    sockaddr& addr();
    const sockaddr& addr() const;

    sockaddr_in& addr_ipv4();
    const sockaddr_in& addr_ipv4() const;

    sockaddr_in6& addr_ipv6();
    const sockaddr_in6& addr_ipv6() const;

    sockaddr_un& addr_unix();
    const sockaddr_un& addr_unix() const;

    std::string to_string() const {
        std::stringstream ss;
        ss << "address {\n";

        switch (address_family()) {
        case socket_address_family::ipv4:
            ss << "  family: IPv4\n";
            ss << "  addr: " << inet_ntoa(addr_ipv4().sin_addr) << '\n';
            ss << "  port: " << port() << '\n';
            break;
        case socket_address_family::unix:
            ss << "  family: Unix\n";
            ss << "  path: " << addr_unix().sun_path << '\n';
            break;
        case socket_address_family::ipv6:
            ss << "  family: IPv6\n";
            break;
        case socket_address_family::unknown:
            ss << "  family: None\n";
            break;
        default:
            throw std::runtime_error("address family not supported");
        }

        ss << '}';
        return ss.str();
    }

private:
    sockaddr_storage storage_;
};

}
