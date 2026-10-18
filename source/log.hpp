#pragma once

#include <string>

#define APP_DIR "sdmc:/switch/stremnx"

void logOpen(const char* path);
void logClose();
void logMsg(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

bool readFile(const std::string& path, std::string& out);
bool writeFile(const std::string& path, const void* data, size_t size);
bool fileExists(const std::string& path);
void ensureDir(const std::string& path);
