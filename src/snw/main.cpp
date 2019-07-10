#include <iostream>
#include <memory>
#include <string>
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
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

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

    socket& operator=(socket&& rhs) {
        if (this != &rhs) {
            close();

            address_family_ = rhs.address_family_;
            type_ = rhs.type_;
            protocol_ = rhs.protocol_;
            socket_ = rhs.socket_;

            rhs.socket_ = invalid_socket;
        }

        return *this;
    }

    socket& operator=(const socket&) = delete;

    explicit operator bool() const {
        return socket_ != invalid_socket;
    }

    void close() {
        if (!*this) {
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

    void set_blocking(bool blocking) {
        if (!*this) {
            throw std::runtime_error("socket is closed");
        }

#if defined(SNW_OS_UNIX)
        int flags = fcntl(socket_, F_GETFL, 0);
        if (flags < 0) {
            throw std::runtime_error("failed to set socket option"); // TODO: details
        }

        if (blocking) {
            flags = flags & ~O_NONBLOCK;
        }
        else {
            flags = flags | O_NONBLOCK;
        }

        if (fcntl(socket_, F_SETFL, flags) < 0) {
            throw std::runtime_error("failed to set socket option"); // TODO: details
        }

#elif defined(SNW_OS_WINDOWS)
        unsigned long in_buf = blocking ? 0 : 1;
        unsigned long out_buf = 0;
        DWORD out_cnt = 0;

        int err = WSAIoctl(
            socket_,
            FIONBIO,
            &in_buf,
            sizeof(in_buf),
            &out_buf,
            sizeof(out_buf),
            &out_cnt,
            nullptr,
            nullptr
        );

        if (err < 0) {
            throw std::runtime_error("failed to set socket option"); // TODO: details
        }
#else
#error "not implemented"
#endif
    }

private:
    socket_address_family address_family_;
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

    // TODO: make name a string_view
    address(const char* name, socket_address_family address_family=socket_address_family::unknown) {
        memset(&storage_, 0, sizeof(storage_));

#if SNW_OS_UNIX
        if (address_family == socket_address_family::unix) {
            // TODO
            return;
        }
#endif

        static constexpr size_t buf_len = 4096;
        char buf[buf_len];

        int rc;
        int err;
        struct hostent hbuf;
        struct hostent* hres = nullptr;

        switch (address_family) {
        case socket_address_family::ipv4:
#ifdef SNW_OS_UNIX
        case socket_address_family::ipv6:
#endif
            rc = gethostbyname2_r(name, static_cast<int>(address_family), &hbuf, buf, buf_len, &hres, &err);
            break;

        default:
            rc = gethostbyname_r(name, &hbuf, buf, buf_len, &hres, &err);
            break;
        }

        if (rc == 0 && hres) {
            std::cout << "host: " << hres->h_name << std::endl;
            while (*hres->h_aliases) {
                std::cout << "alias: " << (hres->h_aliases++) << std::endl;
            }
            std::cout << "addrtype: " << hres->h_addrtype << std::endl;
            std::cout << "addrlen: " << hres->h_length << std::endl;
            while (*hres->h_addr_list) {
                std::cout << "addr: " << (void*)(hres->h_addr_list++) << std::endl;
            }
        }
        else if (rc == ERANGE) {
            std::cout << "buffer too small" << std::endl;
        }
        else {
            std::cout << "other error" << std::endl;
        }
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

    sockaddr_in* addr_ipv4() {
        return reinterpret_cast<sockaddr_in*>(&storage_);
    }
    
    const sockaddr_in* addr_ipv4() const {
        return reinterpret_cast<const sockaddr_in*>(&storage_);
    }

#if defined(SNW_OS_UNIX)
    sockaddr_in6* addr_ipv6() {
        return reinterpret_cast<sockaddr_in6*>(&storage_);
    }

    const sockaddr_in6* addr_ipv6() const {
        return reinterpret_cast<const sockaddr_in6*>(&storage_);
    }

    sockaddr_un* addr_unix() {
        return reinterpret_cast<sockaddr_un*>(&storage_);
    }

    const sockaddr_un* addr_unix() const {
        return reinterpret_cast<const sockaddr_un*>(&storage_);
    }
#elif defined(SNW_OS_WINDOWS)
    SOCKADDR_IN6* addr_ipv6() {
        return reinterpret_cast<SOCKADDR_IN6*>(&storage_);
    }

    const SOCKADDR_IN6* addr_ipv6() const {
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
    sockaddr_storage storage_;
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
class application {
public:
    application(size_t argc, char** argv) {
        (void)argc;
        (void)argv;
    }

    template<typename F>
    int run(F&& f) {
        int rc = EXIT_SUCCESS;

        try {
            setup();
            f();
            cleanup();
        }
        catch (const std::exception& ex) {
            std::cerr << ex.what() << std::endl;
            rc = EXIT_FAILURE;
        }

        return rc;
    }

private:
    application(application&&) = delete;
    application(const application&) = delete;
    application& operator=(application&&) = delete;
    application& operator=(const application&) = delete;

    void setup() {
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

    void cleanup() {
#if defined(SNW_OS_WINDOWS)
        if (WSACleanup() < 0) {
            throw std::runtime_error("WSACleanup failed");
        }
#endif
    }
};

int main(int argc, char** argv) {
    application app(argc, argv);

    return application(argc, argv).run([]() {
        snw::socket sock(snw::socket_address_family::ipv4, snw::socket_type::stream);
        sock.set_blocking(false);
        snw::address addr("google.com");
    });

#if defined(SNW_OS_WINDOWS)
    std::system("pause");
#endif
    return 0;
}
