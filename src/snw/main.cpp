#include <iostream>
#include <chrono>
#include <vector>
#include <limits>
#include <string>
#include <atomic>
#include <memory>
#include <thread>
#include <stdexcept>
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cassert>

#include <unistd.h>

#include "snw_stream.h"
#include "align.h"
#include "varchar.h"

enum class msg_type {
    login,
    logout,
    heartbeat,

    open_doc,
    create_doc,
    close_doc,
};

struct msg_base {
    msg_type type;

    msg_base(msg_type type): type(type) {}
    virtual ~msg_base() {}
};

struct login_msg : public msg_base {
    snw::varchar<16> db_name;
    snw::varchar<16> app_name;
    snw::varchar<16> user_name;
    bool             read_only;

    login_msg(const char* db_name, const char* app_name, const char* user_name, bool read_only)
        : msg_base(msg_type::login)
        , db_name(db_name)
        , app_name(app_name)
        , user_name(user_name)
        , read_only(read_only)
    {
    }
};

struct logout_msg : public msg_base {
    logout_msg()
        : msg_base(msg_type::logout)
    {}
};

struct heartbeat_msg : public msg_base {
    heartbeat_msg()
        : msg_base(msg_type::heartbeat)
    {}
};

struct open_doc_msg : public msg_base {
    snw::varchar<256> path;

    open_doc_msg(const char* path)
        : msg_base(msg_type::open_doc)
        , path(path)
    {}
};

struct create_doc_msg : public msg_base {
    create_doc_msg()
        : msg_base(msg_type::create_doc)
    {}
};

struct close_doc_msg : public msg_base {
    close_doc_msg()
        : msg_base(msg_type::close_doc)
    {}
};

using msg_stream = snw::atomic_message_stream<msg_base>;

class doc_server {
public:
    doc_server(msg_stream& rx_stream, msg_stream& tx_stream)
        : rx_stream_(rx_stream)
        , tx_stream_(tx_stream)
        , stop_(false)
    {
        thread_ = std::thread(&doc_server::run, this);
    }

    ~doc_server() {
        stop_ = true;
        thread_.join();
    }

private:
    void run() {
        while (!stop_) {
            rx_stream_.read([&](msg_base& base) {
                switch (base.type) {
                case msg_type::login:
                    handle_login(static_cast<login_msg&>(base));
                    break;

                case msg_type::logout:
                    handle_logout(static_cast<logout_msg&>(base));
                    break;

                case msg_type::heartbeat:
                    handle_heartbeat(static_cast<heartbeat_msg&>(base));
                    break;

                default:
                    break;
                }
            });
        }
    }

    void handle_login(login_msg& msg) {
        std::cout << "login" << std::endl;
    }

    void handle_logout(logout_msg& msg) {
        std::cout << "logout" << std::endl;
    }

    void handle_heartbeat(heartbeat_msg& msg) {
        std::cout << "heartbeat" << std::endl;
    }

private:
    msg_stream&      rx_stream_;
    msg_stream&      tx_stream_;
    std::thread      thread_;
    std::atomic_bool stop_;
};

class doc_client {
public:
    doc_client(msg_stream& rx_stream, msg_stream& tx_stream)
        : rx_stream_(rx_stream)
        , tx_stream_(tx_stream)
    {
    }

    void login(const char* db_name, const char* app_name, const char* user_name, bool read_only) {
        tx_stream_.write<login_msg>(db_name, app_name, user_name, read_only);
    }

    void logout() {
        tx_stream_.write<logout_msg>();
    }

private:
    msg_stream& rx_stream_;
    msg_stream& tx_stream_;
};

int main(int argc, const char** argv) {
    msg_stream s1(1 << 16);
    msg_stream s2(1 << 16);

    doc_server server(s1, s2);
    doc_client client(s2, s1);

    client.login("snw", "test_client", "me", false);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    client.logout();

    return 0;
}
