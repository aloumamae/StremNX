#pragma once

#include <pthread.h>

#include <functional>

#include "log.hpp"

// std::thread gets a small default stack on libnx; curl/TLS and image decoding need more.
inline bool startThread(std::function<void()> fn, size_t stackSize = 1024 * 1024) {
    auto* heapFn = new std::function<void()>(std::move(fn));
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, stackSize);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_t th;
    int rc = pthread_create(
        &th, &attr,
        [](void* p) -> void* {
            auto* f = static_cast<std::function<void()>*>(p);
            (*f)();
            delete f;
            return nullptr;
        },
        heapFn);
    pthread_attr_destroy(&attr);
    if (rc != 0) {
        delete heapFn;
        logMsg("pthread_create failed: %d", rc);
        return false;
    }
    return true;
}
