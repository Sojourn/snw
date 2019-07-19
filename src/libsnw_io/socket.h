#pragma once

#include "platform.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

namespace snw {

class address;

enum class socket_address_family {
    unknown = AF_UNSPEC, // FIXME: rename?
    ipv4    = AF_INET,
    ipv6    = AF_INET6,
    unix    = AF_UNIX,
};

enum class socket_type {
    stream = SOCK_STREAM,
    dgram  = SOCK_DGRAM,
    raw    = SOCK_RAW,
};

class socket {
public:
    socket(socket_address_family address_family, socket_type type);
    socket(socket&& other);
    socket(const socket&) = delete;
    ~socket();

    socket& operator=(socket&& rhs);
    socket& operator=(const socket&) = delete;

    void close();
    bool is_open() const;
    explicit operator bool() const;

    void set_blocking(bool blocking);

private:
    int fd_;
};

}
