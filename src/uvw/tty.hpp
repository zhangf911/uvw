#pragma once


#include <utility>
#include <cstddef>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "stream.hpp"
#include "util.hpp"


namespace uvw {


namespace details {


enum class UVTTYModeT: std::underlying_type_t<uv_tty_mode_t> {
    NORMAL = UV_TTY_MODE_NORMAL,
    RAW = UV_TTY_MODE_RAW,
    IO = UV_TTY_MODE_IO
};


}


class TTYHandle final: public StreamHandle<TTYHandle, uv_tty_t> {
    explicit TTYHandle(std::shared_ptr<Loop> ref,
                 FileHandle desc,
                 bool readable)
        : StreamHandle{std::move(ref)}, fd{desc}, rw{readable}
    { }

public:
    using Mode = details::UVTTYModeT;

    template<typename... Args>
    static std::shared_ptr<TTYHandle> create(Args&&... args) {
        return std::shared_ptr<TTYHandle>{new TTYHandle{std::forward<Args>(args)...}};
    }

    bool init() { return initialize<uv_tty_t>(&uv_tty_init, fd, rw); }

    void mode(Mode m) {
        // uv_tty_set_mode is inline, it cannot be used with invoke directly
        invoke([](auto *handle, auto m) {
            return uv_tty_set_mode(handle, m);
        }, get<uv_tty_t>(), static_cast<std::underlying_type_t<Mode>>(m));
    }

    void reset() { invoke(&uv_tty_reset_mode); }

    WinSize getWinSize() {
        std::pair<int, int> size{0, 0};
        int width;
        int height;

        if(0 == invoke(&uv_tty_get_winsize, get<uv_tty_t>(), &width, &height)) {
            size.first = width;
            size.second = height;
        }

        /**
         * See Boost/Mutant idiom:
         *     https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Boost_mutant
         */
        return reinterpret_cast<WinSize&>(size);
    }

private:
    FileHandle::Type fd;
    int rw;
};


}
