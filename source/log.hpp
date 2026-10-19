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

// Freeze watchdog: threads report progress with a short stage label; a background thread logs
// any slot that stops beating for a few seconds (the log is flushed line by line, so it survives
// having to force the app closed).
enum WatchSlot { WatchMain = 0, WatchEngine = 1, WatchSlots = 2 };
void watchdogStart();
void watchBeat(int slot, const char* stage);  // stage must be a string literal
void watchIdle(int slot);                     // slot is allowed to sleep (e.g. engine stopped)
