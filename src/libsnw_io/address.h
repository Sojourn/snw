#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include "socket.h"

namespace snw {

class address {
public:
    address();
    address(const char* name, socket_address_family address_family=socket_address_family::unknown);
    address(const address& other);

    address& operator=(const address& rhs);

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

    friend std::ostream& operator<<(std::ostream& out, const address& addr);

private:
    sockaddr_storage storage_;
};

}
