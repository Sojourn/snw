#include <stdexcept>
#include <cassert>
#include <fcntl.h>
#include "socket.h"

snw::socket::socket(socket_address_family address_family, socket_type type) {
    fd_ = ::socket(static_cast<int>(address_family), static_cast<int>(type), 0);
    if (fd_ < 0) {
        throw std::runtime_error("failed to open socket");
    }
}

snw::socket::socket(socket&& other)
    : fd_(other.fd_)
{
    other.fd_ = -1;
}

snw::socket::~socket() {
    close();
}

snw::socket& snw::socket::operator=(socket&& rhs) {
    if (this != &rhs) {
        close();

        fd_ = rhs.fd_;
        rhs.fd_ = -1;
    }

    return *this;
}

bool snw::socket::is_open() const {
    return fd_ >= 0;
}

snw::socket::operator bool() const {
    return is_open();
}

void snw::socket::close() {
    if (!is_open()) {
        return;
    }

    int err = ::close(fd_);
    assert(!err);
    fd_ = -1;
}

void snw::socket::set_blocking(bool blocking) {
    if (!is_open()) {
        throw std::runtime_error("socket is closed");
    }

    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags < 0) {
        throw std::runtime_error("failed to set socket option"); // TODO: details
    }

    if (blocking) {
        flags = flags & ~O_NONBLOCK;
    }
    else {
        flags = flags | O_NONBLOCK;
    }

    if (fcntl(fd_, F_SETFL, flags) < 0) {
        throw std::runtime_error("failed to set socket option"); // TODO: details
    }
}
