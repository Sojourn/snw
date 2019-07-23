#include <cstring>
#include <cassert>
#include <stdexcept>
#include "address.h"

snw::address::address() {
    memset(&storage_, 0, sizeof(storage_));

    addr().sa_family = AF_UNSPEC;
}

snw::address::address(const char* name, socket_address_family address_family) {
    memset(&storage_, 0, sizeof(storage_));

    // TODO: clean up this switch statement
    switch (address_family) {
    case socket_address_family::ipv4: {
        sockaddr_in& sin = addr_ipv4();
        sin.sin_family = AF_INET;
        if (inet_aton(name, &sin.sin_addr) == 0) {
            return;
        }
        // fall through
    }
    case socket_address_family::ipv6:
    case socket_address_family::unknown: {
        int rc;
        int err;
        char buf[2048];
        struct hostent hbuf;
        struct hostent* hres = nullptr;

        if (address_family == socket_address_family::unknown) {
            rc = gethostbyname_r(name, &hbuf, buf, sizeof(buf), &hres, &err);
        }
        else {
            rc = gethostbyname2_r(name, static_cast<int>(address_family), &hbuf, buf, sizeof(buf), &hres, &err);
        }

        if (rc == 0 && hres) {
            // TODO: store one of these
            // while (*hres->h_aliases) {
            //     hres->h_aliases++;
            // }

            if (hres->h_addrtype == AF_INET) {
                sockaddr_in& sin = addr_ipv4();
                sin.sin_family = AF_INET;
                assert(hres->h_length == sizeof(sin.sin_addr));
                memcpy(&sin.sin_addr, *hres->h_addr_list, hres->h_length);
            }
            else if (hres->h_addrtype == AF_INET6) {
                sockaddr_in6& sin6 = addr_ipv6();
                sin6.sin6_family = AF_INET6;
                assert(hres->h_length == sizeof(sin6.sin6_addr));
                memcpy(&sin6.sin6_addr, *hres->h_addr_list, hres->h_length);
            }
            else {
                throw std::runtime_error("unsupported address type");
            }
        }
        // else if (rc == ERANGE) {
        //     throw std::runtime_error("buffer overflow");
        // }
        else {
            throw std::runtime_error(strerror(errno));
        }
        break;
    }

    case socket_address_family::unix: {
        sockaddr_un& sun = addr_unix();
        sun.sun_family = static_cast<int>(AF_UNIX);

        int name_len = strlen(name);
        if ((name_len + 1) > sizeof(sun.sun_path)) {
            throw std::runtime_error("path is too long");
        }

        memcpy(sun.sun_path, name, name_len + 1);
        break;
    }

    default:
        throw std::runtime_error("unsupported address family");
    }
}

snw::address::address(const address& other) {
    memcpy(&storage_, &other.storage_, sizeof(storage_));
}

snw::address& snw::address::operator=(const address& rhs) {
    if (this != &rhs) {
        memcpy(&storage_, &rhs.storage_, sizeof(storage_));
    }

    return *this;
}

bool snw::address::operator==(const address& rhs) const {
    return memcmp(&storage_, &rhs.storage_, sizeof(storage_)) == 0;
}

bool snw::address::operator!=(const address& rhs) const {
    return !operator==(rhs);
}

snw::address::operator bool() const {
    return address_family() != socket_address_family::unknown;
}

snw::socket_address_family snw::address::address_family() const {
    return static_cast<socket_address_family>(addr().sa_family);
}

uint16_t snw::address::port() const {
    switch (address_family()) {
    case socket_address_family::ipv4:
        return ntohs(addr_ipv4().sin_port);
    case socket_address_family::ipv6:
        return ntohs(addr_ipv6().sin6_port);
    default:
        throw std::runtime_error("address family does not use ports");
    }
}

void snw::address::set_port(uint16_t port) {
    switch (address_family()) {
    case socket_address_family::ipv4:
        addr_ipv4().sin_port = htons(port);
        break;
    case socket_address_family::ipv6:
        addr_ipv6().sin6_port = htons(port);
        break;
    default:
        throw std::runtime_error("address family does not use ports");
    }
}

sockaddr& snw::address::addr() {
    return *reinterpret_cast<sockaddr*>(&storage_);
}

const sockaddr& snw::address::addr() const {
    return *reinterpret_cast<const sockaddr*>(&storage_);
}

sockaddr_in& snw::address::addr_ipv4() {
    return *reinterpret_cast<sockaddr_in*>(&storage_);
}

const sockaddr_in& snw::address::addr_ipv4() const {
    return *reinterpret_cast<const sockaddr_in*>(&storage_);
}

sockaddr_in6& snw::address::addr_ipv6() {
    return *reinterpret_cast<sockaddr_in6*>(&storage_);
}

const sockaddr_in6& snw::address::addr_ipv6() const {
    return *reinterpret_cast<const sockaddr_in6*>(&storage_);
}

sockaddr_un& snw::address::addr_unix() {
    return *reinterpret_cast<sockaddr_un*>(&storage_);
}

const sockaddr_un& snw::address::addr_unix() const {
    return *reinterpret_cast<const sockaddr_un*>(&storage_);
}
