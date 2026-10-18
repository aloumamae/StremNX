#include "log.hpp"

#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <sys/stat.h>

static std::mutex g_logMutex;
static FILE* g_log = nullptr;

void logOpen(const char* path) {
    g_log = fopen(path, "w");
}

void logClose() {
    if (g_log) fclose(g_log);
    g_log = nullptr;
}

void logMsg(const char* fmt, ...) {
    std::lock_guard<std::mutex> lock(g_logMutex);
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
    if (g_log) {
        va_start(ap, fmt);
        vfprintf(g_log, fmt, ap);
        va_end(ap);
        fputc('\n', g_log);
        fflush(g_log);
    }
}

bool readFile(const std::string& path, std::string& out) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return false;
    out.clear();
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) out.append(buf, n);
    fclose(f);
    return true;
}

bool writeFile(const std::string& path, const void* data, size_t size) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;
    bool ok = fwrite(data, 1, size, f) == size;
    fclose(f);
    return ok;
}

bool fileExists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

void ensureDir(const std::string& path) {
    mkdir(path.c_str(), 0777);
}
