#pragma once

#include <string>

void httpInit();
void httpCleanup();

// Return true on HTTP 2xx. On failure, `err` gets a short description.
bool httpGet(const std::string& url, std::string& out, std::string& err, long timeoutSec = 30);
bool httpPostJson(const std::string& url, const std::string& body, std::string& out, std::string& err);

std::string urlEncode(const std::string& s);
