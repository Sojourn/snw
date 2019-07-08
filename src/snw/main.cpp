#include <iostream>
#include <memory>
#include <cstdint>

#include "intrusive_list.h"
#include "snw_lang.h"
#include "message_stream.h"
#include "slot_allocator.h"

#if defined(SNW_OS_UNIX)

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#elif defined(SNW_OS_WINDOWS)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#endif

namespace snw {

enum class socket_address_family {
    unknown = AF_UNSPEC, // FIXME: rename?
    ipv4    = AF_INET,
#if defined(SNW_OS_UNIX)
    ipv6    = AF_INET6,
    unix    = AF_UNIX,
#endif
};

enum class socket_type {
    stream = SOCK_STREAM,
    dgram  = SOCK_DGRAM,
    raw    = SOCK_RAW,
};

enum class socket_protocol {
    unknown = 0,
};

#if 0
class socket_protocol {
public:
    socket_protocol();
    socket_protocol(socket_address_family address_family, const char* protocol_name);

    explicit operator int() const {
        return id_;
    }

private:
    int id_;
};
#endif

class socket {
#if defined(SNW_OS_UNIX)
    using native_socket = int;
    static constexpr native_socket invalid_socket = -1;
#elif defined(SNW_OS_WINDOWS)
    using native_socket = SOCKET;
    static constexpr native_socket invalid_socket = INVALID_SOCKET;
#endif

public:
    socket(socket_address_family address_family, socket_type type, socket_protocol protocol=socket_protocol::unknown)
        : address_family_(address_family)
        , type_(type)
        , protocol_(protocol)
    {
#if defined(SNW_OS_UNIX)
        socket_ = ::socket(static_cast<int>(address_family_), static_cast<int>(type), static_cast<int>(protocol));
        if (socket_ < 0) {
            throw std::runtime_error("Failed to create socket"); // TODO: format error
        }
#elif defined(SNW_OS_WINDOWS)
        DWORD flags = WSA_FLAG_OVERLAPPED;

        socket_ = WSASocketW(static_cast<int>(address_family_), static_cast<int>(type_), static_cast<int>(protocol_), NULL, 0, flags);
        if (socket_ == INVALID_SOCKET) {
            int wsa_err = WSAGetLastError();
            (void)wsa_err;

            throw std::runtime_error("Failed to create socket"); // TODO: format error
        }
#else
#error "not implemented"
#endif
    }

    socket(socket&& other)
        : address_family_(other.address_family_)
        , type_(other.type_)
        , protocol_(other.protocol_)
        , socket_(other.socket_)
    {
        other.socket_ = invalid_socket;
    }

    socket(const socket& other) = delete;

    ~socket() {
        close();
    }

    void close() {
        if (socket_ == invalid_socket) {
            return;
        }

#if defined(SNW_OS_UNIX)
        int err = ::close(socket_);
        assert(!err);
#elif defined(SNW_OS_WINDOWS)
        int err = closesocket(socket_);
        assert(!err);
#else
#error "not implemented"
#endif

        socket_ = invalid_socket;
    }

private:
    socket_address_family address_family_;

    // Does anything need these?
    socket_type           type_;
    socket_protocol       protocol_;
    native_socket         socket_;
};

class address {
public:
    address() {
        memset(&storage_, 0, sizeof(storage_));
        addr()->sa_family = static_cast<decltype(addr()->sa_family)>(socket_address_family::unknown);
    }

    address(const address& other) {
        memcpy(&storage_, &other.storage_, sizeof(storage_));
    }

    address& operator=(const address& rhs) {
        if (this != &rhs) {
            memcpy(&storage_, &rhs.storage_, sizeof(storage_));
        }
        
        return *this;
    }

    sockaddr* addr() {
        return reinterpret_cast<sockaddr*>(&storage_);
    }
    
    const sockaddr* addr() const {
        return reinterpret_cast<const sockaddr*>(&storage_);
    }

    sockaddr_in* addr_in() {
        return reinterpret_cast<sockaddr_in*>(&storage_);
    }
    
    const sockaddr_in* addr_in() const {
        return reinterpret_cast<const sockaddr_in*>(&storage_);
    }

#if defined(SNW_OS_WINDOWS)
    SOCKADDR_IN6* addr_in6() {
        return reinterpret_cast<SOCKADDR_IN6*>(&storage_);
    }
#endif

#if defined(SNW_OS_WINDOWS)
    const SOCKADDR_IN6* addr_in6() const {
        return reinterpret_cast<const SOCKADDR_IN6*>(&storage_);
    }
#endif

    socket_address_family address_family() const {
        return static_cast<socket_address_family>(addr()->sa_family);
    }

    explicit operator bool() const {
        return address_family() == socket_address_family::unknown;
    }

private:
#if defined(SNW_OS_UNIX)
    // FIXME: does this already exist?
    union {
        sockaddr_in ipv4_addr;
        sockaddr_un unix_addr;
    } storage_;
#elif defined(SNW_OS_WINDOWS)
    SOCKADDR_STORAGE storage_;
#else
#error "not implemented"
#endif
};

address make_address(const char* hostname, socket_address_family address_family = socket_address_family::unknown) {
    return address();
}

}

// TODO: snake_case
class Application {
public:
    Application() {
#if defined(SNW_OS_WINDOWS)
        int err;
        WORD requested_version;
        WSADATA wsa_data;

        requested_version = MAKEWORD(2, 2);
        err = WSAStartup(requested_version, &wsa_data);
        if (err) {
            throw std::runtime_error("Failed to initialize winsocks2");
        }

        if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2) {
            err = WSACleanup();
            assert(!err);
            throw std::runtime_error("Failed to find a usable version of winsocks2");
        }
#endif
    }

    ~Application() {
#if defined(SNW_OS_WINDOWS)
        int err;
        
        err = WSACleanup();
        assert(!err);
#endif
    }

private:
    Application(Application&&) = delete;
    Application(const Application&) = delete;
    Application& operator=(Application&&) = delete;
    Application& operator=(const Application&) = delete;
};

int main(int argc, char** argv) {
    Application app;

    snw::socket sock(snw::socket_address_family::ipv4, snw::socket_type::stream);

#if defined(SNW_OS_WINDOWS)
    std::system("pause");
#endif
    return 0;
}
