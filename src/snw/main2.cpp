#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "snw_util.h"
#include "snw_event.h"

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

namespace snw {

    struct file_open_req {
        const char *file_path;
        bool        readable = false;
        bool        writable = false;

        file_open_req(const char *file_path, bool readable = false, bool writable = false)
            : file_path(file_path)
            , readable(readable)
            , writable(writable)
        {}
    };

    struct file_open_rep {
        int status = -1;
        int file_handle = -1;
    };

    struct file_close_req {
        int file_handle = -1;
    };

    struct file_close_rep {
        int status = -1;
    };

    template<typename EventRouter>
    class filesystem_driver {
    public:
        filesystem_driver(EventRouter& router): router_(router) {
            router_.register_module(this);
        }

        ~filesystem_driver() {
            router_.unregister_module(this);
        }

        void recv(file_open_req req) {
            router_.send(file_open_rep {
            });
        }

        void recv(file_close_req req) {
            router_.send(file_open_rep {
            });
        }

    private:
        EventRouter& router_;
    };

    template<>
    struct subscription_list<filesystem_driver> {
        using events = event_list<
            file_open_req,
            file_close_req
        >;
    };
}

#if 0
int main(int argc, char** argv) {
    using EventRouter = snw::event_router<
        snw::filesystem_driver
    >;

    EventRouter event_router;
    snw::filesystem_driver<EventRouter> filesystem_driver(event_router);

    event_router.send(snw::file_open_req {
        "/usr/jrausch/saddness",
        true,
        false
    });

#ifdef SNW_OS_WINDOWS
    std::system("pause");
#endif
    return 0;
}

#endif

int main(int argc, char **argv) {
    int numbers[] = {
        4, 256, 3, -14, 180
    };

    int min_number = std::numeric_limits<int>::max();
    int max_number = std::numeric_limits<int>::min();

    for (int number: numbers) {
        min_number = std::min(number, min_number);
        max_number = std::max(number, max_number);
    }

    std::cout << "min: " << min_number << std::endl;
    std::cout << "max: " << max_number << std::endl;

    return 0;
}
